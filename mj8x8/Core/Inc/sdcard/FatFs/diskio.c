/*-----------------------------------------------------------------------*/
/* Low level disk I/O module SKELETON for FatFs     (C)ChaN, 2025        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various existing      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

#include "ff.h"
#include "diskio.h"
#include "user_diskio_spi.h"

// Only support drive 0 (SD card via SPI)

/**
 * @brief  Gets Disk Status
 * @param  pdrv: Physical drive number (0..) to identify the drive
 * @retval DSTATUS: Operation status
 */
DSTATUS disk_status(BYTE pdrv)
{
	(void) pdrv;  // we only have drive 0

	return USER_SPI_status();
}

/**
 * @brief  Initializes a Drive
 * @param  pdrv: Physical drive number (0..) to identify the drive
 * @retval DSTATUS: Operation status
 */
DSTATUS disk_initialize(BYTE pdrv)
{
	(void) pdrv;  // we only have drive 0

	return USER_SPI_initialize();
}

/**
 * @brief  Reads Sector(s)
 * @param  pdrv: Physical drive number (0..) to identify the drive
 * @param  *buff: Data buffer to store read data
 * @param  sector: Sector address (LBA)
 * @param  count: Number of sectors to read (1..128)
 * @retval DRESULT: Operation result
 */
DRESULT disk_read(BYTE pdrv, BYTE *buff, DWORD sector, UINT count)
{
	(void) pdrv;  // we only have drive 0

	return USER_SPI_read(buff, sector, count);
}

#if FF_FS_READONLY == 0
/**
 * @brief  Writes Sector(s)
 * @param  pdrv: Physical drive number (0..) to identify the drive
 * @param  *buff: Data to be written
 * @param  sector: Sector address (LBA)
 * @param  count: Number of sectors to write (1..128)
 * @retval DRESULT: Operation result
 */
DRESULT disk_write(BYTE pdrv, const BYTE *buff, DWORD sector, UINT count)
{
	(void) pdrv;  // we only have drive 0

	return USER_SPI_write(buff, sector, count);
}
#endif

/**
 * @brief  I/O control operation
 * @param  pdrv: Physical drive number (0..)
 * @param  cmd: Control code
 * @param  *buff: Buffer to send/receive control data
 * @retval DRESULT: Operation result
 */
DRESULT disk_ioctl(BYTE pdrv, BYTE cmd, void *buff)
{
	if(pdrv != 0)
		return RES_PARERR;

	return USER_SPI_ioctl(cmd, buff);
}

/**
 * @brief  Gets Time from RTC
 * @param  None
 * @retval Time in DWORD
 */
DWORD get_fattime(void)
{
	return 0;
}
