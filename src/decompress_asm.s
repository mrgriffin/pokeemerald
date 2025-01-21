    .syntax unified

    .arm
    .section .iwram.code
    .align 2

.global FastUnsafeCopy32
.type FastUnsafeCopy32, %function
    
    @ Word aligned, 32-byte copy
    @ This function WILL overwrite your buffer, so make sure it is at least 32 bytes larger than the desired size.
FastUnsafeCopy32:
    push    {r4-r10}
.Lloop_32:
    ldmia r1!, {r3-r10}
    stmia r0!, {r3-r10}
    subs    r2, r2, #32
    bgt     .Lloop_32
    pop     {r4-r10}
    bx    lr

    .section .rodata
.global DecodeInstructions
.type DecodeInstructions, %function
DecodeInstructions:
    @ r0: u32 headerLoSize
    @ r1: u8 *loVec
    @ r2: u16 *symVec
    @ r3: u16 *dest
    push {r4-r6}
    add r0, r1 @ r0: u8* loVecEnd
    b .LDecodeInstructions_Loop

.LDecodeInstructions_CopyDest:
    lsl r4, #1
.LDecodeInstructions_CopyDest_Loop:
    ldrh r6, [r3, -r4] @ r6: *(dest - currOffset)
    strh r6, [r3], #2 @ *dest++ = r6
    subs r5, #1 @ currLength--
    bne .LDecodeInstructions_CopyDest_Loop

.LDecodeInstructions_Loop:
    cmp r1, r0
    beq .LDecodeInstructions_Exit @ if (loVec == loVecEnd) goto Exit
    ldrb r4, [r1], #1 @ r4: *loVec++
    lsrs r4, r4, #1 @ r4: u32 currOffset
    ldrbcs r5, [r1], #1 @ r5: *loVec++
    addcs r4, r4, r5, lsl #7
    ldrb r5, [r1], #1 @ r5: *loVec++
    lsrs r5, r5, #1 @ r5: u32 currLength
    ldrbcs r6, [r1], #1 @ r6: *loVec++
    addscs r5, r5, r6, lsl #7
    beq .LDecodeInstructions_CopySyms @ if (currLength == 0) goto CopySyms
    ldrh r6, [r2], #2 @ r6: *symVec++
    strh r6, [r3], #2 @ *dest++ = r6
    cmp r4, #1
    bne .LDecodeInstructions_CopyDest

.LDecodeInstructions_Fill:
    orr r6, r6, r6, lsl #16
    tst r3, #2
    strhne r6, [r3], #2 @ *dest++ = r6
    subsne r5, #1 @ currLength--
.LDecodeInstructions_Fill_Loop32:
    str r6, [r3], #4 @ *dest++ = r6, *dest++ = r6
    subs r5, #2 @ currLength -= 2
    bgt .LDecodeInstructions_Fill_Loop32
    submi r3, #2 @ r6 -= 1 // if overflowed
    b .LDecodeInstructions_Loop

@.LDecodeInstructions_Fill:
@    strh r6, [r3], #2 @ *dest++ = r6
@    subs r5, #1 @ currLength--
@    bne .LDecodeInstructions_Fill
@    b .LDecodeInstructions_Loop

.LDecodeInstructions_CopySyms:
    ldrh r5, [r2], #2 @ r5: *symVec++
    strh r5, [r3], #2 @ *dest++ = r5
    subs r4, #1 @ currOffset--
    bne .LDecodeInstructions_CopySyms
    b .LDecodeInstructions_Loop

.LDecodeInstructions_Exit:
    pop {r4-r6}
    bx lr
.global DecodeInstructions_End
DecodeInstructions_End:
