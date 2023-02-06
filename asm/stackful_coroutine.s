	.syntax unified

	@ The state required to resume a coroutine or caller.
	@
	@ r4-r11, sp are restored.
	@ stack is copied to [sp..sp+stackSize].
	@ bx lr.
	@
	@ NOTE: stackSize may be negative for a caller's context, in
	@ which case the stack is not copied. This occurs when the
	@ coroutine's stack is at a lower address than the caller's
	@ stack.
	@
	@ struct Context
	@ {
	@     u32 r4, r5, r6, r7, r8, r9, r10, r11;
	@     void *sp, *lr;
	@     ssize_t stackSize;
	@     u32 stack[];
	@ };

	.set ctx_r4, 0
	.set ctx_r5, 4
	.set ctx_r6, 8
	.set ctx_r7, 12
	.set ctx_r8, 16
	.set ctx_r9, 20
	.set ctx_r10, 24
	.set ctx_r11, 28
	.set ctx_sp, 32
	.set ctx_lr, 36
	.set ctx_stackSize, 40
	.set ctx_stack, 44

	.global CreateCoroutine0
	.thumb_func
@ Coroutine *CreateCoroutine0(void (*)(void))
CreateCoroutine0:
	.global CreateCoroutine1
	.thumb_func
@ struct Coroutine *CreateCoroutine1(void (*)(), union CoroutineArgument)
CreateCoroutine1:
	push {r4, r5, lr}
@ r4 = f
	movs r4, r0
@ r5 = arg0
	movs r5, r1
@ co = Alloc(sizeof(struct Context))
	movs r0, #ctx_stack
	bl Alloc
@ if (!co) goto .LCreateCoroutine_OOM
	cmp r0, #0
	beq .LCreateCoroutine_OOM
