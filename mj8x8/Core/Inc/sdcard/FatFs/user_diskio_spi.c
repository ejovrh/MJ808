/**
 ******************************************************************************
 * @file    user_diskio_spi.c
 * @brief   This file contains the implementation of the user_diskio_spi FatFs
 *          driver.
 ******************************************************************************
 * Portions copyright (C) 2014, ChaN, all rights reserved.
 * Portions copyright (C) 2017, kiwih, all rights reserved.
 *
 * This software is a free software and there is NO WARRANTY.
 * No restriction on use. You can use, modify and redistribute it for
 * personal, non-profit or commercial products UNDER YOUR RESPONSIBILITY.
 * Redistributions of source code must retain the above copyright notice.
 *
 ******************************************************************************
 */

//This code was ported by kiwih from a copyrighted (C) library written by ChaN
//available at http://elm-chan.org/fsw/ff/ffsample.zip
//(text at http://elm-chan.org/fsw/ff/00index_e.html)
//This file provides the FatFs driver functions and SPI code required to manage
//an SPI-connected MMC or compatible SD card with FAT
//It is designed to be wrapped by a STM32CubeMX generated user_diskio.c file.
#include "main.h" /* Provide the low-level HAL functions */
#include "user_diskio_spi.h"
#include "spi/spi.h" // Corrected SPI object header include

/**
 * @enum SD_DataResponseToken
 * @brief SD Card Data Response Token family (Table 4-8)
 */
typedef enum
{
	  SD_DATA_RESP_NONE = 0x00,
	  SD_DATA_RESP_ACCEPTED = 0x05,  // Data accepted
	  SD_DATA_RESP_CRC_ERR = 0x0B,  // CRC error
	  SD_DATA_RESP_WRITE_ERR = 0x0D,  // Write error
} SD_DataResponseToken;

/**
 * @enum SD_DataStartStopToken
 * @brief SD Card Data Start/Stop Token family (Table 4-6, 4-9)
 */
typedef enum
{
	  SD_TOKEN_DATA_START_BLOCK = 0xFE,  // Start single block read/write
	  SD_TOKEN_DATA_START_MULTI = 0xFC,  // Start multi-block write
	  SD_TOKEN_DATA_STOP_TRAN = 0xFD,  // Stop transmission (multi-block write)
} SD_DataStartStopToken;

/**
 * @enum SD_DataErrorToken
 * @brief SD Card Data Error Token family (Table 4-7)
 */
typedef enum
{
	  SD_DATA_ERR_NONE = 0x00,
	  SD_DATA_ERR_ERROR = 0x01,  // General error
	  SD_DATA_ERR_CC_ERROR = 0x02,  // CC error
	  SD_DATA_ERR_ECC_FAILED = 0x04,  // Card ECC failed
	  SD_DATA_ERR_OUT_OF_RANGE = 0x08,  // Out of range
} SD_DataErrorToken;

/**
 * @enum SD_DataBusyReadyToken
 * @brief SD Card Busy/Ready Token family (Section 4.6.2)
 *
 * SD_TOKEN_DATA_BUSY (0x00): Card is busy, not ready for data transfer.
 * SD_TOKEN_DATA_READY (0xFF): Card is ready for data transfer.
 */
typedef enum
{
	  SD_TOKEN_DATA_BUSY = 0x00,  // Card is busy
	  SD_TOKEN_DATA_READY = 0xFF,  // Card is ready
} SD_DataBusyReadyToken;

/* SD Card Data Response Tokens (see SD Physical Layer Simplified Specification, Table 4-8) */
#define SD_DATA_RESP_MASK      0x1F    /* Mask for data response bits (Table 4-8) */
#define SD_DATA_RESP_ACCEPTED  0x05    /* Data accepted (010: Data accepted) */
#define SD_DATA_RESP_CRC_ERR   0x0B    /* CRC error (101: Data rejected due to CRC error) */
#define SD_DATA_RESP_WRITE_ERR 0x0D    /* Write error (110: Data rejected due to write error) */

/* SD Card Data Start/Stop Tokens (see SD Physical Layer Simplified Specification, Table 4-6, 4-9) */
#define SD_TOKEN_DATA_START_BLOCK   0xFE  /* Data Start Token for single block read/write (Table 4-6, 4-9) */
#define SD_TOKEN_DATA_START_MULTI   0xFC  /* Data Start Token for multi-block write (Table 4-9) */
#define SD_TOKEN_STOP_TRAN          0xFD  /* Stop Tran Token for multi-block write (Table 4-9) */

/* SD Card Data Error Token bits (see SD Physical Layer Simplified Specification, Table 4-7) */
#define SD_DATA_ERR_MASK            0x0F  /* Mask for data error token bits (Table 4-7) */
#define SD_DATA_ERR_ERROR           0x01  /* Error (not implemented) */
#define SD_DATA_ERR_CC_ERROR        0x02  /* CC Error (not implemented) */
#define SD_DATA_ERR_ECC_FAILED      0x04  /* Card ECC Failed (not implemented) */
#define SD_DATA_ERR_OUT_OF_RANGE    0x08  /* Out of Range (not implemented) */

