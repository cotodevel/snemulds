#include <nds.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>//BRK(); SBRK();
#include <nds/ndstypes.h>
#include <nds/memory.h>
#include <nds/bios.h>
#include <nds/system.h>
#include <fcntl.h>
#include <fat.h>
#include <sys/stat.h>

//Protection Unit calls, wrappers, go here.
#include "pu.h"

#include "../main.h"
#include "../opcodes.h"

/* exception vector abort handlers
arm variables		status

exceptionrst 		working OK!
exceptionundef		working OK!
exceptionswi		working OK!
exceptionprefabt	working OK!
exceptiondatabt		working OK!
exceptionreserv		working OK!
exceptionirq		working OK!
exceptionfiq		working OK!
*/

/*Processor modes

User	usr	%10000	Normal program execution, no privileges	All
FIQ	fiq	%10001	Fast interrupt handling	All
IRQ	irq	%10010	Normal interrupt handling	All
Supervisor	svc	%10011	Privileged mode for the operating system	All
Abort	abt	%10111	For virtual memory and memory protection	ARMv3+
Undefined	und	%11011	Facilitates emulation of co-processors in hardware	ARMv3+
System	sys	%11111	Runs programs with some privileges	ARMv4+

0x10 usr
0x11 fiq
0x12 irq
0x13 svc
0x17 abt
0x1b und
0x1f sys
*/

/* TGBA (LD memorymap settings)
rom     : ORIGIN = 0x08000000, LENGTH = 32M
gbarom  : ORIGIN = 0x02180000, LENGTH = 3M -512k
gbaew   : ORIGIN = 0x02000000, LENGTH = 256k
ewram   : ORIGIN = 0x02040000, LENGTH = 1M - 256k + 512k = 0x140000 = 1280KB
dtcm    : ORIGIN = 0x027C0000, LENGTH = 16K
vectors : ORIGIN = 0x01FF8000, LENGTH = 16K
itcm    : ORIGIN = 0x01FFC000, LENGTH = 16K
*/

/*
 GBA Memory Map

General Internal Memory
  00000000-00003FFF   BIOS - System ROM         (16 KBytes)
  00004000-01FFFFFF   Not used
  02000000-0203FFFF   WRAM - On-board Work RAM  (256 KBytes) 2 Wait
  02040000-02FFFFFF   Not used
  03000000-03007FFF   WRAM - On-chip Work RAM   (32 KBytes)
  03008000-03FFFFFF   Not used
  04000000-040003FE   I/O Registers
  04000400-04FFFFFF   Not used
Internal Display Memory
  05000000-050003FF   BG/OBJ Palette RAM        (1 Kbyte)
  05000400-05FFFFFF   Not used
  06000000-06017FFF   VRAM - Video RAM          (96 KBytes)
  06018000-06FFFFFF   Not used
  07000000-070003FF   OAM - OBJ Attributes      (1 Kbyte)
  07000400-07FFFFFF   Not used
External Memory (Game Pak)
  08000000-09FFFFFF   Game Pak ROM/FlashROM (max 32MB) - Wait State 0
  0A000000-0BFFFFFF   Game Pak ROM/FlashROM (max 32MB) - Wait State 1
  0C000000-0DFFFFFF   Game Pak ROM/FlashROM (max 32MB) - Wait State 2
  0E000000-0E00FFFF   Game Pak SRAM    (max 64 KBytes) - 8bit Bus width
  0E010000-0FFFFFFF   Not used
*/

/* defaults (NDS)
	Region  Name            Address   Size   Cache WBuf Code Data
	-       Background      00000000h 4GB    -     -    -    -
	0       I/O and VRAM    04000000h 64MB   -     -    R/W  R/W
	1       Main Memory     02000000h 4MB    On    On   R/W  R/W
	2       ARM7-dedicated  027C0000h 256KB  -     -    -    -
	3       GBA Slot        08000000h 128MB  -     -    -    R/W
	4       DTCM            027C0000h 16KB   -     -    -    R/W
	5       ITCM            01000000h 32KB   -     -    R/W  R/W
	6       BIOS            FFFF0000h 32KB   On    -    R    R
	7       Shared Work     027FF000h 4KB    -     -    -    R/W
*/

void gbamode(){
	pu_SetCodePermissions(0x06333333);	//instruction tcm rights: bit 0-3 reg0 - bit 4-7 reg1 - etc...
	pu_SetDataPermissions(0x06333333);	//data tcm rights: 

}

void ndsmode(){
//ori: pu_SetCodePermissions(0x33330333); //instruction region access permissions to others - C5,C0,3 - access rights: 3 rw , 6 ro
//ori: pu_SetDataPermissions(0x33330333); //data region access permission to others - c5, c0, 2 <-- could cause freezes if not properly set
pu_SetCodePermissions(0x33333333); //instruction region access permissions to others - C5,C0,3 - access rights: 3 rw , 6 ro
pu_SetDataPermissions(0x33333333); //data region access permission to others - c5, c0, 2 <-- could cause freezes if not properly set

}

