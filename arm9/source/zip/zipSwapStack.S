.section	.text
.arm

.global snapshotRegs
snapshotRegs:
	.word 0x00000000	@r0
	.word 0x00000000	@r1
	.word 0x00000000	@r2
	.word 0x00000000	@r3
	.word 0x00000000	@r4
	.word 0x00000000	@r5
	.word 0x00000000	@r6
	.word 0x00000000	@r7
	.word 0x00000000	@r8
	.word 0x00000000	@r9
	.word 0x00000000	@r10
	.word 0x00000000	@r11
	.word 0x00000000	@r12
	.word 0x00000000	@r13 / sp
	.word 0x00000000	@r14 / lr
	.word 0x00000000	@r15 / pc


@int do_decompression_ewramstack(r0 = char *inname, r1 = char *outname);
	.global	do_decompression_ewramstack
	.type	do_decompression_ewramstack STT_FUNC
do_decompression_ewramstack:
	ldr r3,=snapshotRegs
	stmia	r3, {r1,r2,r4-r14}
	
	mov r7,r0
	mov r8,r1		@save: (r0 = char *inname, r1 = char *outname);
	
	ldr r0, =(128*1024)	@128K stack
	blx malloc 
	mov sp,r0	@use banked stack
	
	mov r0,r7
	mov r1,r8
	
	push {lr}
	blx do_decompression	@r0 ret
	pop {lr}
	
	mov r0,sp
	ldr r3,=snapshotRegs
	ldmia	r3, {r1,r2,r4-r14}
	
	blx free 
	
	bx	lr

.pool

.end