/* SD Card Busy/Ready Tokens (see SD Physical Layer Simplified Specification, Section 4.6.2) */
#define SD_TOKEN_BUSY               0x00  /* Card is busy (all bits zero, not implemented) */
#define SD_TOKEN_READY              0xFF  /* Card is ready (not busy, not implemented) */

//Make sure you set #define SD_SPI_HANDLE as some hspix in main.h
//Make sure you set #define SD_CS_GPIO_Port as some GPIO port in main.h
//Make sure you set #define SD_CS_Pin as some GPIO pin in main.h
static uint8_t _dummy_spi_byte = 0xFF;
static uint8_t _rx;
/* Function prototypes */

#define CS_HIGH()	{HAL_GPIO_WritePin(SD_Card_GPIO_Port, SD_Card_CS_Pin, GPIO_PIN_SET);}
#define CS_LOW()	{HAL_GPIO_WritePin(SD_Card_GPIO_Port, SD_Card_CS_Pin, GPIO_PIN_RESET);}

/*--------------------------------------------------------------------------

 Module Private Functions

 ---------------------------------------------------------------------------*/

/* SD Card SPI command set (see SD Physical Layer Simplified Specification) */
#define CMD0    (0)         /* GO_IDLE_STATE: Resets the card to idle state */
#define CMD1    (1)         /* SEND_OP_COND: MMC only, not used for SD cards */
#define ACMD41  (0x80+41)   /* SD_SEND_OP_COND: SD card initialization (after CMD0, with preceding CMD55) */
#define CMD8    (8)         /* SEND_IF_COND: Checks voltage range and SD version (SD v2.0+) */
#define CMD9    (9)         /* SEND_CSD: Read Card-Specific Data register */
#define CMD10   (10)        /* SEND_CID: Read Card Identification register */
#define CMD12   (12)        /* STOP_TRANSMISSION: Stop a multiple block read/write operation */
#define ACMD13  (0x80+13)   /* SD_STATUS: Read SD Status register (with preceding CMD55) */
#define CMD16   (16)        /* SET_BLOCKLEN: Set block length (ignored for SDHC/SDXC, always 512 bytes) */
#define CMD17   (17)        /* READ_SINGLE_BLOCK: Read a single block (sector) */
#define CMD18   (18)        /* READ_MULTIPLE_BLOCK: Read multiple blocks (sectors) */
#define CMD23   (23)        /* SET_BLOCK_COUNT: MMC only, not used for SD cards */
#define ACMD23  (0x80+23)   /* SET_WR_BLK_ERASE_COUNT: Set number of write blocks to be pre-erased (SD, with preceding CMD55) */
#define CMD24   (24)        /* WRITE_BLOCK: Write a single block (sector) */
#define CMD25   (25)        /* WRITE_MULTIPLE_BLOCK: Write multiple blocks (sectors) */
#define CMD32   (32)        /* ERASE_WR_BLK_START: Set start address for erase */
#define CMD33   (33)        /* ERASE_WR_BLK_END: Set end address for erase */
#define CMD38   (38)        /* ERASE: Erase selected range */
#define CMD55   (55)        /* APP_CMD: Indicates next command is application-specific (ACMD) */
#define CMD58   (58)        /* READ_OCR: Read OCR (Operation Conditions Register) */
#define CMD13   (13)        /* SEND_STATUS: Read card status register */
#define ACMD51  (0x80+51)   /* SEND_SCR: Read SD Configuration Register (with preceding CMD55) */

/* MMC card type flags (MMC_GET_TYPE) */
#define CT_MMC      0x01        /* MMC ver 3.x (legacy, not SD) */
#define CT_SD1      0x02        /* SD v1.x (Standard Capacity, byte addressing) */
#define CT_SD2      0x04        /* SD v2.x (SDHC/SDXC if CT_BLOCK is set, block addressing) */
#define CT_SDC      (CT_SD1|CT_SD2) /* Any SD card (v1.x or v2.x) */
#define CT_BLOCK    0x08        /* Block addressing (SDHC/SDXC, sector units) */

static volatile DSTATUS Stat = STA_NOINIT; /* Physical drive status */

static BYTE CardType; /* Card type flags */

uint32_t spiTimerTickStart;
uint32_t spiTimerTickDelay;

/**
 * @brief  Starts the SPI timer for a specified duration.
 * @param  waitTicks Number of ticks to wait.
 */
static inline void SPI_Timer_On(uint32_t waitTicks)
{
	spiTimerTickStart = HAL_GetTick();
	spiTimerTickDelay = waitTicks;
}

/**
 * @brief  Checks if the SPI timer is still running.
 * @retval 1 if timer is running, 0 otherwise.
 */
static inline uint8_t SPI_Timer_Status()
{
	return ((HAL_GetTick() - spiTimerTickStart) < spiTimerTickDelay);
}

/**
 * @brief Waits for a specific response byte from the SD card via SPI within a timeout period.
 * @details This function repeatedly sends dummy bytes to the SD card and reads the response, polling until the expected
 * 					response is received or a timeout occurs.
 * 					It is used to wait for the card to become ready (0xFF), for a data start token (0xFE), or other protocol-specific tokens.
 * 					The chip select state is not changed by this function; the caller is responsible for selecting/de-selecting the card.
 * 					Useful for synchronising with card state transitions and ensuring protocol compliance.
 * @param expected Expected response byte.
 * @param timeout_ms Timeout in ms.
 * @return SD_DataBusyReadyToken Ready/busy status.
 *
 * @note This function is protocol-agnostic and can be used for any SD card SPI polling scenario.
 *       It does not modify the chip select (CS) state; the caller must ensure the card is selected as needed.
 */
