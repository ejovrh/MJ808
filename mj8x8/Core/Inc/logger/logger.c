/**
 * @file logger.c
 * @brief Logger implementation for MJ808 project.
 * @details
 * This file implements the logger module, providing logging to a ring buffer and SD card.
 * It supports logging raw binary values, flushing the buffer, and managing the logger lifecycle.
 */
#include "main.h"

#if USE_SPI && USE_LOGGER // Only compile if SPI and LOGGER are enabled
#include "logger.h"
#include "lwrb/lwrb.h"

/**
 * @def LOG_BUFFER_SIZE
 * @brief Size of the raw log buffer in bytes.
 * @details
 * Defines the maximum number of bytes that can be stored in the logger's ring buffer.
 */
#define LOG_BUFFER_SIZE 1024 // raw buffer size in bytes

/**
 * @var _RingBufferData
 * @brief Raw buffer container for log data.
 * @details
 * Stores the raw binary log entries before they are flushed to the SD card.
 */
static uint8_t _RingBufferData[LOG_BUFFER_SIZE];

/**
 * @var _RingBufferObject
 * @brief Ring buffer object for log entries.
 * @details
 * Manages the circular buffer for storing log entries.
 */
static lwrb_t _RingBufferObject;

/**
 * @struct __logger_t
 * @brief Internal logger structure.
 * @details
 * Contains the public logger interface, ring buffer pointer, log file name, and SD card object pointer.
 */
typedef struct
{
	logger_t public; /**< Public logger interface. */
	lwrb_t *_RingBuffer; /**< Ring buffer for log entries. */
	const char *_LogFileName; /**< Private log file name pointer. */
	sdcard_t *_SDCardObj; /**< Pointer to SD card object. */
} __logger_t;

/**
 * @var __LOGGER
 * @brief Preallocated internal logger object.
 * @details
 * Stores the logger state and is placed in the .data section.
 */
static __logger_t __LOGGER __attribute__((section(".data")));/**
 * @brief Write raw values to the ring buffer.
 * @details
 * Logs values as raw binary, always 4 bytes per value, no header. Adds commas between values and appends CRLF.
 * @param entry Array of pointers to 4-byte values to log.
 * @param count Number of entries in the array.
 */
static inline void _WriteRawValuesToRingBuffer(void *const*entry, const uint8_t count)
{
	// Calculate buffer size: 4 bytes per value, (count-1) commas, 2 bytes for CRLF
	uint8_t buffer[4 * count + (count ? (count - 1) : 0) + 2];
	uint8_t *p = buffer;

	for(uint8_t i = 0; i < count; ++i)
		{
			uint32_t val;
			memcpy(&val, entry[i], 4);  // Safe for unaligned access
			*p++ = (uint8_t) (val);
			*p++ = (uint8_t) (val >> 8);
			*p++ = (uint8_t) (val >> 16);
			*p++ = (uint8_t) (val >> 24);

			if(i < count - 1)
				*p++ = ',';
		}

	*p++ = '\r';
	*p++ = '\n';

	lwrb_write(__LOGGER._RingBuffer, buffer, (lwrb_sz_t) (p - buffer));
}

/**
 * @brief Flush ring buffer to SD card.
 * @details
 * Writes all buffered log entries to the SD card and skips them in the buffer.
 * @param logger Pointer to the internal logger object.
 */
static inline void _FlushRingBufferToSDCard(__logger_t *logger)
{
	const uint16_t SECTOR_SIZE = 512;
	while(lwrb_get_full(logger->_RingBuffer) >= SECTOR_SIZE)
	{
		lwrb_sz_t len = lwrb_get_linear_block_read_length(logger->_RingBuffer);
		if(len >= SECTOR_SIZE)
		{
			// Only write full 512-byte blocks
			uint16_t write_len = (uint16_t)(len - (len % SECTOR_SIZE));
			uint8_t *ptr = (uint8_t*) lwrb_get_linear_block_read_address(logger->_RingBuffer);
			logger->_SDCardObj->Write(logger->_LogFileName, ptr, write_len);
			lwrb_skip(logger->_RingBuffer, write_len);
		}
		else
		{
			// Not enough for a full block, wait for more data
			break;
		}
	}
}

/**
 * @brief Flush function for the logger.
 * @details
 * Flushes the ring buffer to the SD card and unmounts the card.
 */
static inline void _Flush(void)
{
	_FlushRingBufferToSDCard(&__LOGGER);  // flush ring buffer to SD card
	__LOGGER._SDCardObj->Unmount();  // stop SD card
}

/**
 * @brief Log function for the logger.
 * @details
 * Writes values to the ring buffer and flushes if buffer space is low.
 * @param entry Array of pointers to 4-byte values to log.
 * @param count Number of entries in the array.
 */
static void _Log(void *const*entry, const uint8_t count)
{
	_WriteRawValuesToRingBuffer(entry, count);  // Write values only

	if(lwrb_get_free(__LOGGER._RingBuffer) < 64)
		_FlushRingBufferToSDCard(&__LOGGER);  // flush ring buffer to SD card
}

/**
 * @brief Internal logger object instantiation and function pointer setup.
 * @details
 * Sets up the public logger interface and assigns function pointers for logging and flushing.
 */
static __logger_t __LOGGER =  // instantiate logger_t actual and set function pointers
	{  //
	.public.Log = &_Log,  // set function pointer
	.public.Flush = &_Flush,  // ditto
	};

/**
 * @brief Logger constructor.
 * @details
 * Initializes the logger with the specified log file name and SD card object pointer.
 * @param _LogFileName Name of the log file to write to.
 * @param SDCard Pointer to the SD card object.
 */
void logger_ctor(const char *_LogFileName, void *SDCard)  //
{
	__LOGGER._LogFileName = _LogFileName;  // set log file name pointer
	__LOGGER._RingBuffer = &_RingBufferObject;  // set ring buffer pointer
	__LOGGER._SDCardObj = (sdcard_t*) SDCard;  // store SD card object pointer

	lwrb_init(__LOGGER._RingBuffer, _RingBufferData, LOG_BUFFER_SIZE);  // initialize ring buffer
}

/**
 * @brief Public logger pointer.
 * @details
 * Exposes the logger's public interface for use in application code.
 */
logger_t *const Logger = &__LOGGER.public;  // set pointer to Try public part

#endif