@ co->r4 = r5
	str r5, [r0, #ctx_r4]
@ co->sp = co->stackSize = 0
	movs r1, #0
	str r1, [r0, #ctx_sp]
	str r1, [r0, #ctx_stackSize]
@ co->lr = r4
	str r4, [r0, #ctx_lr]
	pop {r4, r5}
	pop {r1}
	bx r1
	.pool
.LCreateCoroutine_OOM:
	bl CoroutineOOM
	b .

	.global ResumeCoroutine
	.thumb_func
@ struct Coroutine *ResumeCoroutine(struct Coroutine *)
ResumeCoroutine:
	r_co .req r4
	r_ca .req r5
	r_ca_sp .req r6
	r_ca_stackSize .req r7
@ TODO: Abort if sCurrentCaller != NULL.
	push {r4, r5, r6, r7, lr}
	add r_ca_sp, sp, #20
	movs r_co, r0
@ co_sp0 = co->sp + co->stackSize
	ldr r0, [r_co, #ctx_sp]
	ldr r1, [r_co, #ctx_stackSize]
	adds r0, r1
@ if (co_sp0 == 0) co_sp = sp
	bne .LResumeCoroutine_NonZeroSize
	movs r0, r_ca_sp
.LResumeCoroutine_NonZeroSize:
@ ca_stackSize = co_sp0 - ca_sp
	subs r0, r_ca_sp
	movs r_ca_stackSize, r0
@ ca = Alloc(sizeof(struct Context) + max(ca_stackSize, 0))
	bge .LResumeCoroutine_NonNegativeSize
	movs r0, #0
.LResumeCoroutine_NonNegativeSize:
	adds r0, #ctx_stack
	bl Alloc
@ if (!ca) goto .LResumeCoroutine_OOM
	movs r_ca, r0
	beq .LResumeCoroutine_OOM
@ if (ca_stackSize > 0) CpuSet(ca_sp, ca->stack, CPU_SET_32BIT | (ca_stackSize / 4))
	asrs r2, r_ca_stackSize, #2
	ble .LResumeCoroutine_StackCopied
	movs r0, r_ca_sp
	movs r1, r_ca
	adds r1, #ctx_stack
	ldr r3, =1 << 26
	orrs r2, r3
	swi 0x0b
.LResumeCoroutine_StackCopied:
@ ca->sp = ca_sp
	str r_ca_sp, [r_ca, #ctx_sp]
@ ca->stackSize = ca_stackSize
	str r_ca_stackSize, [r_ca, #ctx_stackSize]
	.unreq r_ca_sp
	.unreq r_ca_stackSize
@ ca->{r4-r7, lr} = ...
	pop {r0, r1, r2, r3, r7}
	str r7, [r_ca, #ctx_lr]
	adds r6, r_ca, #ctx_r4
	stmia r6!, {r0, r1, r2, r3}
@ ca->{r8-r11} = ...
	mov r0, r8
	mov r1, r9
	mov r2, r10
	mov r3, r11
	stmia r6!, {r0, r1, r2, r3}
@ sCurrentCaller = ca
	ldr r0, =sCurrentCaller
	str r_ca, [r0]
	.unreq r_ca
@ Free(co)
@ NOTE: Assumes that Free does not write to the freed memory.
	movs r0, r_co
	bl Free
@ if (co->sp != NULL) { sp = co->sp; CpuSet(co->stack, sp, CPU_SET_32BIT | (co->stackSize / 4)) }
	ldr r1, [r_co, #ctx_sp]
	cmp r1, #0
	beq .LResumeCoroutine_Initial
	mov sp, r1
	movs r0, r_co
	adds r0, #ctx_stack
	ldr r2, [r_co, #ctx_stackSize]
	asrs r2, #2
	ldr r3, =1 << 26
	orrs r2, r3
	swi 0x0b
@ ... = co->{r8-r11}
	movs r6, r_co
	adds r6, #ctx_r8
	ldmia r6!, {r0, r1, r2, r3}
	mov r8, r0
	mov r9, r1
	mov r10, r2
	mov r11, r3
@ ... = co->{r4-r7}; co->lr()
	adds r0, r_co, #ctx_r4
	ldr r1, [r0, #ctx_lr - ctx_r4]
	ldmia r0!, {r4, r5, r6, r7}
	bx r1
.LResumeCoroutine_Initial:
@ co->lr(co->r4)
	ldr r2, =ExitCoroutine
	mov lr, r2
	ldr r0, [r_co, #ctx_r4]
	ldr r1, [r_co, #ctx_lr]
	bx r1
	.pool
	.unreq r_co
.LResumeCoroutine_OOM:
	bl CoroutineOOM
	b .

	.global SuspendCoroutine
	.thumb_func
@ void SuspendCoroutine(void)
SuspendCoroutine:
	r_ca .req r4
	r_co .req r5
	r_co_sp .req r6
	r_co_stackSize .req r7
	push {r4, r5, r6, r7, lr}
	add r_co_sp, sp, #20
@ ca = sCurrentCaller
	ldr r0, =sCurrentCaller
	ldr r_ca, [r0]
@ sCurrentCaller = NULL
	movs r1, #0
	str r1, [r0]
@ co_sp0 = ca->sp + ca->stackSize
@ co_stackSize = co_sp0 - sp
	ldr r_co_stackSize, [r_ca, #ctx_sp]
	ldr r1, [r_ca, #ctx_stackSize]
	adds r_co_stackSize, r1
	subs r_co_stackSize, r_co_sp
@ co = Alloc(sizeof(struct Context) + co_stackSize)
	movs r0, r_co_stackSize
	adds r0, #ctx_stack
	bl Alloc
	movs r_co, r0
@ if (!co) goto .LSuspendCoroutine_OOM
	beq .LSuspendCoroutine_OOM
@ CpuSet(co_sp, co->stack, CPU_SET_32BIT | (co_stackSize / 4))
	movs r0, r_co_sp
	movs r1, r_co
	adds r1, #ctx_stack
	asrs r2, r_co_stackSize, #2
	ldr r3, =1 << 26
	orrs r2, r3
	swi 0x0b
@ co->sp = co_sp
	str r_co_sp, [r_co, #ctx_sp]
@ co->stackSize = co_stackSize
	str r_co_stackSize, [r_co, #ctx_stackSize]
	.unreq r_co_sp
	.unreq r_co_stackSize
@ co->{r4-r7, lr} = ...
	pop {r0, r1, r2, r3, r7}
	str r7, [r_co, #ctx_lr]
	adds r6, r_co, #ctx_r4
	stmia r6!, {r0, r1, r2, r3}
@ co->{r8-r11} = ...
	mov r0, r8
	mov r1, r9
	mov r2, r10
	mov r3, r11
	stmia r6!, {r0, r1, r2, r3}
@ Free(ca)
@ NOTE: Assumes that Free does not write to the freed memory.
	movs r0, r_ca
	bl Free
@ sp = ca->sp
	ldr r1, [r_ca, #ctx_sp]
	mov sp, r1
@ if (ca->stackSize > 0) CpuSet(ca->stack, ca->sp, CPU_SET_32BIT | (ca->stackSize / 4))
	ldr r2, [r_ca, #ctx_stackSize]
	asrs r2, #2
	ble .LSuspendCoroutine_StackCopied
	ldr r3, =1 << 26
	orrs r2, r3
	movs r0, r_ca
	adds r0, #ctx_stack
	swi 0x0b
.LSuspendCoroutine_StackCopied:
@ ... = ca->{r8-r11}
	movs r6, r_ca
	adds r6, #ctx_r8
	ldmia r6!, {r0, r1, r2, r3}
	mov r8, r0
	mov r9, r1
	mov r10, r2
	mov r11, r3
@ r0 = co
	movs r0, r_co
	.unreq r_co
@ r1 = ca
	movs r1, r_ca
	.unreq r_ca
@ ... = ca->{r4-r7}
	adds r2, r1, #ctx_r4
	ldmia r2!, {r4, r5, r6, r7}
@ ca->lr(r0)
	ldr r1, [r1, #ctx_lr]
	bx r1
	.pool
.LSuspendCoroutine_OOM:
	bl CoroutineOOM
	b .

	.thumb_func
ExitCoroutine:
@ ca = sCurrentCaller
	ldr r0, =sCurrentCaller
	ldr r4, [r0]
@ sCurrentCaller = NULL
	movs r1, #0
	str r1, [r0]
@ Free(ca)
@ NOTE: Assumes that Free does not write to the freed memory.
	movs r0, r4
	bl Free
@ sp = ca->sp
IsFree:
	ldr r1, [r4, #ctx_sp]
	mov sp, r1
@ if (ca->stackSize > 0) CpuSet(ca->stack, sp, CPU_SET_32BIT | (ca->stackSize / 4))
	ldr r2, [r4, #ctx_stackSize]
	asrs r2, #2
	ble .LExitCoroutine_StackCopied
	movs r0, r4
	adds r0, #ctx_stack
	mov r1, sp
	ldr r3, =1 << 26
	orrs r2, r3
	swi 0x0b
.LExitCoroutine_StackCopied:
@ ... = ca->{r8-r11}
	movs r1, r4
	adds r4, #ctx_r8
	ldmia r4!, {r3, r5, r6, r7}
	mov r8, r3
	mov r9, r5
	mov r10, r6
	mov r11, r7
@ ... = ca->{r4-r7}
	adds r0, r1, #ctx_r4
	ldmia r0!, {r4, r5, r6, r7}
@ ca->lr(NULL)
	ldr r1, [r1, #ctx_lr]
	movs r0, #0
	bx r1
	.pool

	.global CoroutineOOM
	.weak CoroutineOOM
	.thumb_func
CoroutineOOM:
	swi #0x00

	.section .bss
	.balign 4
sCurrentCaller:
	.space 4