static inline SD_DataBusyReadyToken SDCard_WaitForResponse(uint8_t expected, uint32_t timeout_ms)
{
	uint8_t rx;  // variable to store received byte
	uint32_t start = HAL_GetTick();  // get current tick count

	while((HAL_GetTick() - start) < timeout_ms)
		{
			Device->mj8x8->spi->TransmitReceive(&_dummy_spi_byte, &rx, 1);  // send dummy byte and receive response

			if(rx == expected)  // check if response matches expected
				return SD_TOKEN_DATA_READY;  // success

			if(rx == SD_TOKEN_DATA_BUSY)
				continue;  // Card is busy, keep waiting
		}

	return SD_TOKEN_DATA_BUSY;  // Timeout or still busy
}

/**
 * @brief De-selects the SD card and releases the SPI bus.
 * @details Sets the SD card's chip select (CS) pin high to de-select the card, then sends a dummy byte (0xFF)
 *          to provide at least 8 clock cycles as required by the SD Card Physical Layer Specification (Section 4.7.2).
 *          This ensures the card completes its operation before the next command sequence.
 */
static inline void SDCardSPI_DeSelect(void)
{
	CS_HIGH();  // de-select the SPI device
	SDCard_WaitForResponse(0xFF, 10);  // wait for card to go ready
}

/**
 * @brief Selects the SD card and waits for it to become ready.
 * @details Sets the SD card's chip select (CS) pin low to select the card, then calls wait_ready() to poll the card
 *          until it responds with 0xFF (ready) or a timeout occurs. If the card does not become ready, it is de-selected.
 * @retval 1 if the card is ready, 0 if a timeout occurred.
 */
static inline uint8_t SDCardSPI_Select(void)
{
	CS_LOW();  // first, select the card

	if(SDCard_WaitForResponse(0xFF, 10))
		return 1;  // card is ready

	SDCardSPI_DeSelect();
	return 0;  // timeout
}

/**
 * @brief  Receives a data block from the SD card via SPI.
 * @details This function waits for a data start token from the card, then reads the specified number of bytes into the provided buffer.
 * 					After the data block, it discards two CRC bytes as required by the SD protocol.
 * 					If the expected token is not received or a timeout/error occurs, the function returns an appropriate error code.
 * 					This is a blocking operation and is essential for reading sectors, CSD, or CID registers from the card.
 *					Waits for a data start token (0xFE), then reads 'btr' bytes into 'buff'.
 * 					Discards two CRC bytes after the data block.
 *
 * @param buff Pointer to buffer for received data.
 * @param btr Number of bytes to receive (typically 512).
 * @return SD_DataErrorToken (enum):
 *   - SD_DATA_ERR_NONE (0x00): Success, data block received.
 *   - Error codes if token indicates error or timeout.
 *
 * @note  The function blocks until the data block is received or the timeout expires. It does not check the CRC.
 *        The caller must ensure the SD card is selected and ready before calling this function.
 */
// Mask new enum-based functions via preprocessor define
#ifdef SD_ENUM_FUNCTIONS
static SD_DataErrorToken rcvr_datablock(BYTE *buff, uint32_t btr)
{
	uint8_t token;
	uint32_t start = HAL_GetTick();

	do
		{
			Device->mj8x8->spi->TransmitReceive(&_dummy_spi_byte, &token, 1);
			if(token == SD_TOKEN_DATA_START_BLOCK)
				{
					for(uint32_t i = 0; i < btr; i++)
						Device->mj8x8->spi->TransmitReceive(&_dummy_spi_byte, &buff[i], 1);

					uint8_t crc;
					Device->mj8x8->spi->TransmitReceive(&_dummy_spi_byte, &crc, 1);
					Device->mj8x8->spi->TransmitReceive(&_dummy_spi_byte, &crc, 1);

					return SD_DATA_ERR_NONE;
				}
			else if((token & SD_DATA_ERR_MASK) != 0)
				{
					// Return the specific error token
					return (SD_DataErrorToken) (token & SD_DATA_ERR_MASK);
				}
		}
	while((HAL_GetTick() - start) < 200);
	// Timeout: treat as out of range error
	return SD_DATA_ERR_OUT_OF_RANGE;
}
#else
static SD_DataErrorToken rcvr_datablock(BYTE *buff, uint32_t btr)
{
	if(SDCard_WaitForResponse(SD_TOKEN_DATA_START_BLOCK, 200) != SD_TOKEN_DATA_READY)  // Wait for data start token
		return 0;  // Token not received, fail

	for(uint32_t i = 0; i < btr; i++)
		Device->mj8x8->spi->TransmitReceive(&_dummy_spi_byte, &buff[i], 1);  // Receive data block

	// Discard CRC bytes
	uint8_t crc;  // CRC return value
	Device->mj8x8->spi->TransmitReceive(&_dummy_spi_byte, &crc, 1);  // receive first CRC byte
	Device->mj8x8->spi->TransmitReceive(&_dummy_spi_byte, &crc, 1);  // receive second CRC byte

	return 1;  // Success
}
#endif

#if FF_FS_READONLY == 0
/**
 * @brief  Transmits a data block to the SD card via SPI.
 * @details This function waits for the card to be ready, sends the appropriate data token, transmits the data block and two dummy CRC bytes,
 * 					then checks the card's response for acceptance or error. It is used for writing sectors to the card and handles both single and
 * 					multi-block write operations. The function ensures protocol compliance and returns a response code indicating success
 * 					or the type of error encountered.
 * 					This function sends a data or stop token, followed by a 512-byte data block and two dummy CRC bytes,
 * 					then waits for the SD card to respond with a data response token. It is used for writing sectors to the SD card.
 *
 * 					Protocol steps:
 *   					1. Wait for the SD card to be ready (not busy).
 *   					2. Send the data/stop token.
 *   					3. If token is not StopTran (0xFD):
 *      				a. Send the 512-byte data block.
 *      				b. Send two dummy CRC bytes (0xFF).
 *      				c. Receive the data response byte and check if the card accepted the data.
 *
 * @param  buff Data buffer.
 * @param  token Data token.
 * @retval 1 if success, 0 if fail.
 *
 * @note  This function assumes the SD card is already selected and ready for data transmission.
 *        It does not check for write protection or card initialization; caller must ensure these conditions.
 *        For StopTran (0xFD), no data or CRC is sent, only the token.
 *        For other tokens, buff must point to a valid 512-byte data block.
 *        The function uses blocking SPI transfers and is intended for use in FatFs disk I/O routines.
 */
// Mask new enum-based functions via preprocessor define
#ifdef SD_ENUM_FUNCTIONS
static SD_DataResponseToken xmit_datablock(const BYTE *buff, BYTE token)
{
	if(SDCard_WaitForResponse(SD_TOKEN_READY, 10) != SD_TOKEN_DATA_READY)
		return 0;

	Device->mj8x8->spi->TransmitReceive(&token, &_rx, 1);

	if(token == SD_TOKEN_DATA_STOP_TRAN)
		return SD_DATA_RESP_ACCEPTED;

	Device->mj8x8->spi->Transmit((uint8_t*) buff, 512);
	uint8_t dummy_crc = 0xFF;
	Device->mj8x8->spi->Transmit(&dummy_crc, 1);
	Device->mj8x8->spi->Transmit(&dummy_crc, 1);
	uint8_t resp;
	Device->mj8x8->spi->TransmitReceive(&dummy_crc, &resp, 1);

	switch(resp & SD_DATA_RESP_MASK)
		{
		case SD_DATA_RESP_ACCEPTED:
			return SD_DATA_RESP_ACCEPTED;
		case SD_DATA_RESP_CRC_ERR:
			return SD_DATA_RESP_CRC_ERR;
		case SD_DATA_RESP_WRITE_ERR:
			return SD_DATA_RESP_WRITE_ERR;
		default:
			return SD_DATA_RESP_NONE;
		}
}
#else
static SD_DataResponseToken xmit_datablock(const BYTE *buff, BYTE token)
{
// Wait for card to be ready
	if(SDCard_WaitForResponse(SD_TOKEN_READY, 10) != SD_TOKEN_DATA_READY)
		return 0;

	Device->mj8x8->spi->TransmitReceive(&token, &_rx, 1);

	if(token == SD_TOKEN_STOP_TRAN)
		return 1;  // StopTran token, nothing else to do

	// Send data block (512 bytes)
	Device->mj8x8->spi->Transmit((uint8_t*) buff, 512);

	// Send two dummy CRC bytes (required by SD protocol, ignored by most cards)
	uint8_t dummy_crc = 0xFF;  // Dummy CRC byte
	Device->mj8x8->spi->Transmit(&dummy_crc, 1);  // First CRC byte
	Device->mj8x8->spi->Transmit(&dummy_crc, 1);  // Second CRC byte

	// Receive data response
	uint8_t resp;
	Device->mj8x8->spi->TransmitReceive(&dummy_crc, &resp, 1);

	// Check if data accepted (resp & SD_DATA_RESP_MASK == SD_DATA_RESP_ACCEPTED)
	if((resp & SD_DATA_RESP_MASK) != SD_DATA_RESP_ACCEPTED)
		return 0;

	return 1;
}
#endif
#endif

/**
 * @brief Sends a command packet to the SD card via SPI and returns the R1 response.
 * @details
 * This function sends a command to the SD card using the SPI protocol. It handles both standard and application-specific commands (ACMD),
 * manages card selection/de-selection, prepares the command packet, sends it byte-by-byte, and waits for the card's response.
 * Special handling is performed for CMD12 (STOP_TRANSMISSION) and for CRC requirements of CMD0 and CMD8. The function returns the R1 response byte.
 *
 * Protocol steps:
 *   1. If ACMD<n>, send CMD55 first.
 *   2. Select card and wait for ready, except for CMD12.
 *   3. Prepare command packet (start+cmd, arg[31:24], arg[23:16], arg[15:8], arg[7:0], CRC).
 *   4. Send command packet byte-by-byte.
 *   5. For CMD12, discard one byte after command.
 *   6. Wait for R1 response (max SD_CMD_RESPONSE_WAIT_MAX bytes).
 *   7. Return response byte.
 *
 * @param cmd Command index (standard or ACMD).
 * @param arg Command argument (32-bit value).
 * @return BYTE R1 response from SD card.
 */
#define SD_CMD_RESPONSE_WAIT_MAX 10
#define SD_CMD_ERROR_RESPONSE 0xFF

static BYTE send_cmd(uint8_t cmd, uint32_t arg)
{
	uint8_t crc, resp;  // CRC for command packet, response byte
	int wait_count;    // Counter for response wait loop

	if(cmd & 0x80)  // If ACMD<n>, send CMD55 first
		{
			cmd &= 0x7F;  // Clear ACMD bit
			resp = send_cmd(CMD55, 0);  // Send CMD55
			if(resp > 1)
				return resp;  // Return error if CMD55 fails
		}

	// Select card and wait for ready, except for CMD12 (STOP_TRANSMISSION)
	if(cmd != CMD12)
		{
			SDCardSPI_DeSelect();  // Deselect card (CS high)
			if(!SDCardSPI_Select())  // Select card (CS low) and wait for ready
				return SD_CMD_ERROR_RESPONSE;  // Return error if not ready
		}

	// Prepare command packet
	uint8_t cmd_packet[6];  // Command packet buffer
	cmd_packet[0] = 0x40 | cmd;  // Start + command index
	cmd_packet[1] = (uint8_t) (arg >> 24);  // Argument[31..24]
	cmd_packet[2] = (uint8_t) (arg >> 16);  // Argument[23..16]
	cmd_packet[3] = (uint8_t) (arg >> 8);  // Argument[15..8]
	cmd_packet[4] = (uint8_t) arg;         // Argument[7..0]

	// Only CMD0 and CMD8 require valid CRC, others can use dummy
	crc = 0x01;  // Dummy CRC + Stop

	if(cmd == CMD0)
		crc = 0x95;  // Valid CRC for CMD0(0)
	else if(cmd == CMD8)
		crc = 0x87;  // Valid CRC for CMD8(0x1AA)

	cmd_packet[5] = crc;  // CRC

	// Send command packet byte-by-byte
	for(int i = 0; i < 6; i++)
		Device->mj8x8->spi->TransmitReceive(&cmd_packet[i], &_rx, 1);  // Transmit each byte and receive dummy

	// Special case: CMD12 requires discarding one byte after command
	if(cmd == CMD12)
		Device->mj8x8->spi->TransmitReceive(&_dummy_spi_byte, &_rx, 1);  // Discard one byte

	// Wait for response (max SD_CMD_RESPONSE_WAIT_MAX bytes)
	wait_count = SD_CMD_RESPONSE_WAIT_MAX;
	do
		{
			Device->mj8x8->spi->TransmitReceive(&_dummy_spi_byte, &resp, 1);  // Transmit dummy and receive response
		}
	while((resp & 0x80) && --wait_count);  // Wait for MSB to clear

	return resp;  // Return received response
}

/*--------------------------------------------------------------------------

 Public FatFs Functions (wrapped in diskio.c)

 ---------------------------------------------------------------------------*/

//The following functions are defined as inline because they aren't the functions that
//are passed to FatFs - they are wrapped by auto-generated (non-inline) STM32CubeMX template
//code.
//If you do not wish to use STM32CubeMX, remove the "inline" from these functions here
//and in the associated .h
/**
 * @brief Initializes the SD card via SPI interface.
 * @details This function performs the complete SD/MMC card initialization sequence over SPI.
 * 					It sends the required dummy clocks, issues commands to detect card type (SD v1.x, SD v2.x, SDHC/SDXC, MMC),
 * 					checks voltage compatibility, and configures block addressing.
 * 					The function sets global card type and status flags, and must be called before any other SD card operations.
 * 					It ensures the card is ready for use by the FatFs file system and handles all protocol-specific steps for legacy and modern cards.
 * 					Alters global CardType and Stat.
 * @return DSTATUS (disk status flags):
 *   - STA_NOINIT (0x01): Initialization failed or no card detected.
 *   - 0: Initialization successful, card ready.
 *   - May include other disk status flags as defined in FatFs.
 */
