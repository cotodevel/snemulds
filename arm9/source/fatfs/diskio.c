/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2014        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

#include "diskio.h"		/* FatFs lower layer API */
#include "dldi.h"
#include "common.h"

/* Definitions of physical drive number for each media */
//#define SDCARD        0
//#define CTRNAND       1
#define DLDICART        0

//coto: support for dldi driver (:

/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
	__attribute__((unused))
	BYTE pdrv		/* Physical drive nmuber to identify the drive */
)
{
	return RES_OK;
}



/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
	BYTE pdrv				/* Physical drive nmuber to identify the drive */
)
{
        DSTATUS ret = 0;
		
		//3DS
        /*
		static u32 sdmmcInitResult = 4;
		
        if(sdmmcInitResult == 4) sdmmcInitResult = sdmmc_sdcard_init();
		
        if(pdrv == CTRNAND)
        {
            if(!(sdmmcInitResult & 1))
            {
                ctrNandInit();
                ret = 0;
            }
            else ret = STA_NOINIT;
        }
        else 
		
		ret = (!(sdmmcInitResult & 2)) ? 0 : STA_NOINIT;
		*/
		
		//DS
		if(pdrv == DLDICART){
			//DS DLDI
			struct DISC_INTERFACE_STRUCT* inst = (struct DISC_INTERFACE_STRUCT*)dldiGetInternal();
		
			if( (!inst->startup()) || (!inst->isInserted()) ){
				ret = STA_NOINIT;
			}
			else{
				ret = 0;	//init OK!
			}
		}
		
	return ret;
}

/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
	BYTE pdrv,		/* Physical drive nmuber to identify the drive */
	BYTE *buff,		/* Data buffer to store read data */
	DWORD sector,	/* Sector address in LBA */
	UINT count		/* Number of sectors to read */
)
{
        //return ((pdrv == SDCARD && !sdmmc_sdcard_readsectors(sector, count, buff)) ||
        //        (pdrv == CTRNAND && !ctrNandRead(sector, count, buff))) ? RES_OK : RES_PARERR;
		
	return ( ((pdrv == DLDICART) && io_dldi_data->ioInterface.readSectors(sector, count, buff) == TRUE) ? RES_OK : RES_ERROR);
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if _USE_WRITE
DRESULT disk_write (
	BYTE pdrv,			/* Physical drive nmuber to identify the drive */
	const BYTE *buff,       	/* Data to be written */
	DWORD sector,		/* Sector address in LBA */
	UINT count			/* Number of sectors to write */
)
{
        //return ((pdrv == SDCARD && !sdmmc_sdcard_writesectors(sector, count, buff)) ||
        //        (pdrv == CTRNAND && !ctrNandWrite(sector, count, buff))) ? RES_OK : RES_PARERR;
		
	return ( ((pdrv == DLDICART) && io_dldi_data->ioInterface.writeSectors(sector, count, buff) == TRUE) ? RES_OK : RES_ERROR);
}
#endif



/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

#if _USE_IOCTL
DRESULT disk_ioctl (
	__attribute__((unused))
	BYTE pdrv,		/* Physical drive nmuber (0..) */
	__attribute__((unused))
	BYTE cmd,		/* Control code */
	__attribute__((unused))
	void *buff		/* Buffer to send/receive control data */
)
{
	return RES_PARERR;
}
#endif
