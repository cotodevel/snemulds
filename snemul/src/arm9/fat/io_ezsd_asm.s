	.align	4
	.code 32	
	.arm
.global  SD_WriteBufferToLine ,  SD_WriteBufferToLine , SD_ReadResponse 
.global  SD_ReadData , SD_WriteData

SD_WriteBufferToLine:
@;  r0 - char *pBuf  , r1 - int lenght
	stmfd	sp!,{r2-r7}
	ldr		r3,=0x9FFFF40
sdw_l1: 						@;此段为命令字节个数的循环
	ldrb		r2, [r0],#+1
	mov		r6,#0
sdw_l2:						@;此段为8字节的位发送循环
	mov		r4,r2,lsl r6
	strh		r4,[r3]
	add		r6,r6,#1
	cmp		r6,#8
	bne		sdw_l2
	subs		r1,r1,#1
	bne		sdw_l1			
	ldmfd	sp!,{r2-r7}
	bx		lr 

SD_ReadResponse:
@; r0 - char *pBuf ; r1 - int lenght , r2 - waitlen[timeout]
@; 返回值
	stmfd	sp!,{r3-r7}
@;	cmp		r1,#0			@;长度为0退出
@;	beq		sdexit1
@;	cmp		r2,#0
@;	beq		sdexit
	ldr		r3,=0x9FFFF40	
sdrr1:
	subs		r2,r2,#1
	beq		sdexit2
	ldrh		r4,[r3]
	tst		r4,#0x1
	bne		sdrr1
	mov		r4,#0
readnext:
	mov		r7,#8
	mov		r6,#0
sdr_l2:
	ldrneh	r4,[r3]
	and		r5,r4,#1
	subs		r7,r7,#1
	add		r6,r6,r5,lsl r7
	bne		sdr_l2
	strb		r6,[r0],#+1
	subs		r1,r1,#1
	bne		readnext
sdexit1	:
@;	response need loop for next command
	ldrh		r4,[r3]
	ldrh		r5,[r3]
	ldrh		r6,[r3]
	ldrh		r7,[r3]
	ldrh		r4,[r3]
	ldrh		r5,[r3]
	ldrh		r6,[r3]
	ldrh		r7,[r3]
	mov		r0,#1
	ldmfd	sp!,{r3-r7}
	bx		lr 
sdexit2:
	mov		r0,#0
	ldmfd	sp!,{r3-r7}
	bx		lr
	
SD_ReadData:
@;r0- buf , r1 - size ,  r2 - waitlen[timeout]
	stmfd	sp!,{r3-r7}
	ldr		r3,=0x9FEA000
	mov		r4,#0xF00
sdrd_l1:
	subs		r2,r2,#1
	beq		sdr_exit2
	ldrh		r5,[r3]
	ands	r6,r5,r4	
	bne		sdrd_l1
sdrd_l2:
	ldmia	r3,{r4-r5}
	mov		r6,r5,lsr #16
	strh		r6,[r0],#+2
	subs		r1,r1,#2
	bne		sdrd_l2
sdr_exit:
	mov		r0,#1
	ldmfd	sp!,{r3-r7}
	bx		lr
sdr_exit2:
	mov		r0,#0 
	ldmfd	sp!,{r3-r7}
	bx		lr

SD_WriteData:
@;r0- buf , r1 - size ,  r2 - waitlen[timeout]
	stmfd	sp!,{r3-r7}
	ldr		r3,=0x9FEA000
@;start bit 
	mov		r4,#0
	strh		r4,[r3]
sdwd_l1:
	ldrb		r4,[r0], #+1
	ldrb		r5,[r0], #+1
	mov		r4,r4,lsl #8
	strh		r4,[r3]
	mov		r4,r4,lsl #4
	strh		r4,[r3,#2]
	mov		r5,r5,lsl #8
	strh		r5,[r3,#4]
	mov		r5,r5, lsl #4
	strh		r5,[r3,#6]
	subs		r1,r1,#2
	bne		sdwd_l1
@;end bit 
	mov		r4,#0xFFFFFFFF	
	strh		r4,[r3]
sdw_exit:

	ldmfd	sp!,{r3-r7}
	bx		lr
	.ALIGN
@;sample  DCD     0x100
	.END