//ori C install handlers

//branching method
/*
unsigned Install_Handler (unsigned routine, unsigned *vector){

 Updates contents of 'vector' to contain branch instruction 
 to reach 'routine' from 'vector'. Function return value is 
 original contents of 'vector'.
 NB: 'Routine' must be within range of 32Mbytes from 'vector'.

unsigned vec, oldvec;
vec = ((routine - (unsigned)vector - 0x8)>>2);
if (vec & 0xff000000){
	printf ("Installation of Handler failed");
	exit (1);
}
vec = 0xea000000 | vec; //branching method
oldvec = *vector;
*vector = vec;
return (oldvec);
}
*/
//Install_Handler(handler,vectors);

//LDR,PC+adress method
/*
unsigned Install_Handler (unsigned *location, unsigned *vector)
 Updates contents of 'vector' to contain LDR pc, [pc, #offset] 
 instruction to cause long branch to address in ‘location’. 
 Function return value is original contents of 'vector'.
{ unsigned vec, oldvec;
vec = ((unsigned)location - (unsigned)vector-0x8) | 0xe59ff000 //ldr pc,arm register method
oldvec = *vector;
*vector = vec;
return (oldvec);
}
*/

//BIOS debug vector: 0x027FFD9C //ARM9 exception vector: 0xFFFF0000 / <-- only if vectors & 0xffff0000
//exception handler address, exception vectors
unsigned Install_Handler (u32 * location, u32 *vector){
// Updates contents of 'vector' to contain LDR pc, [pc, #offset] 
// instruction to cause long branch to address in ‘location’. 
// Function return value is original contents of 'vector'.

u32 vec, oldvec;
vec = ((u32*)location - (u32*)vector- 0x8) | 0xe59ff000; //ldr pc,arm register method
oldvec = *vector;
*vector = vec;
return (oldvec); //return (vec);
}


u32 exception_dump(){

//todo: find a way to detect exception source

unsigned int reg0=0,reg1=0,reg2=0,reg3=0,reg13=0,reg14=0,reg15=0;

__asm__ volatile (
"mov %[reg0],r0" "\n\t"
"mov %[reg1],r1" "\n\t"
"mov %[reg2],r2" "\n\t"
"mov %[reg3],r3" "\n\t"

"mov %[reg13],#0;" "\n\t"
"add %[reg13],sp,#0;" "\n\t"

"mov %[reg14],#0;" "\n\t"
"add %[reg14],%[reg14], lr" "\n\t"//"sub lr, lr, #8;" "\n\t"
"sub %[reg14],%[reg14],	#0x8" "\n\t"

"mov %[reg15],#0;" "\n\t"
"add %[reg15],pc,#0;" "\n\t"

:[reg0] "=r" (reg0),[reg1] "=r" (reg1),[reg2] "=r" (reg2),[reg3] "=r" (reg3),[reg13] "=r" (reg13),[reg14] "=r" (reg14),[reg15] "=r" (reg15)
);	//r = hardware registers / g = frame pointer indirect stacking of registers

iprintf("\n EXCEPTION HAS OCCURED: CPSR:(%x) \n SPSR:(%x) ",(unsigned int)cpuGetCPSR(),(unsigned int)cpuGetSPSR());
iprintf("\n r0: %x |r1: %x |r2: %x |r3: %x \n",(unsigned int)reg0,(unsigned int)reg1,(unsigned int)reg2,(unsigned int)reg3);
iprintf("r13: %x |r14: %x |r15: %x \n",(unsigned int)reg13,(unsigned int)reg14,(unsigned int)reg15);

return 0;
}

/* Exception Vectors */
u32 exceptreset(){
	exception_dump();
	iprintf("\n PU exception type: RESET \n " );
	while(1);
return 0;
}

