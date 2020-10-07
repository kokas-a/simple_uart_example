
/* Zero register:  Always returns 0 */
#define zero               $0

/* Assembler temporary register:  Reserved for use by the assembler */
#define at_reg             $1

/* Return value registers:  Value returned by function */
#define v0                 $2
#define v1                 $3

/* Argument registers:  First four parameters to a function */
#define a0                 $4
#define a1                 $5
#define a2                 $6
#define a3                 $7

/* Volatile registers: Registers that can be used without saving */
#define t0                 $8
#define t1                 $9
#define t2                 $10
#define t3                 $11
#define t4                 $12
#define t5                 $13
#define t6                 $14
#define t7                 $15

/* Stack pointer register:  Stack pointer */
#define sp                 $29

/* cp0 registers */
#define CP0_COUNT $9
#define CP0_CAUSE $13

/****************************************************************************
 *
 ****************************************************************************/
	.extern init

	.type	.Linitparms, %object
.Linitparms:
	.long	kbss_start
	.long	kbss_size
	.size	.Linitparms, . -.Linitparms

/****************************************************************************
 *
 ****************************************************************************/
	.section .text

	.set	noreorder

	.global __start
__start:
 		mtc0	zero, CP0_COUNT // Clear cp0 Count
 		nop

		li		a0, (1 << 27)	/* Stop counter */
		mtc0	a0, CP0_CAUSE
		nop

	zero_bss:
    	/* Clear uninitialized data sections */
		la		t0, kbss_start
		la		t1, kbss_size
		add     t1, t0, t1

		b		bsscheck
		nop

	bssloop:
		sw		zero, 0x0(t0)
		sw		zero, 0x4(t0)
		sw		zero, 0x8(t0)
		sw		zero, 0xc(t0)
		addu	t0,   16

	bsscheck:
		sltu    t2,   t0,   t1
		bne     t2,   zero, bssloop
		nop

		la		sp,   temp_stack

   		la 		a2,   init
    	jalr 	a2
    	nop

	all_done:
		/* Loop forewer */
    	b       all_done
    	nop

	.size __start, . - __start

/****************************************************************************
 *
 ****************************************************************************/
	.align 12
	.space 4096
	.global temp_stack
temp_stack:
