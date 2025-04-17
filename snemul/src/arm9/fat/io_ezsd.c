/*
	io_m3sd.c based on io_m3cf.c by SaTa.

	io_m3cf.c based on

	compact_flash.c
	By chishm (Michael Chisholm)

	Hardware Routines for reading a compact flash card
	using the M3 Perfect CF Adapter

	CF routines modified with help from Darkfader

	This software is completely free. No warranty is provided.
	If you use it, please give me credit and email me about your
	project at chishm@hotmail.com

	See gba_nds_fat.txt for help and license details.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "io_ezsd.h"
#include <nds.h>
#include <nds/registers_alt.h>


/********************************************************************************/
void		OpenWrite()
{
	  WAIT_CR &= ~0x80;
	*(vuint16 *)0x9fe0000 = 0xd200;
	*(vuint16 *)0x8000000 = 0x1500;
	*(vuint16 *)0x8020000 = 0xd200;
	*(vuint16 *)0x8040000 = 0x1500;
	*(vuint16 *)0x9C40000 = 0x1500;
	*(vuint16 *)0x9fc0000 = 0x1500;
}

void		CloseWrite()
{
	*(vuint16 *)0x9fe0000 = 0xd200;
	*(vuint16 *)0x8000000 = 0x1500;
	*(vuint16 *)0x8020000 = 0xd200;
	*(vuint16 *)0x8040000 = 0x1500;  
	*(vuint16 *)0x9C40000 = 0xd200;
	*(vuint16 *)0x9fc0000 = 0x1500;
  	WAIT_CR |= 0x80;

}

void  SetNandControl(uint16  control)
{
	*(vuint16 *)0x9fe0000 = 0xd200;
	*(vuint16 *)0x8000000 = 0x1500;
	*(vuint16 *)0x8020000 = 0xd200;
	*(vuint16 *)0x8040000 = 0x1500;
	*(vuint16 *)0x9400000 = control;
	*(vuint16 *)0x9fc0000 = 0x1500;
}

void		dontknow(uint16  number)  //3
{
	*(vuint16 *)0x9fe0000 = 0xd200;
	*(vuint16 *)0x8000000 = 0x1500;
	*(vuint16 *)0x8020000 = 0xd200;
	*(vuint16 *)0x8040000 = 0x1500;
	*(vuint16 *)0x9e00000 = number;
	*(vuint16 *)0x9fc0000 = 0x1500;
}

void		dontknow1()  // 4
{
	WAIT_CR &= ~0x80;
	*(vuint16 *)0x9fe0000 = 0xd200;
	*(vuint16 *)0x8000000 = 0x1500;
	*(vuint16 *)0x8020000 = 0xd200;
	*(vuint16 *)0x8040000 = 0x1500;  
	*(vuint16 *)0x9C40000 = 0x2d00;
	*(vuint16 *)0x9fc0000 = 0x1500;
  	WAIT_CR |= 0x80;

}

void		dontknow2()  // 5
{
	WAIT_CR &= ~0x80;
	*(vuint16 *)0x9fe0000 = 0xd200;
	*(vuint16 *)0x8000000 = 0x1500;
	*(vuint16 *)0x8020000 = 0xd200;
	*(vuint16 *)0x8040000 = 0x1500;  
	*(vuint16 *)0x9C40000 = 0xA200;
	*(vuint16 *)0x9fc0000 = 0x1500;
  	WAIT_CR |= 0x80;

}

void		dontknow3(uint16  number) //6
{
	  WAIT_CR &= ~0x80;
	*(vuint16 *)0x9fe0000 = 0xd200;
	*(vuint16 *)0x8000000 = 0x1500;
	*(vuint16 *)0x8020000 = 0xd200;
	*(vuint16 *)0x8040000 = 0x1500;
	*(vuint16 *)0x9C00000 = number;
	*(vuint16 *)0x9fc0000 = 0x1500;
}

void		dontknow4(uint16  number) // 7
{
	  WAIT_CR &= ~0x80;
	*(vuint16 *)0x9fe0000 = 0xd200;
	*(vuint16 *)0x8000000 = 0x1500;
	*(vuint16 *)0x8020000 = 0xd200;
	*(vuint16 *)0x8040000 = 0x1500;
	*(vuint16 *)0x9880000 = number;
	*(vuint16 *)0x9fc0000 = 0x1500;
}
/********************************************************************************/

u32 SDadd ; //此全局变量为沟通的SD地址

//**功能描述:	用于产生多项式为X^16+X^12+X^5+1的CRC码
//**用于快速产生CRC16的结果
const unsigned short wCRCTalbeAbs[] =
{
	0x0000, 0xCC01, 0xD801, 0x1400, 0xF001, 0x3C00, 0x2800, 0xE401, 0xA001, 0x6C00, 0x7800, 0xB401, 0x5000, 0x9C01, 0x8801, 0x4400,
};

