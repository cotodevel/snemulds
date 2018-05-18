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

	.TEXT
	.ARM
	.ALIGN

.GLOBAL IntrHandlerAsm
IntrHandlerAsm:
    mov r0, #0x4000000
    ldr r1, [r0,#0x214] @ IF

    tsts    r1, #0x02               @ h-blank interrupt
    bne     Hblank
    tsts    r1, #0x01               @ v-blank interrupt
    bne     Vblank
    bx      lr
    
.GLOBAL DesMuMeDebug
DesMuMeDebug:
	swi		#0x1e0000
