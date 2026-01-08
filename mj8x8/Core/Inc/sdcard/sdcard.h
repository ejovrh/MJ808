/**
 * @file sdcard.h
 * @brief SD Card interface for logging and file operations.
 * @details Provides an abstraction for SD card operations using FatFs, including write and unmount functionality.
 */
#ifndef CORE_INC_SDCARD_SDCARD_H_
#define CORE_INC_SDCARD_SDCARD_H_

#include "main.h"

#if USE_SPI && USE_SDCARD // Only compile if SPI and SD card logging are enabled
#include "FATFS/ff.h"

#define COMPILE_WITH_READ 0 // if 1, implement Read() function (not implemented yet)

/**
 * @struct sdcard_t
 * @brief Public API for SD Card operations.
 * @details This struct provides function pointers for SD card operations. All internal implementation details are hidden.
 */
typedef struct
{
#if COMPILE_WITH_READ
    /**
     * @brief Read data from SD card (not implemented).
     * @details This function is a stub and does not perform any operation.
     */
    void (*Read)(void);
#endif
	/**
	 * @brief Write data to filename on SD card.
	 * @details Data length must be a multiple of 512 bytes (SD card sector size).
	 * If not, the function will return an error and not write.
	 * @param filename File to write to.
	 * @param data Pointer to data buffer.
	 * @param len Length of data in bytes (must be multiple of 512).
	 * @return FatFs error code (FR_OK on success).
	 */
	FRESULT (*Write)(const char *filename, const uint8_t *data, uint16_t len);
	/**
	 * @brief Unmount the SD card.
	 * @details Ensures all data is flushed and the card is safely unmounted.
	 */
	void (*Unmount)(void);
} sdcard_t;

/**
 * @brief SD Card constructor.
 * @details Initializes the SD card object and prepares it for use. Only the public API is exposed.
 * @param sdcard_port GPIO port for card detect.
 * @param card_detect_pin GPIO pin for card detect.
 * @param ss_pin Chip select pin for SD card SPI.
 * @return Pointer to initialised sdcard_t object.
 */
sdcard_t* sdcard_ctor(GPIO_TypeDef *sdcard_port, uint16_t card_detect_pin, uint16_t ss_pin);

#endif

#endif /* CORE_INC_SDCARD_SDCARD_H_ */