u32 exceptundef(u32 undefopcode){
	/*
	cpu_SetCP15Cnt(cpu_GetCP15Cnt() & ~0x1);
		
		unsigned int *lnk_ptr;
		__asm__ volatile (
		"mov %[lnk_ptr],#0;" "\n\t"
		"add %[lnk_ptr],%[lnk_ptr], lr" "\n\t"//"sub lr, lr, #8;" "\n\t"
		"sub %[lnk_ptr],%[lnk_ptr],#0x8" 
		:[lnk_ptr] "=r" (lnk_ptr)
		);
	
		iprintf("\n before exception: ");
		if (cpuGetSPSR() & 0x5) iprintf("thumb mode ");
		else iprintf("ARM mode ");
	
		iprintf("\n IN exception: ");
		if (cpuGetCPSR() & 0x5) iprintf("thumb mode ");
		else iprintf("ARM mode ");
		iprintf("\n OPCODE: %x",undefopcode);
		iprintf("\n PU exception type: UNDEFINED \n at %p (0x%08X) ", lnk_ptr, *(lnk_ptr));
	
	pu_Enable();
	*/
	
	//cpu_SetCP15Cnt(cpu_GetCP15Cnt() & ~0x1);	//MPU turning /off/on causes to jump to 0x00000000
		/*
		iprintf("\n before exception: ");
		if (cpuGetSPSR() & 0x5) iprintf("thumb mode ");
		else iprintf("ARM mode ");
	
		iprintf("\n IN exception: ");
		if (cpuGetCPSR() & 0x5) iprintf("thumb mode ");
		else iprintf("ARM mode ");
		*/
		
		exception_dump();
		iprintf("\n PU exception type: UNDEFINED \n");
		while(1);

	//pu_Enable();
	
return 0;
}

//swi abort part
u32 exceptswi (u32 swiaddress){
//iprintf("[swi: %x] \n",(unsigned int)swiaddress); //sorry, swi code can¡t handle printf overflows?!!
//while(1);
//iprintf("swi ctm \n");
	if (swiaddress == 0x0){
	//swi 0
	//iprintf("swi 0! \n");
	return 0;
	}
	else if(swiaddress == 0x1){
	//swi 1
	//iprintf("swi 1! \n");
	return 0;
	}
	else if(swiaddress == 0x2){
	//swi 2
	//iprintf("swi 2! \n");
	return 0;
	}
	
	else if(swiaddress == 0x4){
	//swi 2
	//iprintf("swi 4! \n");
	return 0;
	}
	
	/*
	//detect game filesize otherwise cause freezes
	else if((swiaddress >= 0x08000000) && (swiaddress < (romsize+0x08000000))  ){
	//iprintf("swigb\n");
	//return (ichfly_readu32(swiaddress ^ (u32)0x08000000));
	}
	*/
	
	else{
	//iprintf("swi unknown! \n");
	/*
		//cpu_SetCP15Cnt(cpu_GetCP15Cnt() & ~0x1);
		unsigned int *lnk_ptr;
		__asm__ volatile (
		"mov %[lnk_ptr],#0;" "\n\t"
		"add %[lnk_ptr],%[lnk_ptr], lr" "\n\t"//"sub lr, lr, #8;" "\n\t"
		"sub %[lnk_ptr],%[lnk_ptr],#0x8" 
		:[lnk_ptr] "=r" (lnk_ptr)
		);
	
		iprintf("\n before exception: ");
		if (cpuGetSPSR() & 0x5) iprintf("thumb mode ");
		else iprintf("ARM mode ");
	
		iprintf("\n IN exception: ");
		if (cpuGetCPSR() & 0x5) iprintf("thumb mode ");
		else iprintf("ARM mode ");
		iprintf("\n SWI #num: %x",swiaddress);
		iprintf("\n PU exception type: swi \n at %p (0x%08X) ", lnk_ptr, *(lnk_ptr));
		//iprintf("\n btw romsize: %x",romsize);
	
		//pu_Enable();
	*/
	}

exception_dump();
iprintf("swi exception within range 0..1Fh.");
iprintf("\n PU exception type: SWI \n");
while(1);
	
return 0;
}

u32 exceptprefabt(){

exception_dump();
iprintf("\n PU exception type: PREFETCH ABORT \n");
while(1);

return 0;
}

//data_abort handler
u32 exceptdataabt(u32 dabtaddress){
	/*
	//16 bit reads
	if( ((cpsrvirt>>5)&1) == 0x1 ){
	
		if((dabtaddress >= 0x08000000) && (dabtaddress < 0x08000200)  ){
			//iprintf("\n => data abt! (%x):%x ",dabtaddress,gbaheaderbuf[(dabtaddress ^ 0x08000000)/4]);
			return ((gbaheaderbuf[(dabtaddress ^ 0x08000000)/4])&0xffff);
		}
	
		else if((dabtaddress >= 0x08000200) && (dabtaddress < (romsize+0x08000000))  ){
			//iprintf("\n => data abt! (%x):%x ",dabtaddress,ichfly_readu32(dabtaddress ^ 0x08000000));
			return stream_readu16(dabtaddress ^ 0x08000000);
		}
	}
	//32Bit reads
	else{
	
		if((dabtaddress >= 0x08000000) && (dabtaddress < 0x08000200)  ){
			//iprintf("\n => data abt! (%x):%x ",dabtaddress,gbaheaderbuf[(dabtaddress ^ 0x08000000)/4]);
			return gbaheaderbuf[(dabtaddress ^ 0x08000000)/4];
		}
	
		else if((dabtaddress >= 0x08000200) && (dabtaddress < (romsize+0x08000000))  ){
			//iprintf("\n => data abt! (%x):%x ",dabtaddress,ichfly_readu32(dabtaddress ^ 0x08000000));
			return stream_readu32(dabtaddress ^ 0x08000000);
		}
	}
	*/

iprintf("data abort exception!");
exception_dump();
while(1);	
return 0;
}