inline DSTATUS USER_SPI_initialize(void)
{
	BYTE n, cmd, ty, ocr[4];

	// Assume SPI already initialized
	if(Stat & STA_NODISK)
		return Stat;

	// Send 80 dummy clocks to ensure card is powered up and ready for SPI mode
	for(n = 10; n; n--)
		Device->mj8x8->spi->TransmitReceive(&_dummy_spi_byte, &_rx, 1);

	// Card type detection variable
	ty = 0;

	// CMD0: Put card into idle state
	if(send_cmd(CMD0, 0) == 1)
		{
			SPI_Timer_On(1000); /* Initialization timeout = 1 sec */

			// CMD8: Check SD v2.x support and voltage range
			if(send_cmd(CMD8, 0x1AA) == 1)
				{
					// SD v2.x card detected, read 4 bytes of response
					for(n = 0; n < 4; n++)
						Device->mj8x8->spi->TransmitReceive(&_dummy_spi_byte, &ocr[n], 1);

					// Check if card supports 2.7-3.6V and echo pattern
					if(ocr[2] == 0x01 && ocr[3] == 0xAA)
						{
							// ACMD41: Initialize SD v2.x card with HCS bit
							while(SPI_Timer_Status() && send_cmd(ACMD41, 1UL << 30))
								;

							// CMD58: Read OCR to check CCS bit (SDHC/SDXC detection)
							if(SPI_Timer_Status() && send_cmd(CMD58, 0) == 0)
								{
									for(n = 0; n < 4; n++)
										Device->mj8x8->spi->TransmitReceive(&_dummy_spi_byte, &ocr[n], 1);
									// Set card type: SD v2.x, add CT_BLOCK if SDHC/SDXC
									ty = (ocr[0] & 0x40) ? CT_SD2 | CT_BLOCK : CT_SD2;
								}
						}
				}
			else
				{
					// Not SD v2.x card, try SD v1.x or MMC
					if(send_cmd(ACMD41, 0) <= 1)
						{
							// SD v1.x card detected
							ty = CT_SD1;
							cmd = ACMD41; /* SD v1.x initialization */
						}
					else
						{
							// MMC card detected
							ty = CT_MMC;
							cmd = CMD1; /* MMC initialization */
						}

					// Wait for card to leave idle state
					while(SPI_Timer_Status() && send_cmd(cmd, 0))
						;

					// CMD16: Set block length to 512 bytes (required for SD v1.x/MMC)
					if(!SPI_Timer_Status() || send_cmd(CMD16, 512) != 0)
						ty = 0;
				}
		}
	// Store detected card type
	CardType = ty;

	// De-select card and send dummy clocks
	SDCardSPI_DeSelect();

	// Update status flag
	if(ty)
		Stat = (DSTATUS) ((unsigned char) Stat & (unsigned char) (~STA_NOINIT)); /* Clear STA_NOINIT flag */
	else
		Stat = STA_NOINIT;

	return Stat;
}

/**
 * @brief  Gets the current status of the SD card.
 * @retval DSTATUS: Operation status (disk status flags)
 */
inline DSTATUS USER_SPI_status(void)
{
	return Stat; /* Return disk status */
}

/**
 * @brief  Reads sector(s) from the SD card via SPI.
 * @details
 *   Elegantly reads one or more 512-byte sectors from the SD card using SPI.
 *   Unifies single and multi-sector logic, uses early error handling, and clear buffer management.
 *   Returns FatFs DRESULT code indicating success or error.
 *
 *   Steps:
 *     1. Check if the card is initialised (returns RES_NOTRDY if not).
 *     2. Convert sector address to byte address if required.
 *     3. Send CMD17 (single) or CMD18 (multi) to start read.
 *     4. Loop for each sector:
 *         - Receive 512-byte block into buffer.
 *         - Advance buffer pointer.
 *         - Early exit on error.
 *     5. If multi-sector, send CMD12 to stop transmission.
 *     6. De-select card and return result.
 *
 *   @param buff Pointer to buffer to store read data.
 *   @param sector Sector address (LBA).
 *   @param count Number of sectors to read (1..128).
 *   @return DRESULT FatFs result code (RES_OK, RES_ERROR, RES_NOTRDY).
 */
inline DRESULT USER_SPI_read(BYTE *buff, DWORD sector, UINT count)
{
	// 1. Check initialization
	if(Stat & STA_NOINIT)
		return RES_NOTRDY;

	// 2. Convert sector address for non-block-addressing cards
	if(!(CardType & CT_BLOCK))
		sector *= 512;

	// 3. Select command based on count
	BYTE cmd = (count == 1) ? CMD17 : CMD18;
	if(send_cmd(cmd, sector) != 0)
		{
			SDCardSPI_DeSelect();
			return RES_ERROR;
		}

	// 4. Read sectors in a loop
	DRESULT res = RES_OK;  // Declare and initialize result variable
	for(UINT i = 0; i < count; i++)
		{
			// Attempt to receive a 512-byte data block from the SD card into the buffer
			if(!rcvr_datablock(buff, 512))
				{
					// If data block reception fails, set result to error and exit loop early
					res = RES_ERROR;
					break;
				}
			// Advance buffer pointer to next sector location
			buff += 512;
		}

	// 5. If multi-sector, send CMD12 to stop transmission
	if(count > 1)
		send_cmd(CMD12, 0);

	// 6. De-select card
	SDCardSPI_DeSelect();

	return res;
}

/**
 * @brief  Writes sector(s) to the SD card via SPI.
 * @details
 *   This function writes one or more 512-byte sectors to the SD card using SPI.
 *   It checks card initialization and write protection, converts sector address if needed,
 *   sends the appropriate write command, and transmits data blocks. For multi-sector writes,
 *   it sends a stop token at the end. The function returns a FatFs DRESULT code indicating
 *   success or error. It handles both single and multi-sector writes, and ensures protocol compliance.
 *
 *   Steps:
 *     1. Check if the card is initialised and not write-protected.
 *     2. Convert sector address to byte address if required.
 *     3. Send ACMD23 for pre-erase if multi-sector and SD card.
 *     4. Send CMD24 (single) or CMD25 (multi) to start write.
 *     5. Loop for each sector:
 *         - Transmit 512-byte data block.
 *         - Advance buffer pointer.
 *         - Early exit on error.
 *     6. If multi-sector, send stop token (0xFD).
 *     7. De-select card and return result.
 *
 *   @param buff Pointer to buffer containing data to write.
 *   @param sector Sector address (LBA).
 *   @param count Number of sectors to write (1..128).
 *   @return DRESULT FatFs result code (RES_OK, RES_ERROR, RES_NOTRDY, RES_WRPRT, RES_PARERR).
 */