unsigned short CRC16_2( unsigned char* pchMsg, unsigned short wDataLen)
{
        unsigned short wCRC = 0xFFFF;
        unsigned short i;
        unsigned char chChar;

        for (i = 0; i < wDataLen; i++)
        {
                chChar = *pchMsg++;
                wCRC = wCRCTalbeAbs[(chChar ^ wCRC) & 15] ^ (wCRC >> 4);
                wCRC = wCRCTalbeAbs[((chChar >> 4) ^ wCRC) & 15] ^ (wCRC >> 4);
        }

        return wCRC;
}

//**功能描述:	用于产生多项式为X^16+X^12+X^5+1的CRC码
unsigned int cal_crc_CCITT(unsigned char *ptr,unsigned char len)
{
	unsigned char i;
	unsigned int  crc=0;
	while(len--!=0)
	{
		for(i=0x80;i!=0;i/=2)
		{
			if((crc & 0x8000)!=0)
			{
				crc*=2;
				crc^=0x1021;
			}		
			else
			{
				crc*=2;
			}
			if((*ptr & i)!=0)
				crc ^= 0x1021;
		}
		ptr++;
	}
	return crc;
}
//**功能描述:	用于产生多项式为X^7+X^3+1的CRC码
unsigned char cal_crc_730(unsigned char *ptr,unsigned char len)
{
	unsigned char i;
	unsigned char  crc=0;
	while(len--!=0)
	{
		for(i=0x80;i!=0;i/=2)
		{
			if((crc & 0x40)!=0)
			{
				crc*=2;
				crc^=0x09;
			}		
			else
			{
				crc*=2;
			}
			if((*ptr & i)!=0)
				crc ^= 0x09;
		}
		ptr++;
	}
	return crc;
}

void  SD_SendCommand (int type , unsigned int param )
{
	unsigned char ppbuf[8];
	unsigned char crc = 0 ;
	ppbuf[0] = 0x40 | (type&0xFF) ;
	ppbuf[1] = (param>>24)&0xFF ;
	ppbuf[2] = (param>>16)&0xFF ;
	ppbuf[3] = (param>>8)&0xFF ;
	ppbuf[4] = (param)&0xFF ;
	crc = cal_crc_730(ppbuf,5);
	ppbuf[5] = (crc<<1)|0x1 ;
	SD_WriteBufferToLine(ppbuf,6);
}
bool SD_R16Response(unsigned char *ppbuf, int wait)
{
	return SD_ReadResponse(ppbuf,6,0x10000);
}

bool SD_R2Response(unsigned char *ppbuf, int wait)
{
	return SD_ReadResponse(ppbuf,17,0x10000);
}

bool SD_R3Response(unsigned char *ppbuf, int wait)
{
	bool ret ;
	ret = SD_ReadResponse(ppbuf,6,0x10000);
	if(ret)
	{
		if((ppbuf[0]!=0x3F)&&(ppbuf[5]!=0xFF))
			return false ;
		return true ;
	}
	return false ;
}
void SD_ReadLoop(int lp)
{
	int i ,k;
	for(i=0;i<lp;i++)
		k = *(vu16*)0x9FFFF40 ;
}

bool SD_WaitDataline(int loop)
{
	unsigned short sk=0xf ;
	if(loop==0)
	{
		do
		{
			sk = 0xF00&(*(vu16*)0x9FEA000) ;
		}
		while(!sk);
		if(sk) return true ;
		else   return false ;
	}
	else
	{
		do
		{
			sk = 0xF00&(*(vu16*)0x9FEA000) ;
			loop -- ;
		}while(loop & (!sk)) ;
		if(!loop)	return false ;
		if(sk) return true ;
	}
	return true ;
}
bool SD_ReadSingleBlock(unsigned int address , unsigned char *ppbuf, int len)
{
	SD_SendCommand(17,address); // single block  read , parm = address
	SD_ReadData(ppbuf,len,0x100000);
	SD_ReadLoop(8);
	return true ;

}

bool SD_ReadMultiBlock(unsigned int address , unsigned char *ppbuf, int len)
{
	unsigned char pp[534] ;
	int off = 0 ;
	unsigned char p[8];
	SD_SendCommand(18,address); // single block  read , parm = address
	do
	{
		if(off)
			SD_WaitDataline(0);
		SD_ReadData(pp,524,0x100000);
		memcpy(ppbuf+off,pp,512);  
		off += 512 ;
	}
	while(off<len);
	SD_SendCommand(12,0);	//写结束，发送Stop命令
	SD_R16Response(p,0);	
	SD_ReadLoop(8);
	return true ;

}

