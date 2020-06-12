/****************************************************************************/

/*
 *	linux/arch/arm/mach-nds/head.S
 *
 *	Copyright (C) 2005, Malcolm Parsons <pepsiman@blueyonder.co.uk>
 *
 *  uClinux kernel startup code for NDS
 *  which has no proper bootloader for linux startup
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

.arch	armv5te
.cpu	arm946e-s
.section    .text
.arm
 
/****************************************************************************/

#include "dsregs_asm.h"


@ Protection Region Settings: http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.ddi0201d/I1039570.html

@coto: enable this flag to allow to set proper physical -> linear sections + setting MPU
#define CRT0SREPLACEMENT

@region settings

@format:
@addr = IO address
@PAGE_X = Region size in Page Size units to cover, starting addr
@(1<<0) & (region_enable_bit<<0) = bit setting for enable(1) or disable(0) such Region.
@example: ( PAGE_64M | addr | 1)

#define region_enable_bit 1

@for adding custom ControlRegister bits while keeping context
.global CP15ControlRegisterEnable
.type   CP15ControlRegisterEnable STT_FUNC
CP15ControlRegisterEnable:
mrc	p15, 0, r1, c1, c0, 0
orr	r1,r1,r0
mcr	p15, 0, r1, c1, c0, 0
bx lr

.global CP15ControlRegisterDisable
.type   CP15ControlRegisterDisable STT_FUNC
CP15ControlRegisterDisable:
mrc	p15, 0, r1, c1, c0, 0
bic	r1,r1,r0
mcr	p15, 0, r1, c1, c0, 0
bx lr

.global DrainWriteBuffer
.type   DrainWriteBuffer STT_FUNC
DrainWriteBuffer:
mov r0,#0
mcr	p15, 0, r0, c7, c10, 4
bx lr

.global DTCMDisable
.type   DTCMDisable STT_FUNC
DTCMDisable:
mrc	p15, 0, r0, c1, c0, 0
ldr	r1,= (CR_DT)
bic	r0,r0,r1
mcr	p15, 0, r0, c1, c0, 0
bx lr

.global ITCMDisable
.type   ITCMDisable STT_FUNC
ITCMDisable:
mrc	p15, 0, r0, c1, c0, 0
ldr	r1,= (CR_IT)
bic	r0,r0,r1
mcr	p15, 0, r0, c1, c0, 0
bx lr

.global DTCMEnable
.type   DTCMEnable STT_FUNC
DTCMEnable:
mrc	p15, 0, r0, c1, c0, 0
ldr	r1,= (CR_DT)
orr	r0,r0,r1
mcr	p15, 0, r0, c1, c0, 0
bx lr

.global ITCMEnable
.type   ITCMEnable STT_FUNC
ITCMEnable:
mrc	p15, 0, r0, c1, c0, 0
ldr	r1,= (CR_IT)
orr	r0,r0,r1
mcr	p15, 0, r0, c1, c0, 0
bx lr

.global DCacheDisable
.type   DCacheDisable STT_FUNC
DCacheDisable:
mrc	p15, 0, r2, c1, c0, 0
bic r2,r2,#(CR_C)
mcr	p15, 0, r2, c1, c0, 0
bx lr

.global ICacheDisable
.type   ICacheDisable STT_FUNC
ICacheDisable:
mrc	p15, 0, r2, c1, c0, 0
bic r2,r2,#(CR_I)
mcr	p15, 0, r2, c1, c0, 0
bx lr

.global DCacheEnable
.type   DCacheEnable STT_FUNC
DCacheEnable:
mrc	p15, 0, r2, c1, c0, 0
orr r2,r2,#(CR_C)
mcr	p15, 0, r2, c1, c0, 0
bx lr

.global ICacheEnable
.type   ICacheEnable STT_FUNC
ICacheEnable:
mrc	p15, 0, r2, c1, c0, 0
orr r2,r2,#(CR_I)
mcr	p15, 0, r2, c1, c0, 0
bx lr



