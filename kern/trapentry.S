/* See COPYRIGHT for copyright information. */

#include <inc/mmu.h>
#include <inc/memlayout.h>
#include <inc/trap.h>
#include <kern/macro.h>

#include <kern/picirq.h>

#ifdef CONFIG_KSPACE

.globl clock_thdlr
.type clock_thdlr, @function
clock_thdlr:
    call save_trapframe_trap
    # Set trap code for trapframe
    movl $(IRQ_OFFSET + IRQ_CLOCK), 136(%rsp)
    call trap
    jmp .

.globl timer_thdlr
.type timer_thdlr, @function
timer_thdlr:
    call save_trapframe_trap
    # Set trap code for trapframe
    movl $(IRQ_OFFSET + IRQ_TIMER), 136(%rsp)
    call trap
    jmp .

#else

# TRAPHANDLER defines a globally-visible function for handling a trap.
# It pushes a trap number onto the stack, then jumps to _alltraps.
# Use TRAPHANDLER for traps where the CPU automatically pushes an error code.
#
# You shouldn't call a TRAPHANDLER function from C, but you may
# need to _declare_ one in C (for instance, to get a function pointer
# during IDT setup).  You can declare the function with
#   void NAME();
# where NAME is the argument passed to TRAPHANDLER.

#define TRAPHANDLER(name, num) \
.globl name;                   \
.type name, @function;         \
.align 2;                      \
name:                          \
    pushq $(num);              \
    jmp _alltraps

# Use TRAPHANDLER_NOEC for traps where the CPU doesn't push an error code.
# It pushes a 0 in place of the error code, so the trap frame has the same
# format in either case.

#define TRAPHANDLER_NOEC(name, num) \
.globl name;                        \
.type name, @function;              \
.align 2;                           \
name:                               \
    pushq $0;                       \
    pushq $(num);                   \
    jmp _alltraps

.text

.globl _alltraps
.type _alltraps, @function;
.align 2
_alltraps:
    # LAB 8: Your code here
    # Complete `struct Trapframe' on stack
    # NOTE: Members after tf_paddind4 already on stack.
    #       For struct PushRegs you can use PUSHA macro.
    
    # Reserve space for tf_ds, tf_padding3, tf_padding4, tf_es, tf_padding1, tf_padding2
    subq $16, %rsp
    movw %ds, 8(%rsp)
    movw %es, (%rsp)
    
    PUSHA
    
    # Load kernel data segment into ds and es
    movw $(GD_KD), %ax
    movw %ax, %ds
    movw %ax, %es

    # LAB 8: Your code here
    # Invoke `trap' with a pointer to struct trap as an argument.
    
    # Pass pointer to trapframe as first argument (in rdi)
    movq %rsp, %rdi
    call trap
    
    jmp .

# LAB 8: Your code here
# Use TRAPHANDLER or TRAPHANDLER_NOEC to setup
# all trap handlers' entry points
# (Look up their list in inc/trap.h)

TRAPHANDLER_NOEC(divide_thdlr, T_DIVIDE)
TRAPHANDLER_NOEC(debug_thdlr, T_DEBUG)
TRAPHANDLER_NOEC(nmi_thdlr, T_NMI)
TRAPHANDLER_NOEC(brkpt_thdlr, T_BRKPT)
TRAPHANDLER_NOEC(oflow_thdlr, T_OFLOW)
TRAPHANDLER_NOEC(bound_thdlr, T_BOUND)
TRAPHANDLER_NOEC(illop_thdlr, T_ILLOP)
TRAPHANDLER_NOEC(device_thdlr, T_DEVICE)
TRAPHANDLER(dblflt_thdlr, T_DBLFLT)
/* T_COPROC 9 reserved */
TRAPHANDLER(tss_thdlr, T_TSS)
TRAPHANDLER(segnp_thdlr, T_SEGNP)
TRAPHANDLER(stack_thdlr, T_STACK)
TRAPHANDLER(gpflt_thdlr, T_GPFLT)
TRAPHANDLER(pgflt_thdlr, T_PGFLT)
/* T_RES 15 reserved */
TRAPHANDLER_NOEC(fperr_thdlr, T_FPERR)
TRAPHANDLER(align_thdlr, T_ALIGN)
TRAPHANDLER_NOEC(mchk_thdlr, T_MCHK)
TRAPHANDLER_NOEC(simderr_thdlr, T_SIMDERR)

TRAPHANDLER_NOEC(irq_timer_thdlr, IRQ_OFFSET + IRQ_TIMER)
TRAPHANDLER_NOEC(irq_kbd_thdlr, IRQ_OFFSET + IRQ_KBD)
TRAPHANDLER_NOEC(irq_2_thdlr, IRQ_OFFSET + 2)
TRAPHANDLER_NOEC(irq_3_thdlr, IRQ_OFFSET + 3)
TRAPHANDLER_NOEC(irq_serial_thdlr, IRQ_OFFSET + IRQ_SERIAL)
TRAPHANDLER_NOEC(irq_5_thdlr, IRQ_OFFSET + 5)
TRAPHANDLER_NOEC(irq_6_thdlr, IRQ_OFFSET + 6)
TRAPHANDLER_NOEC(irq_spurious_thdlr, IRQ_OFFSET + IRQ_SPURIOUS)
TRAPHANDLER_NOEC(irq_clock_thdlr, IRQ_OFFSET + IRQ_CLOCK)
TRAPHANDLER_NOEC(irq_9_thdlr, IRQ_OFFSET + 9)
TRAPHANDLER_NOEC(irq_10_thdlr, IRQ_OFFSET + 10)
TRAPHANDLER_NOEC(irq_11_thdlr, IRQ_OFFSET + 11)
TRAPHANDLER_NOEC(irq_12_thdlr, IRQ_OFFSET + 12)
TRAPHANDLER_NOEC(irq_13_thdlr, IRQ_OFFSET + 13)
TRAPHANDLER_NOEC(irq_ide_thdlr, IRQ_OFFSET + IRQ_IDE)
TRAPHANDLER_NOEC(irq_15_thdlr, IRQ_OFFSET + 15)
TRAPHANDLER_NOEC(irq_error_thdlr, IRQ_OFFSET + IRQ_ERROR)

TRAPHANDLER_NOEC(syscall_thdlr, T_SYSCALL)

#endif