inline DRESULT USER_SPI_write(const BYTE *buff, DWORD sector, UINT count)
{
	// Check if card is not initialised
	if(Stat & STA_NOINIT)
		return RES_NOTRDY;  // Return not ready if card is not initialised

	// Check if card is write-protected
	if(Stat & STA_PROTECT)
		return RES_WRPRT;  // Return write-protected error

	// Convert sector address to byte address for non-block-addressing cards
	if(!(CardType & CT_BLOCK))
		sector *= 512;  // Multiply sector by 512 for byte addressing

	// Check for zero count (invalid parameter)
	if(count == 0)
		return RES_PARERR;  // Return parameter error

	DRESULT res = RES_ERROR;  // Default result is error
	uint8_t error = 0;        // Error flag for write loop

	// Select write command and data token based on count
	BYTE write_cmd = (count == 1) ? CMD24 : CMD25;  // CMD24 for single, CMD25 for multi
	BYTE data_token = (count == 1) ? 0xFE : 0xFC;  // 0xFE for single, 0xFC for multi

	// For multi-sector write on SD card, send ACMD23 to set pre-erase count
	if(count > 1 && (CardType & CT_SDC))
		send_cmd(ACMD23, count);  // Send ACMD23 with count

	// Send write command (CMD24 or CMD25) with sector address
	if(send_cmd(write_cmd, sector) == 0)
		{
			// Loop to write each sector
			for(UINT i = 0; i < count; i++)
				{
					// Transmit data block to SD card
					if(!xmit_datablock(buff, data_token))
						{
							error = 1;  // Set error flag if write fails
							break;      // Exit loop early on error
						}
					buff += 512;  // Advance buffer pointer to next sector
				}
			// If no error occurred during write
			if(!error)
				{
					// For single sector or successful stop token, set result to OK
					if(count == 1 || xmit_datablock(0, 0xFD))
						res = RES_OK;
				}
		}

	SDCardSPI_DeSelect();  // De-select card after operation
	return res;            // Return result code
}

/**
 * @brief Reads the Card-Specific Data (CSD) register from the SD card.
 * @details
 * This function sends the CMD9 command to the SD card to request the CSD register.
 * It then receives a 16-byte data block containing the CSD contents via SPI.
 * The CSD register contains important information about the card's capacity, block size, and other parameters required for file system operations.
 * Returns 1 on success, 0 on error.
 *
 * Typical usage:
 *   BYTE csd[16];
 *   if (read_csd(csd)) { // use csd contents  }
 *
 * @param csd Pointer to a 16-byte buffer to store the CSD register.
 * @return 1 if successful, 0 if an error occurred.
 */
static inline uint8_t read_csd(BYTE *csd)
{

	if(send_cmd(CMD9, 0) != 0)  // Send CMD9 to request CSD register
		return 0;  // If command fails, return error

	// Receive 16 bytes of CSD data block
	return rcvr_datablock(csd, 16);  // Return 1 if successful, 0 if error
}

/**
 * @brief Parses the Card-Specific Data (CSD) register and extracts sector count and block size.
 * @details
 *   This function analyses the 16-byte CSD register data from an SD/MMC card and extracts the total sector count and block size.
 *   It supports both SD v2.0 (SDHC/SDXC) and SD v1.x/MMC cards, handling the differences in CSD structure and field encoding.
 *   - For SD v2.0 cards, it calculates the sector count using the C_SIZE field and sets block size to 512 bytes.
 *   - For SD v1.x/MMC cards, it extracts the sector count and block size using the appropriate fields and bit manipulations.
 *   The function writes the results to the provided pointers if they are not NULL.
 *   This is useful for file system operations that require knowledge of the card's geometry.
 *
 *   Typical usage:
 *     DWORD sector_count, block_size;
 *     parse_csd_info(csd, &sector_count, &block_size);
 *
 * @param csd Pointer to the 16-byte CSD register data.
 * @param sector_count Pointer to DWORD to store the total sector count (can be NULL).
 * @param block_size Pointer to DWORD to store the block size in bytes (can be NULL).
 */
static inline void parse_csd_info(const BYTE *csd, DWORD *sector_count, DWORD *block_size)
{
	// Check CSD structure version: SD v2.0 (SDHC/SDXC)
	if((csd[0] >> 6) == 1)
		{
			// SD v2.0: Calculate sector count
			DWORD csize = (DWORD) csd[9] + ((DWORD) csd[8] << 8) + ((DWORD) (csd[7] & 63) << 16) + 1;  // Extract C_SIZE field

			if(sector_count)
				*sector_count = csize << 10;  // Sector count = (C_SIZE+1) * 1024

			if(block_size)
				*block_size = 512;  // SDHC/SDXC always 512 bytes per block
		}
	else
		{
			// SD v1.x/MMC: Calculate sector count and block size
			BYTE n = (BYTE) ((csd[5] & 15) + ((csd[10] & 128) >> 7) + ((csd[9] & 3) << 1) + 2);  // Extract READ_BL_LEN and related fields
			DWORD csize = (DWORD) (csd[8] >> 6) + ((DWORD) csd[7] << 2) + ((DWORD) (csd[6] & 3) << 10) + 1;  // Extract C_SIZE field

			if(sector_count)
				*sector_count = csize << (n - 9);  // Sector count = (C_SIZE+1) << (n-9)

			if(block_size)
				{
					if(CardType & CT_SD1)
						*block_size = (DWORD) ((((csd[10] & 63) << 1) + ((WORD) (csd[11] & 128) >> 7) + 1) << ((csd[13] >> 6) - 1));  // SD v1.x block size
					else
						*block_size = (DWORD) (((WORD) ((csd[10] & 124) >> 2) + 1) * (((csd[11] & 3) << 3) + ((csd[11] & 224) >> 5) + 1));  // MMC block size
				}
		}
}