.global MProtectionDisable
.type   MProtectionDisable STT_FUNC
MProtectionDisable:
mrc	p15, 0, r2, c1, c0, 0
bic r2,r2,#(CR_M)
mcr	p15, 0, r2, c1, c0, 0
bx lr

.global MProtectionEnable
.type   MProtectionEnable STT_FUNC
MProtectionEnable:
mrc	p15, 0, r2, c1, c0, 0
orr r2,r2,#(CR_M)
mcr	p15, 0, r2, c1, c0, 0
bx lr

.global MProtectionRegionBase0
.type   MProtectionRegionBase0 STT_FUNC
MProtectionRegionBase0:
mcr	p15, 0, r0, c6, c0, 0
bx lr

.global MProtectionRegionBase1
.type   MProtectionRegionBase1 STT_FUNC
MProtectionRegionBase1:
mcr	p15, 0, r0, c6, c1, 0
bx lr

.global MProtectionRegionBase2
.type   MProtectionRegionBase2 STT_FUNC
MProtectionRegionBase2:
mcr	p15, 0, r0, c6, c2, 0
bx lr

.global MProtectionRegionBase3
.type   MProtectionRegionBase3 STT_FUNC
MProtectionRegionBase3:
mcr	p15, 0, r0, c6, c3, 0
bx lr

.global MProtectionRegionBase4
.type   MProtectionRegionBase4 STT_FUNC
MProtectionRegionBase4:
mcr	p15, 0, r0, c6, c4, 0
bx lr

.global MProtectionRegionBase5
.type   MProtectionRegionBase5 STT_FUNC
MProtectionRegionBase5:
mcr	p15, 0, r0, c6, c5, 0
bx lr

.global MProtectionRegionBase6
.type   MProtectionRegionBase6 STT_FUNC
MProtectionRegionBase6:
mcr	p15, 0, r0, c6, c6, 0
bx lr

.global MProtectionRegionBase7
.type   MProtectionRegionBase7 STT_FUNC
MProtectionRegionBase7:
mcr	p15, 0, r0, c6, c7, 0
bx lr


@WritebufferControl
@format: r0 = Regions to enable/disable WriteBuffer
.global WritebufferControl
.type   WritebufferControl STT_FUNC
WritebufferControl:
mcr	p15, 0, r0, c3, c0, 0
bx lr


.global RegionDCacheEnable
.type   RegionDCacheEnable STT_FUNC
RegionDCacheEnable:
mcr	p15, 0, r0, c2, c0, 0		@ region d-cacheable
bx lr

.global RegionICacheEnable
.type   RegionICacheEnable STT_FUNC
RegionICacheEnable:
mcr	p15, 0, r0, c2, c0, 1		@ region i-cacheable
bx lr	

.global RegionIAccessPermission
.type   RegionIAccessPermission STT_FUNC
RegionIAccessPermission:
mcr	p15, 0, r0, c5, c0, 3		@ set inst. access permission
bx lr


.global RegionDAccessPermission
.type   RegionDAccessPermission STT_FUNC
RegionDAccessPermission:
mcr	p15, 0, r0, c5, c0, 2		@ set data access permission
bx lr


.global setitcm
.type   setitcm STT_FUNC
setitcm:
mov	r0,#0x20
mcr	p15, 0, r0, c9, c1,1		@ ITCM base = 0 , size = 32 MB
bx lr

.global setdtcm
.type   setdtcm STT_FUNC
setdtcm:
ldr	r0, =_dtcm_start
orr	r0,r0,#0x0a
mcr	p15, 0, r0, c9, c1,0		@ DTCM base = _dtcm_start, size = 16 KB
bx lr



.global MPUSet
.type   MPUSet STT_FUNC
MPUSet:
	push {r0-r12,lr}
	
	#ifdef EXCEPTION_VECTORS_0xffff0000
	bl set0xFFFF0000FastMPUSettings
	#endif
	
	#ifdef EXCEPTION_VECTORS_0x00000000
	bl set0x00000000FastMPUSettings
	#endif
	
pop {r0-r12,lr}
bx	lr
	
.pool
.end