bool SD_GetCSDStruct(unsigned char * ppbuf , int len , SD_CSD *pCsd)
{
	//const float fay[]={0 , 1.0 , 1.2 , 1.3 , 1.5 , 2.0 , 2.5 , 3.0 , 3.5 , 4.0 , 4.5 , 5.0 , 5.5 , 6.0 , 7.0 , 8.0 };
	if(!ppbuf) return false;
	if(len!=17) return false ;
	SD_CSDREAL *preal ;
	preal = (SD_CSDREAL *)&ppbuf[1] ;
	pCsd->CSDStruct = preal->CSD_STRUCTURE ;
	pCsd->Taac.TimeUnit = (CSD_TAAC_TIMEUNIT)((preal->TAAC)&0x7);
	pCsd->Taac.fTimeValue = (preal->TAAC&0x7F)>>3 ; 
	pCsd->byNsac = preal->NSAC ;
	pCsd->TranSpeed.transfer_rate = (CSD_TRAN_SPEEDUNIT)((preal->TRAN_SPEED)&0x7) ; 
	pCsd->TranSpeed.fTimeValue = (preal->TRAN_SPEED&0x7F)>>3 ;
	pCsd->wCCC = (preal->CCC_H)*0x10 +  (preal->CCC_L) ;
	pCsd->bRead_Bl_Len = preal->READ_BL_LEN ;
	pCsd->bRead_Bl_Partial = preal->READ_BL_PARTIAL ;
	pCsd->bWrite_Blk_Misalign = preal->WRITE_BLK_MISALIGN ;
	pCsd->bRead_Blk_Misalign  = preal->READ_BLK_MISALIGN ;
	pCsd->bDsr_Imp = preal->DSR_IMP ;
	pCsd->wC_Size = ((preal->C_SIZE_H2)<<10)+((preal->C_SIZE_H1)<<2)+preal->C_SIZE_L ;
	pCsd->byVdd_R_Curr_Min = preal->VDD_R_CURR_MIN ;
	pCsd->byVdd_R_Curr_Max = preal->VDD_R_CURR_MAX ;
	pCsd->byVdd_W_Curr_Min = preal->VDD_W_CURR_MIN;
	pCsd->byVdd_W_Curr_Max = preal->VDD_W_CURR_MAX ;
	BYTE s = ((preal->C_SIZE_MULT_H)<<2) + preal->C_SIZE_MULT_L ;
	pCsd->wC_Size_Mult = 1<<(s+2) ;
	pCsd->bErase_Blk_En = preal->ERASE_BLK_EN ;
	pCsd->bySector_Size = preal->SECTOR_SIZE_H*2 + preal->SECTOR_SIZE_L;
	pCsd->byWp_Grp_Size = preal->WP_GRP_SIZE ;
	pCsd->bWp_Grp_Enable = preal->WP_GRP_ENABLE ;
	pCsd->byR2w_Factor = preal->R2W_FACTOR ;
	pCsd->byWrite_Bl_Len = preal->WRITE_BL_LEN_H * 4 + preal->WRITE_BL_LEN_L ;
	pCsd->bWrite_Bl_Partial = preal->WRITE_BL_PARTIA ;
	pCsd->byFile_Format_Grp = preal->FILE_FORMAT_GRP ;
	pCsd->bCopy = preal->COPY ;
	pCsd->byPerm_Write_Protect = preal->PERM_WRITE_PROTECT ;
	pCsd->byTmp_Write_Protect = preal->TMP_WRITE_PROTECT ;
	pCsd->File_Format = (CSD_FILE_FORMAT)preal->FILE_FORMAT ;
	return true ;
}

bool		SD_GetCIDStruct(unsigned char * ppbuf , int len , SD_CID *pCid)
{
	BYTE *p;
	if(!ppbuf)	return false ;
	if(len != 17) return false ;
	memset(pCid,0,sizeof(SD_CID));
	p = &ppbuf[0];
	pCid->byManufacturerID = p[0];
	pCid->wOemID = *((WORD *) &p[1]);
	memcpy(pCid->szProductName,&p[3],5);
	pCid->byProductReision = p[8];
	pCid->dwProductSn = *((DWORD *) &p[9]);
	pCid->wData = *((WORD *) &p[13]);
	pCid->byCRC = (p[15] & 0xFE);
	//this is just a test 
	return true;
	
}