u32 exceptreserv(){	
return 0;
}

void emulateedbiosstart(){
	cpu_SetCP15Cnt(cpu_GetCP15Cnt() &~BIT(13));
}

//Setup of MPU region & cache settings
void setgbamap(){

	//update DTCM user reserved address
	//dtcm_end_alloced=(unsigned int)(&dtcm_end_alloced+0x2);

nopinlasm();

	//correct PU setting way:
	//u32 reserved_dtcm = dtcm_end_alloced-getdtcmbase();
	
	//DC_FlushAll(); //coherent cache clean /(incoherent ...... hahaha) see below
	
	//IC_InvalidateAll(); //Instruction Cache Lockdown
	
	//let's flush only DTCM that is not used..
	//DC_FlushRange((const void*)dtcm_end_alloced+0x1, ((dtcm_top_ld-getdtcmbase()) - (0x400*4)) - reserved_dtcm ); //(const void* , u32 v)
	
	//iprintf("free DTCM cache: (%d)bytes",(int)( ((dtcm_top_ld-getdtcmbase()) - (0x400*4)) - reserved_dtcm) );
	
	
	//lockdown DCACHE
	//Format A:
	// 0..(31-W)  Reserved/zero
	//(32-W)..31 Lockdown Block Index
	
	/*
	int j=0;
	
	for(j=0;j<( (reserved_dtcm / 32 ) );j++){ //32 byte blocks per MAP INDEX 
		DC_lockdownVA(j);
	}
	int k=0;
	for(k=0;k<( (reserved_dtcm % 32 ) );k++){ //32 byte blocks per OFFSET INDEX 
		DC_lockdownVA(j+k);
	}
	*/
	
nopinlasm();

	IC_InvalidateAll(); //Instruction Cache Lockdown
	DC_InvalidateAll(); //Data Cache lockdown
	
nopinlasm();

	setdtcmbase(); //set dtcm base (see linker file)
	setitcmbase(); //set itcm base (see linker file)

nopinlasm();

	cpu_SetCP15Cnt(cpu_GetCP15Cnt() & ~0x1); //Disable pu to configure it
	//DC_InvalidateAll(); //Data Cache free access

nopinlasm();

	prepcache(); //enable (ON) cache DTCM / ITCM hardware mechanism

nopinlasm();

	pu_SetRegion(0, (u32)0x04000000 	| 	(u32)0x1fffff		|1					); 	//protect IO 0x41fffff OK
	pu_SetRegion(1, (u32)getdtcmbase() |	(u32)0x00004000 	|1					); 	//dtcm OK
	pu_SetRegion(2, (u32)0x01001000 	|	(u32)0x00007000 	|1					);	//itcm OK
	//pu_SetRegion(3, (u32)gbawram_start	| 	(u32)gbawram_size	|1					);	//GBAWRAM OK (temp removed)
	//pu_SetRegion(4, (u32)vector_addr 	|	(u32)(vector_end_addr-vector_addr) 	|1	);	//vectors
	pu_SetRegion(5, (u32)__ewram_start 	|	(u32)(__ewram_end-__ewram_start) 	|1					);	
	
	//iprintf("GBAWRAM size is: %x",(unsigned int)sizeof(gbawram));
	nopinlasm();

	//dtcm & itcm do not use write/cache buffers | IO ports included
	writebufenable	(0b00101000); ////C3,C0,0. for dtcm, itcm is ro enable buffer-write on what regions (pu_GetWriteBufferability())
	dcacheenable	(0b00101000);	//c2,c0,0 data tcm can access to what (pu_SetDataCachability())
	icacheenable	(0b00101000); //c2,c0,1	instruction tcm can access to what (pu_SetCodeCachability())

	pu_SetCodePermissions(0x33363363); //instruction region access permissions to others - C5,C0,3 - access rights: 3 rw , 6 ro
	pu_SetDataPermissions(0x33363633); //data region access permission to others - c5, c0, 2 <-- could cause freezes if not properly set


	//drainwrite(); //update memory banks at cachable + noncachable memory
	nopinlasm();
	nopinlasm();

//requires: 1) asm handlers 2) they are on itcm 3) call to C handlers for each asm handler
#ifdef NONDS9HANDLERS
	iprintf("PU: set exception vec @0x00000000\n ");
	emulateedbiosstart();
#else
	iprintf("PU: set exception vec @0xffff0000\n ");
	*(u32*)(0x027FFD9C)=(u32)&exception_dump;
#endif

	pu_Enable(); //activate region settings	

}
