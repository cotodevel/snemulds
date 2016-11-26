.align 2 @use always align 2 in thumb code
.code 16
.thumb

.global ldsbasm_tmb
.type ldsbasm_tmb STT_FUNC
ldsbasm_tmb:
	@opcode
	ldrsb r0,[r0,r1]						
	bx lr

.global ldshasm_tmb	
.type ldshasm_tmb STT_FUNC
ldshasm_tmb:
	@opcode
	ldrsh r0,[r0,r1]						
	bx lr

@load/store
.global ldru32extasm_tmb	
.type ldru32extasm_tmb STT_FUNC
ldru32extasm_tmb:
	@opcode
	ldr r0,[r0,r1]					
	bx lr

.global ldru16extasm_tmb	
.type ldru16extasm_tmb STT_FUNC
ldru16extasm_tmb:
	@opcode
	ldrh r0,[r0,r1]					
	bx lr

.global ldru8extasm_tmb	
.type ldru8extasm_tmb STT_FUNC
ldru8extasm_tmb:
	@opcode
	ldrb r0,[r0,r1]					
	bx lr

.global stru32extasm_tmb
.type stru32extasm_tmb STT_FUNC 
stru32extasm_tmb:
	@opcode
	str r1,[r0]
	bx lr

.global stru16extasm_tmb
.type stru16extasm_tmb STT_FUNC 
stru16extasm_tmb:
	@opcode
	strh r1,[r0]
	bx lr

.global stru8extasm_tmb
.type stru8extasm_tmb STT_FUNC 
stru8extasm_tmb:
	@opcode
	strb r1,[r0]
	bx lr

@thumb swi
.global swiWaitForVBlank_0
.thumb_func
swiWaitForVBlank_0:
	swi	0x05
	b	swiWaitForVBlank_0	

.align
.pool
.end