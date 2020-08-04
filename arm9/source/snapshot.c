/***********************************************************/
/* This source is part of SNEmulDS                         */
/* ------------------------------------------------------- */
/* (c) 1997-1999, 2006-2007 archeide, All rights reserved. */
/***********************************************************/
/*
This program is free software; you can redistribute it and/or 
modify it under the terms of the GNU General Public License as 
published by the Free Software Foundation; either version 2 of 
the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, 
but WITHOUT ANY WARRANTY; without even the implied warranty of 
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the 
GNU General Public License for more details.
*/

#include <malloc.h>
#include <string.h>
#include "ipcfifoTGDSUser.h"
#include "posixHandleTGDS.h"

#include "common.h"
#include "core.h"
#include "snes.h"
#include "apu.h"
#include "gfx.h"
#include "cfg.h"
#include "snapshot.h"

int		get_snapshot_name(char *file, uchar nb, char *name){
	FILE *f = NULL;
	if (nb > 8){
		return 0;
	}
	file[strlen(file)-1] = '0'+nb; // XXX.SM1 XXXX.SM2 ....
	FS_lock();
	if ((f = fopen(file, "r")) == NULL)
	{
		FS_unlock();
		return 0;
	}
	TSnapShot_Header *header = (TSnapShot_Header *)malloc(sizeof(TSnapShot_Header));
	fread(header, sizeof(TSnapShot_Header), 1, f);

	char header_name[17];
	memcpy(header_name, header->name, 16);
	header_name[16] = 0;
	sprintf(name, "Save #%d - %s", nb, header_name);

	free(header);
	fclose(f);
	FS_unlock();
	return 1;
}

bool	read_snapshot(char *file, uchar nb){
	FILE *f=NULL;
	int	i;
	if (nb > 8){
		return false;
	}
	file[strlen(file)-1] = '0'+nb; // XXX.SM1 XXXX.SM2 ....
	FS_lock();
	f = fopen(file, "r");
	if(f == NULL)
	{
		FS_unlock();
		return false;
	}
	
	TSnapShot_Header *header = (TSnapShot_Header *)malloc(sizeof(TSnapShot_Header));
	fread(header, 1, sizeof(TSnapShot_Header), f);
	
	fread(SNESC.RAM, 1, 0x20000, f);
	coherent_user_range_by_size((uint32)SNESC.RAM, (int)0x20000);	
	
	fread(SNESC.VRAM, 1, 0x10000, f);
	coherent_user_range_by_size((uint32)SNESC.VRAM, (int)0x10000);
	
	fread(SNESC.SRAM, 1, 0x8000, f);
	coherent_user_range_by_size((uint32)SNESC.SRAM, (int)0x8000);
	
	for (i = 0; i < 256; i++)
	{
		uint8	pal[4];
		fread(pal, 1, 3, f);
		coherent_user_range_by_size((uint32)&pal, 4);
		GFX.SNESPal[i] = (pal[2]>>1)|((pal[1]>>1)<<5)|((pal[0]>>1)<<10);
	}

	/*  fread(PPU_PORT, 1, 2*0x100, f);
	fread(DMA_PORT, 1, 2*0x200, f);*/
	
	fread(PPU_PORT, 1, 2*0x90, f);
	coherent_user_range_by_size((uint32)&PPU_PORT[0], 2*0x90);
	
	fread(EMPTYMEM, 1, 2*0x70, f); // unused
	coherent_user_range_by_size((uint32)EMPTYMEM, 2*0x70);
	
	fread(DMA_PORT, 1, 2*0x180, f);
	coherent_user_range_by_size((uint32)&DMA_PORT[0], 2*0x180);
	
	fread(EMPTYMEM, 1, 2*0x80, f); // unused
	coherent_user_range_by_size((uint32)EMPTYMEM, 2*0x80);
	
	TSnapShot *snapshot = (TSnapShot *)malloc(sizeof(TSnapShot));
	fread(snapshot, 1, sizeof(TSnapShot), f);
	
	//GUI_printf("PC =  %02X:%04x\n", snapshot->PB, snapshot->PC);
	
	CPU.A  = snapshot->A;  CPU.X  = snapshot->X;  CPU.Y  = snapshot->Y;
	CPU.S  = snapshot->S;  CPU.P  = snapshot->P;  CPU.D  = snapshot->D;
	CPU.PB = snapshot->PB; CPU.DB = snapshot->DB; CPU.PC = snapshot->PC;
	GFX.BG_scroll_reg = snapshot->BG_scroll_reg;
	GFX.SC_incr = snapshot->SC_incr; GFX.FS_incr = snapshot->FS_incr;
	GFX.OAM_upper_byte = snapshot->OAM_upper_byte;
	SNES.PPU_NeedMultiply = snapshot->PPU_NeedMultiply;

	// FIXME: we should also save sprite infos
	if (snapshot->options[0] >= 1)
	{
		unpackOptions(snapshot->options[0], &snapshot->options[1]); 	
	}

	if (CFG.Sound_output) {
		u8 * spcFile = malloc(0x10200);
		memset(spcFile, 0, 0x10200);
		if(spcFile == NULL){
			return false;
		}
		APU_stop(); // Make sure that the APU is *completely* stopped
		// Read SPC file format
		fread(spcFile, 1, 0x10200, f);
		APU_loadSpc(spcFile);	//blocking, wait APU init
		//int StructFD = fileno(f);
		//struct fd *fdinst = getStructFD(StructFD);
		//int APUOffset = fdinst->filPtr->fptr;
		//GUI_printf("read_snapshot() : %s \n", file);
		//GUI_printf("APU payload offset write@ %x \n", APUOffset);
		free(spcFile);
	}

	free(snapshot);
	free(header);
	fclose(f);
	FS_unlock();
	
	GFX.tiles_dirty = 1;
	GFX.Sprites_table_dirty = 1;
	CPU.unpacked = 0; // Update ASM  
	return true;
}

bool write_snapshot(char *file, unsigned char nb, const char *name){
	FILE *f=NULL;
	int	i;
	if (nb > 8){ 
		return false;
	}
	file[strlen(file)-1] = '0'+nb; // XXX.SM1 XXXX.SM2 ....
	FS_lock();
	
	f = fopen(file, "w+");
	if(f == NULL)
	{
		FS_unlock();
		return false;
	}
	
	TSnapShot_Header *header = (TSnapShot_Header *)malloc(sizeof(TSnapShot_Header));
	strcpy(header->name, name);  
	fwrite(header, 1, sizeof(TSnapShot_Header), f);
	
	coherent_user_range_by_size((uint32)SNESC.RAM, 0x20000);
	fwrite(SNESC.RAM,  1, 0x20000, f);
	
	coherent_user_range_by_size((uint32)SNESC.VRAM, 0x10000);
	fwrite(SNESC.VRAM, 1, 0x10000, f);
	
	coherent_user_range_by_size((uint32)SNESC.SRAM, 0x8000);
	fwrite(SNESC.SRAM, 1, 0x8000, f);
	for (i = 0; i < 256; i++){
		uint8	pal[4];
		pal[2] = GFX.SNESPal[i]<<1;
		pal[1] = (GFX.SNESPal[i]>>5)<<1;
		pal[0] = (GFX.SNESPal[i]>>10)<<1;
		coherent_user_range_by_size((uint32)&pal, 4);
		fwrite(pal, 1, 3, f);
	}
	
	coherent_user_range_by_size((uint32)&PPU_PORT[0], 2*0x90);
	fwrite(PPU_PORT, 1, 2*0x90, f);
	
	coherent_user_range_by_size((uint32)EMPTYMEM, 2*0x70);
	fwrite(EMPTYMEM, 1, 2*0x70, f); // unused
	
	coherent_user_range_by_size((uint32)&DMA_PORT[0], 2*0x180);
	fwrite(DMA_PORT, 1, 2*0x180,f);
	
	coherent_user_range_by_size((uint32)EMPTYMEM, 2*0x80);
	fwrite(EMPTYMEM, 1, 2*0x80, f); // unused

	TSnapShot *snapshot = (TSnapShot *)malloc(sizeof(TSnapShot));
	snapshot->A = CPU.A; snapshot->X = CPU.X; snapshot->Y = CPU.Y;
	snapshot->S = CPU.S; snapshot->P = CPU.P; snapshot->D = CPU.D;
	snapshot->PB = CPU.PB; snapshot->DB = CPU.DB; snapshot->PC = CPU.PC;

	snapshot->BG_scroll_reg = GFX.BG_scroll_reg;
	snapshot->SC_incr = GFX.SC_incr; snapshot->FS_incr = GFX.FS_incr;
	snapshot->OAM_upper_byte = GFX.OAM_upper_byte;
	snapshot->PPU_NeedMultiply = SNES.PPU_NeedMultiply;

	snapshot->options[0] = 2;
	//	GUI_printf("\nUpdate Options\n");
	packOptions(&snapshot->options[1]);
	//GUI_console_printf(0, 23, "State written");
	coherent_user_range_by_size((uint32)snapshot, sizeof(TSnapShot));
	fwrite(snapshot, 1, sizeof(TSnapShot), f);

	if (CFG.Sound_output) {
		u8 * spcFile = malloc(0x10200);
		memset(spcFile, 0, 0x10200);
		if(spcFile == NULL){
			return false;
		}
		APU_pause();	//APU_stop(); // Make sure that the APU is *completely* stopped
		APU_saveSpc(spcFile);	//blocking, wait APU init
		//int StructFD = fileno(f);
		//struct fd *fdinst = getStructFD(StructFD);
		//int APUOffset = fdinst->filPtr->fptr;
		//GUI_printf("write_snapshot() : %s \n", file);
		//GUI_printf("APU payload offset write@ %x \n", APUOffset);
		fwrite(spcFile, 1, 0x10200, f);
		free(spcFile);
		APU_pause();	//restore apu
	}
	free(snapshot);
	free(header);
	fclose(f);
	FS_unlock();
	return true;
}