bool SD_initial()
{

	bool  ret ;
	unsigned char pres[40] ;
	SD_CID	cid ;	
	SD_CSD csd ;
//	char ptmp[64];

	SD_ReadLoop(147);
	SD_SendCommand(0,0);
	SD_ReadLoop(8);

	SD_SendCommand(55,0);   // 指示下一个命令是app命令
	ret = SD_R16Response(pres,0);	

	if(ret)
	{
		SD_SendCommand(41,0x0); //检测电压范围
		ret = SD_R16Response(pres,0);
		do
		{
			SD_SendCommand(55,0);
			ret = SD_R16Response(pres,0);
			SD_SendCommand(41,0xFC0000);
			ret = SD_R3Response(pres,0);
		}while(pres[1]!=0x80) ;
	}
	if(ret)
	{
		SD_SendCommand(2,0);	//Get CID 
		ret = SD_R2Response(pres,0);
		SD_GetCIDStruct(pres,17,&cid);
	}
	if(ret)
	{
		do
		{
			SD_SendCommand(3,0);		//进入stand by 状态，并得到sd卡状态
			ret = SD_R16Response(pres,0);
		}while((pres[3]&0x1E) != 0x6); //stand by state
	}
	if(ret)
	{
		SDadd = pres[1]*0x100 + pres[2] ;
		SD_SendCommand(9,(SDadd<<16));//send_csd
		ret = SD_R2Response(pres,0);
		SD_GetCSDStruct(pres,17,&csd);
	}
	if(ret)
	{
		SD_SendCommand(7,SDadd<<16);	//select card
		ret = SD_R16Response(pres,0);	
	}
	if(ret)
	{
		SD_SendCommand(55,SDadd<<16); //app command
		ret = SD_R16Response(pres,0);
		SD_SendCommand(6,2);		//00, 1 bit , 10  4 bit 
		ret = SD_R16Response(pres,0);
	}
	if(ret)
	{
		SD_SendCommand(16,0x200) ;	//设定一个block为512大小
		ret = SD_R16Response(pres,0);
	}
	return true;
}
//======================================================
bool EZSD_read1sector(u32 sectorn,u32 TAddr)
{
	
	return true;
	
} 
//==================================================


//======================================================
bool EZSD_write1sector(u32 sectorn,u32 TAddr)
{
	return true;
} 
//==================================================

/*-----------------------------------------------------------------
M3SD_IsInserted
Is a compact flash card inserted?
bool return OUT:  true if a CF card is inserted
-----------------------------------------------------------------*/
bool EZSD_IsInserted (void) 
{
	return true;
}


/*-----------------------------------------------------------------
M3SD_ClearStatus
Tries to make the CF card go back to idle mode
bool return OUT:  true if a CF card is idle
-----------------------------------------------------------------*/
bool EZSD_ClearStatus (void) 
{
	return true;
}


/*-----------------------------------------------------------------
M3SD_ReadSectors
Read 512 byte sector numbered "sector" into "buffer"
u32 sector IN: address of first 512 byte sector on CF card to read
u8 numSecs IN: number of 512 byte sectors to read,
 1 to 256 sectors can be read, 0 = 256
void* buffer OUT: pointer to 512 byte buffer to store data in
bool return OUT: true if successful
-----------------------------------------------------------------*/
bool EZSD_ReadSectors (u32 sector, u8 numSecs, void* buffer)
{
	SD_ReadMultiBlock(sector*0x200,buffer,numSecs*0x200);
	return true;
}



/*-----------------------------------------------------------------
M3SD_WriteSectors
Write 512 byte sector numbered "sector" from "buffer"
u32 sector IN: address of 512 byte sector on CF card to read
u8 numSecs IN: number of 512 byte sectors to read,
 1 to 256 sectors can be read, 0 = 256
void* buffer IN: pointer to 512 byte buffer to read data from
bool return OUT: true if successful
-----------------------------------------------------------------*/
bool EZSD_WriteSectors (u32 sector, u8 numSecs, void* buffer)
{
	return true;
}

/*-----------------------------------------------------------------
M3_Unlock
Returns true if M3 was unlocked, false if failed
Added by MightyMax
-----------------------------------------------------------------*/
//bool M3SD_Unlock(void) 
//{
  // return false;
//}

bool EZSD_Shutdown(void) {
	CloseWrite();
	SetNandControl(0);
	return true;
} ;

bool EZSD_StartUp(void) {
	//init sd 
	OpenWrite();
	SetNandControl(1);
	return SD_initial();
} ;


IO_INTERFACE io_ezsd = {
	0x44535A45,	// 'EZSD'
	FEATURE_MEDIUM_CANREAD | FEATURE_MEDIUM_CANWRITE,
	(FN_MEDIUM_STARTUP)&EZSD_StartUp,
	(FN_MEDIUM_ISINSERTED)&EZSD_IsInserted,
	(FN_MEDIUM_READSECTORS)&EZSD_ReadSectors,
	(FN_MEDIUM_WRITESECTORS)&EZSD_WriteSectors,
	(FN_MEDIUM_CLEARSTATUS)&EZSD_ClearStatus,
	(FN_MEDIUM_SHUTDOWN)&EZSD_Shutdown
} ;


LPIO_INTERFACE EZSD_GetInterface(void) {
	return &io_ezsd ;
} ;
