/**
 * @file sdcard.c
 * @brief SD Card implementation for logging and file operations.
 * @details Implements the internal logic for SD card operations using FatFs. All internal details are hidden from the public API.
 */
#include "main.h"

#if USE_SPI && USE_SDCARD // Only compile if SPI and SD card logging are enabled
#include "sdcard.h"
#include "FatFs/ff.h"

#define SDCARD_SECTOR_SIZE 512 /**< SD card sector size in bytes. */

/**
 * @struct __sdcard_t
 * @brief Internal SD Card object.
 * @details Contains all private members and state for SD card operations. Not exposed outside this file.
 */
typedef struct
{
	sdcard_t public; /**< Public API struct. */
	uint8_t (*_CardIsNotPresent)(void); /**< Checks if SD card is NOT present. */
	FRESULT _MountResult; /**< Stores mount command output. */
	FRESULT _fopenResult; /**< Stores file open result. */
	FRESULT _seekResult; /**< Stores file seek result. */
	FRESULT _writeResult; /**< Stores file write result. */
	FSIZE_t _fsizeSize; /**< Stores file size. */
	uint16_t _SDCardDetectPin; /**< SD card detect pin. */
	uint16_t _SDCardChipSelectPin; /**< SD card chip select pin. */
	GPIO_TypeDef *_SDCardPort; /**< SD card detect port. */
	FATFS _FatFs; /**< FatFs object. */
	FIL _FilePointer; /**< File handler. */
} __sdcard_t;

/**
 * @brief Internal SD card object instance.
 * @details Preallocated in .data section for single-instance use.
 */
static __sdcard_t __SDCARD __attribute__((section(".data")));

/**
 * @brief Checks if SD card is NOT present.
 * @details Reads the GPIO pin to determine card presence. Returns 1 if not present, 0 if present.
 * @return 1 if card not present, 0 if present.
 */
static inline uint8_t _CardIsNotPresent(void)
{
	// GPIO will be high if SD card is not present
	return HAL_GPIO_ReadPin(__SDCARD._SDCardPort, __SDCARD._SDCardDetectPin);
}

/**
 * @brief Mounts the SD card and prepares for read/write.
 * @details Mounts the default drive using FatFs. Returns FatFs error code.
 * @return FatFs error code (FR_OK on success).
 */
static inline FRESULT _Mount(void)
{
	if(__SDCARD._CardIsNotPresent())  // if card not present
		return FR_NOT_READY;  // do nothing
	__SDCARD._MountResult = f_mount(&__SDCARD._FatFs, "", 1);  // Mount the default drive using private member
	return __SDCARD._MountResult;  // return mount result
}

/**
 * @brief Resets internal error codes and file size.
 * @details Used to clear state after unmount or initialization.
 */
static inline void _ResetState(void)
{
	__SDCARD._MountResult = FR_NOT_READY;
	__SDCARD._fopenResult = FR_NOT_READY;
	__SDCARD._seekResult = FR_NOT_READY;
	__SDCARD._writeResult = FR_NOT_READY;
	__SDCARD._fsizeSize = 0;
}

/**
 * @brief Unmounts the SD card.
 * @details Flushes and closes the file, unmounts the drive, and resets internal state.
 */
static inline void _Unmount(void)
{
	f_sync(&__SDCARD._FilePointer);
	f_close(&__SDCARD._FilePointer);
	f_mount(NULL, "", 1);  // unmount the default drive

	_ResetState();
}

#if COMPILE_WITH_READ
/**
 * @brief Stub for reading data from SD card.
 * @details Not implemented. Returns immediately if card not present.
 */
static void _Read(void)
{
    if(__SDCARD._CardIsNotPresent())  // if card not present
        return;  // do nothing
}
#endif

/**
 * @brief Writes data to a file on the SD card.
 * @details Data length must be a multiple of sector size. Opens file in append mode and writes sector-wise.
 * @param filename File to write to.
 * @param data Pointer to data buffer.
 * @param len Length of data in bytes (must be multiple of sector size).
 * @return FatFs error code (FR_OK on success).
 */
static FRESULT _Write(const char *filename, const uint8_t *data, uint16_t len)
{
	if(len == 0 || (len % SDCARD_SECTOR_SIZE) != 0)
		return FR_INVALID_PARAMETER;  // Only write multiples of sector size

	if(_Mount() != FR_OK)
		return __SDCARD._MountResult;

	// Open file in append mode, pointer will be set to end automatically
	__SDCARD._fopenResult = f_open(&__SDCARD._FilePointer, filename, FA_WRITE | FA_OPEN_APPEND);

	if(__SDCARD._fopenResult != FR_OK)
		{
			_Unmount();
			return __SDCARD._fopenResult;
		}

	UINT bytesWrote = 0;
	uint16_t offset = 0;
	uint16_t full_sectors = len / SDCARD_SECTOR_SIZE;

	for(uint16_t i = 0; i < full_sectors; i++)
		{
			__SDCARD._writeResult = f_write(&__SDCARD._FilePointer, data + offset, SDCARD_SECTOR_SIZE, &bytesWrote);

			if(__SDCARD._writeResult != FR_OK || bytesWrote != SDCARD_SECTOR_SIZE)
				{
					_Unmount();
					return __SDCARD._writeResult;
				}

			offset += SDCARD_SECTOR_SIZE;
		}

	_Unmount();
	return FR_OK;  // Success
}

/**
 * @brief Internal SD card object initialization.
 * @details Sets up function pointers for public API and initializes internal state.
 */
static __sdcard_t __SDCARD =
	{._CardIsNotPresent = &_CardIsNotPresent,  // checks for SD card inserted: 1 - not present, 0 - present
	.public.Write = _Write,  // write data to filename on SD card
#if COMPILE_WITH_READ
        .public.Read = _Read,  // TODO - Read not implemented
#endif
	    .public.Unmount = _Unmount,  // unmount SD card
	};

/**
 * @brief SD Card constructor.
 * @details Initializes the SD card object and prepares it for use. Only the public API is exposed.
 * @param card_detect_port GPIO port for card detect.
 * @param card_detect_pin GPIO pin for card detect.
 * @param ss_pin Chip select pin for SD card SPI.
 * @return Pointer to initialised sdcard_t object.
 */
sdcard_t* sdcard_ctor(GPIO_TypeDef *card_detect_port, uint16_t card_detect_pin, uint16_t ss_pin)
{
	__SDCARD._SDCardPort = card_detect_port;  // store port
	__SDCARD._SDCardDetectPin = card_detect_pin;  // store card detect pin
	__SDCARD._SDCardChipSelectPin = ss_pin;  // store chip select pin
	_ResetState();
	return &__SDCARD.public;
}

#endif
