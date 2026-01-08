/**
 * @file logger.h
 * @brief Logger interface for MJ808 project.
 * @details
 * This header defines the public API for the logger module, which provides
 * logging functionality to a ring buffer and SD card. The logger supports
 * logging raw binary values and flushing the buffer to persistent storage.
 */
#ifndef CORE_INC_LOGGER_LOGGER_H_
#define CORE_INC_LOGGER_LOGGER_H_

#include "main.h"

#if USE_SPI && USE_LOGGER // Only compile if SPI and LOGGER are enabled

/**
 * @struct logger_t
 * @brief Struct describing the Logger functionality.
 * @details
 * Contains function pointers for logging entries and flushing the ring buffer
 * to the SD card. Intended to be used as the public interface for the logger.
 */
typedef struct
{
    /**
     * @brief Log entries to the ring buffer.
     * @details
     * Writes an array of entries to the logger's ring buffer. Each entry is
     * expected to be a pointer to a 4-byte value. The count parameter specifies
     * the number of entries to log.
     * @param entry Array of pointers to values to log.
     * @param count Number of entries in the array.
     */
    void (*Log)(void *const*entry, const uint8_t count);

    /**
     * @brief Flush the ring buffer to the SD card.
     * @details
     * Writes all buffered log entries to the SD card and unmounts the card.
     */
    void (*Flush)(void);
} logger_t;

/**
 * @brief Logger constructor.
 * @details
 * Initializes the logger with the specified log file name and SD card object pointer.
 * @param _LogFileName Name of the log file to write to.
 * @param SDCard Pointer to the SD card object.
 */
void logger_ctor(const char *_LogFileName, void *SDCard);

/**
 * @brief Pointer to the public logger struct.
 * @details
 * Exposes the logger's public interface for use in application code.
 */
extern logger_t *const Logger;

#endif

#endif /* CORE_INC_LOGGER_LOGGER_H_ */