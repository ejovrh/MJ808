/**
 ******************************************************************************
 * @file    user_diskio_spi.h
 * @brief   This file contains the common defines and functions prototypes for
 *          the user_diskio_spi driver implementation
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

#ifndef _USER_DISKIO_SPI_H
#define _USER_DISKIO_SPI_H

#include "diskio.h" //from FatFs middle-ware library

DSTATUS USER_SPI_initialize(void);  // initialize a drive
DSTATUS USER_SPI_status(void);  // gets disk status
DRESULT USER_SPI_read(BYTE *buff, DWORD sector, UINT count);  // read sector(s)
#if FF_FS_READONLY == 0
DRESULT USER_SPI_write(const BYTE *buff, DWORD sector, UINT count);  // write sector(s)
#endif /* FF_FS_READONLY == 0 */
DRESULT USER_SPI_ioctl(const BYTE cmd, void *buff);  // I/O control operation

#endif /* _USER_DISKIO_SPI_H */
