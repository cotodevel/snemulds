.align 4
.code 32
.ARM

.global ldsbasm_arm	
.type ldsbasm_arm STT_FUNC
ldsbasm_arm:
	@opcode
	ldrsb r0,[r0,r1]						
	bx lr

.global ldshasm_arm	
.type ldshasm_arm STT_FUNC
ldshasm_arm:
	@opcode
	ldrsh r0,[r0,r1]						
	bx lr

@load/store
.global ldru32extasm_arm	
.type ldru32extasm_arm STT_FUNC
ldru32extasm_arm:
	@opcode
	ldr r0,[r0,r1]					
	bx lr

.global ldru16extasm_arm	
.type ldru16extasm_arm STT_FUNC
ldru16extasm_arm:
	@opcode
	ldrh r0,[r0,r1]					
	bx lr

.global ldru8extasm_arm	
.type ldru8extasm_arm STT_FUNC
ldru8extasm_arm:
	@opcode
	ldrb r0,[r0,r1]					
	bx lr

.global stru32extasm_arm
.type stru32extasm_arm STT_FUNC 
stru32extasm_arm:
	@opcode
	str r1,[r0]
	bx lr

.global stru16extasm_arm
.type stru16extasm_arm STT_FUNC 
stru16extasm_arm:
	@opcode
	strh r1,[r0]
	bx lr

.global stru8extasm_arm
.type stru8extasm_arm STT_FUNC 
stru8extasm_arm:
	@opcode
	strb r1,[r0]
	bx lr

.global rorasm	
.type rorasm STT_FUNC
rorasm:
	STMFD sp!, {r1-r12,lr}		
	movs r0,r0, ror r1 		
	LDMFD sp!, {r1-r12,lr}
	bx lr

.global copy8arm
.type copy8arm STT_FUNC
copy8arm:
	@r0 : source | r1: dest | r2: size
	
	copy:
	cmp r2,#0
	blt done
	bgt do_cpy
	
	do_cpy:
	ldrh r3,[r0]!
	ror r3,#8
	strh r3,[r1]!
	
	@ldmia r0!,{r3}
	@stmia r1!,{r3}
	
	sub r2, r2, #(2) 	@	2 *	1byte (halfword)
	b copy
	
	done:
	bx lr
	


.align
.pool
.end