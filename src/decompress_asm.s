	.syntax unified

	.include "constants/gba_constants.inc"

	.arm
	.section .iwram.code, "ax", %progbits
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

.global RlFastUncompUnsafe

	// header (see src):
	// struct RLFrameHeader {
	//   u16 frame_size_tiles: 8;
	//   u16 n_frames: 8;
	//   u16 offsets[n_frames]; // relative to &src->offsets, not src
	// }
	// followed by compressed data frames:
	// struct RLFrame {
	//   u16 zerofill_halfwords: 8;
	//   u16 copy_halfwords: 8;
	//   u16 data[copy_halfwords];
	// }

	@ r0 = src (word aligned)
	@ r1 = dst (word aligned)
	@ r2 = frame_index

RlFastUncompUnsafe:
	ldrh r3, [r0], #2 // r3 = (frame_size_tiles - 1) | (n_frames << 8)
	cmp r2, r3, lsr #8 // check if frame_index is out of bounds
	bxge lr

	push {r4-r6}

	lsl r2, r2, #1
	ldrh r2, [r0, r2]
	add r0, r2 // r0 = src->offset + src->offset[frame_index]

	and r4, r3, #0x00FF // r4 = frame_size_tiles - 1
	add r4, r4, #0x01 // r4 = frame_size_tiles
	add r4, r1, r4, lsl #5 // r4 = dst + (frame_size_tiles << 5) = dst + frame_size_bytes

	mov r5, REG_BASE
	orr r5, OFFSET_REG_DMA3SAD

	mov r6, #0

rlz_loop:
	ldrh r2, [r0], #2 // zerofill_halfwords | (copy_halfwords << 8)

// fill stage
// TODO: Consider DMA. We'd need a source address in IWRAM (probably PC-
// relative), and to set DMA_SRC_FIXED.
	and r3, r2, #0xFF
branch_fill_loop:
	subs r3, #1
	strhge r6, [r1], #2
	bgt branch_fill_loop

// copy stage
// HINT: copies are very common so rather than branch and pay a 1 cycle
// penalty on non-zero-sized copies, pay a 1 cycle penalty on zero-sized
// copies (a branch costs 3 cycles: 4 - 3 = 1).
	lsrs r2, #8
	orrne r2, DMA_ENABLE << 16
	stmiane r5, {r0, r1, r2}
	// HINT: DMA_ENABLE << 16 is the MSB, so 'lsl #1' clears it.
	addne r0, r0, r2, lsl #1
	addne r1, r1, r2, lsl #1

	cmp r1, r4
	bne rlz_loop

	pop {r4-r6}
	bx lr

	.section .text @Copied to stack on run-time
	.align 2

.global LZ77UnCompWRAMOptimized
.type LZ77UnCompWRAMOptimized, %function
LZ77UnCompWRAMOptimized: @ 0x000010FC
	push {r4, r5, r6, lr}
	// read in data header in r5
	// Data header (32bit)
	// Bit 0-3   Reserved
	// Bit 4-7   Compressed type (must be 1 for LZ77)
	// Bit 8-31  Size of decompressed data
	ldr r5, [r0], #4
	// store decompressed size in r2
	lsr r2, r5, #8
	// main loop
	cmp r2, #0
	ble LZ77_Done
LZ77_MainLoop:
	// read in Flag Byte
	// Flag data (8bit)
	// Bit 0-7   Type Flags for next 8 Blocks, MSB first
	ldrb lr, [r0], #1
	// shift to the highest byte
	lsl lr, lr, #24
	// 8 blocks so set counter (r4) to 8
	mov r4, #8
	b LZ77_EightBlockLoop
LZ77_HandleCompressedData:
	// reading in block type 1 Part 1 into r5
	// Block Type 1 Part 1 - Compressed - Copy N+3 Bytes from Dest-Disp-1 to Dest
	// Bit 0-3   Disp MSBs
	// Bit 4-7   Number of bytes to copy (minus 3)
	// byte copy range: [3, 18]
	ldrb r5, [r0], #1
	

	// 18 -> 0
	// 17 -> 1
	// 16 -> 2
	// ...
	// 3 -> 15
	// formula: do 18 - x
	// want to calculate r3 = 18 - (3 + (numBytesToCopy))
	// r3 = 18 - 3 - (numBytesToCopy)
	// r3 = 15 - numBytesToCopy
	// but then also need to do r2 = r2 - (3 + (numBytesToCopy))
	// r2 = r2 - 3 - numBytesToCopy
	// r2 = r2 - 18 + 18 - 3 - numBytesToCopy
	// r2 = r2 - 18 + 15 - numBytesToCopy
	
	mov r6, #3
	// r3 = 3 + (numBytesToCopy)
	add r3, r6, r5, asr #4
	// get displacement high bits
	and r5, r5, #0xf
	// Now reading Block Type 1 Part 2 into r6
	// Block type 1 Part 2
	// Bit 0-7  Disp LSBs
	ldrb r6, [r0], #1
	// combine low and high bits into r6
	orr r6, r6, r5, lsl #8
	// +1 because of reasons
	add r6, r6, #1
	// subtract how many bytes are going to be copied from the size
	subs r2, r2, r3

	// do duff's device
	// https://en.wikipedia.org/wiki/Duff%27s_device
	// calculate pc offset
	rsb r3, r3, #18
	// jump
	add pc, pc, r3, lsl #3
	nop
	.rept 18
	ldrb r5, [r1, -r6]
	strb r5, [r1], #1
	.endr

	// cpsr flags still preserved from earlier
	// check if no more bytes have to be copied
	ble LZ77_Done
	// check if end of the block
	subs r4, r4, #1
	ble LZ77_MainLoop
LZ77_EightBlockLoop:
	// check if compressed data (bit set)
	lsls lr, lr, #1
	bcs LZ77_HandleCompressedData
	// uncompressed data can only be 1 byte long
	// copy one byte of uncompressed data
	ldrb r6, [r0], #1
	strb r6, [r1], #1
	subs r2, r2, #1
	ble LZ77_Done
LZ77_EightBlockLoop_HandleLoop:
	// check if we're done with the 8 blocks
	subs r4, r4, #1
	bgt LZ77_EightBlockLoop // go back to main loop if so
	// no need to check if r2 is 0 since already checked elsewhere
	b LZ77_MainLoop
LZ77_Done:
	pop {r4, r5, r6, lr}
	bx lr

.global LZ77UnCompWRAMOptimized_end
LZ77UnCompWRAMOptimized_end:
