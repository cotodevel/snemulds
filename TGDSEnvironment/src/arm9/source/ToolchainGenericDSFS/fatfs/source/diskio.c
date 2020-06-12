/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2016        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

#include "diskio.h"		/* FatFs lower layer API */
#include "dldi.h"
#include <stdbool.h>
#include "global_settings.h"

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
		static uint32 sdmmcInitResult = 4;
		
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
			#ifdef ARM7_DLDI
			
			clrscr();
			printf("ARM7 DLDI Init! ");
			u32 targetAddrDLDI7 = (u32)getDLDIARM7Address();
			ARM7DLDIInit(targetAddrDLDI7);
			printf("ARM7 DLDI Done! ");
			
			ret = 0;	//init OK!
			#endif
			
			#ifdef ARM9_DLDI
			if(dldi_handler_init() == true){	//Init DLDI: ARM9 version
				ret = 0;	//init OK!
			}
			else{
				ret = STA_NOINIT;
			}
			#endif
			
		}
		
	return ret;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
	BYTE pdrv,		/* Physical drive nmuber to identify the drive */
	BYTE *buff,		/* Data buffer to store read data */
	DWORD sector,	/* Start sector in LBA */
	UINT count		/* Number of sectors to read */
)
{
	#ifdef ARM7_DLDI
	if(pdrv == DLDICART){
		dldi_handler_read_sectors(sector, count, buff);
		return RES_OK;
	}
	return RES_ERROR;
	#endif
	
	#ifdef ARM9_DLDI
	return ( ((pdrv == DLDICART) && dldi_handler_read_sectors(sector, count, buff) == true) ? RES_OK : RES_ERROR);
	#endif
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

DRESULT disk_write (
	BYTE pdrv,			/* Physical drive nmuber to identify the drive */
	const BYTE *buff,	/* Data to be written */
	DWORD sector,		/* Start sector in LBA */
	UINT count			/* Number of sectors to write */
)
{
	#ifdef ARM7_DLDI
	if(pdrv == DLDICART){
		dldi_handler_write_sectors(sector, count, buff);
		return RES_OK;
	}
	return RES_ERROR;
	#endif
	
	#ifdef ARM9_DLDI
	return ( ((pdrv == DLDICART) && dldi_handler_write_sectors(sector, count, buff) == true) ? RES_OK : RES_ERROR);
	#endif
}



/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT disk_ioctl (
	BYTE pdrv,		/* Physical drive nmuber (0..) */
	BYTE cmd,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{
	return RES_PARERR;
}