/**
 * @brief  I/O control operation for SD card via SPI.
 * @details
 *   This function performs miscellaneous control operations on the SD card, such as synchronising, retrieving sector count and block size,
 *   trimming (erasing) blocks, and obtaining card type or CSD register. It is typically called by FatFs via disk_ioctl().
 *   The function interprets the control code (cmd) and executes the corresponding operation, updating the buffer (buff) as needed.
 *   It handles card initialization status, command dispatch, and result mapping. Card is de-selected at the end of each operation.
 *
 *   Supported commands:
 *     - CTRL_SYNC: Synchronise the card (flush buffers).
 *     - GET_SECTOR_COUNT: Get total number of sectors.
 *     - GET_BLOCK_SIZE: Get block size in bytes.
 *     - CTRL_TRIM: Erase a range of blocks (if supported).
 *     - MMC_GET_TYPE: Get card type flags.
 *     - MMC_GET_CSD: Get raw CSD register.
 *
 *   Returns FatFs DRESULT codes indicating the result of the operation.
 *
 *   Typical usage:
 *     DRESULT res = USER_SPI_ioctl(CTRL_SYNC, NULL);
 *     DWORD sector_count;
 *     USER_SPI_ioctl(GET_SECTOR_COUNT, &sector_count);
 *
 * @param cmd Control code (FatFs ioctl command).
 * @param buff Pointer to buffer for control data (may be NULL for some commands).
 * @return DRESULT FatFs result code (RES_OK, RES_ERROR, RES_NOTRDY, RES_PARERR, etc).
 */
inline DRESULT USER_SPI_ioctl(const BYTE cmd, void *buff)
{
	DRESULT res = RES_ERROR;  // Default result is error
	BYTE csd[16];  // Buffer for CSD register
	DWORD *dp, st, ed, val;  // Pointers and variables for sector/block operations

	if(Stat & STA_NOINIT)  // Check if card is not initialised
		return RES_NOTRDY;

	switch(cmd)
		{
		case CTRL_SYNC:
			// Synchronise the card (flush buffers)
			if(SDCardSPI_Select())  // Select card and check if ready
				res = RES_OK;  // Success
			break;

		case GET_SECTOR_COUNT:
			// Get total number of sectors
			if(read_csd(csd))  // Read CSD register
				{
					parse_csd_info(csd, &val, NULL);  // Parse sector count from CSD
					*(DWORD*) buff = val;  // Store result in buffer
					res = RES_OK;  // Success
				}
			break;

		case GET_BLOCK_SIZE:
			// Get block size in bytes
			if(read_csd(csd))  // Read CSD register
				{
					parse_csd_info(csd, NULL, &val);  // Parse block size from CSD
					*(DWORD*) buff = val;  // Store result in buffer
					res = RES_OK;  // Success
				}
			break;

		case CTRL_TRIM:
			// Erase a range of blocks (trim)
			if(!(CardType & CT_SDC))  // Only SD cards support trim
				break;

			if(!read_csd(csd))  // Read CSD register
				break;

			if(!(csd[0] >> 6) && !(csd[10] & 0x40))  // Check if erase is supported
				break;

			dp = (DWORD*) buff;  // Get start/end addresses from buffer
			st = dp[0];  // Start sector
			ed = dp[1];  // End sector

			if(!(CardType & CT_BLOCK))  // Convert to byte address if needed
				{
					st *= 512;
					ed *= 512;
				}
			// Send erase commands
			if(send_cmd(CMD32, st) == 0 && send_cmd(CMD33, ed) == 0 && send_cmd(CMD38, 0) == 0 && SDCard_WaitForResponse(0xFF, 10))
				res = RES_OK;  // Success

			break;

		case MMC_GET_TYPE:
			// Get card type flags
			if(buff)
				{
					*(BYTE*) buff = CardType;  // Store card type in buffer
					res = RES_OK;  // Success
				}
			else
				res = RES_PARERR;  // Parameter error
			break;

		case MMC_GET_CSD:
			// Get raw CSD register
			if(buff)
				{
					if(read_csd((BYTE*) buff))  // Read CSD into buffer
						res = RES_OK;  // Success
					else
						res = RES_ERROR;  // Error reading CSD
				}
			else
				res = RES_PARERR;  // Parameter error
			break;

		default:
			res = RES_PARERR;  // Unknown command
		}

	SDCardSPI_DeSelect();  // Always de-select card at end
	return res;  // Return result
}
