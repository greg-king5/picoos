/*
 * Copyright (c) 2016, Ari Suutari <ari@stonepile.fi>.
 * All rights reserved. 
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *  3. The name of the author may not be used to endorse or promote
 *     products derived from this software without specific prior written
 *     permission. 
 * 
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
 * INDIRECT,  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */


/**
 * @file    port_ex.h
 *
 * This file is originally from the pico]OS realtime operating system
 * (http://picoos.sourceforge.net).
 */


#ifndef _PORT_EX_H
#define _PORT_EX_H

/*
 * Define IRQ priorities. Use defaults from port.h if
 * not overridden in poscfg.h
 */

#ifdef PORTCFG_API_MAX_PRI
#define PORT_API_MAX_PRI PORTCFG_API_MAX_PRI
#else
#define PORT_API_MAX_PRI PORT_DEFAULT_API_MAX_PRI
#endif

#ifdef PORTCFG_SVCALL_PRI
#define PORT_SVCALL_PRI PORTCFG_SVCALL_PRI
#else
#define PORT_SVCALL_PRI PORT_DEFAULT_SVCALL_PRI
#endif

#ifdef PORTCFG_SYSTICK_PRI
#define PORT_SYSTICK_PRI PORTCFG_SYSTICK_PRI
#else
#define PORT_SYSTICK_PRI PORT_DEFAULT_SYSTICK_PRI
#endif

#ifdef PORTCFG_PENDSV_PRI
#define PORT_PENDSV_PRI PORTCFG_PENDSV_PRI
#else
#define PORT_PENDSV_PRI PORT_DEFAULT_PENDSV_PRI
#endif

#ifdef PORTCFG_CON_PRI
#define PORT_CON_PRI PORTCFG_CON_PRI
#else
#define PORT_CON_PRI PORT_DEFAULT_CON_PRI
#endif

#ifdef _DBG
#define HAVE_PLATFORM_ASSERT
extern void p_pos_assert(const char* text, const char *file, int line);
#endif

/**
 * Macro to save context of current stack.
 */
#if __CORTEX_M >= 4

#define portSaveContext() { \
    register void* pspReg asm("r0");                    \
    asm volatile("mrs %0, psp             \n\t"         \
                 "tst r14, #0x10          \n\t"         \
                 "it  eq                  \n\t"         \
                 "vstmdbeq %0!, {s16-s31} \n\t"         \
                 "stmdb %0!, {r4-r11,r14}   "           \
                  : "=r"(pspReg));                      \
                                                        \
    posCurrentTask_g->stackptr = pspReg;                \
    posCurrentTask_g->critical = portSchedStatus();     \
                                                        \
    if (POSCFG_ARGCHECK > 1)                                              \
      P_ASSERT("TStk", (posCurrentTask_g->stack[0] == PORT_STACK_MAGIC)); \
}

#elif __CORTEX_M == 3

#define portSaveContext() { \
    register void* pspReg asm("r0");                    \
    asm volatile("mrs %0, psp             \n\t"         \
                 "stmdb %0!, {r4-r11,r14}   "           \
                  : "=r"(pspReg));                      \
                                                        \
    posCurrentTask_g->stackptr = pspReg;                \
    posCurrentTask_g->critical = portSchedStatus();     \
                                                        \
    if (POSCFG_ARGCHECK > 1)                                              \
      P_ASSERT("TStk", (posCurrentTask_g->stack[0] == PORT_STACK_MAGIC)); \
}

#else

#define portSaveContext() { \
    register void* pspReg asm("r0");                    \
    asm volatile("mrs %0, psp           \n\t"           \
                 "sub %0, %0, #4*9      \n\t"           \
                 "mov r1, %0            \n\t"           \
                 "stmia r1!, {r4-r7}    \n\t"           \
                 "mov r3, r8            \n\t"           \
                 "mov r4, r9            \n\t"           \
                 "mov r5, r10           \n\t"           \
                 "mov r6, r11           \n\t"           \
                 "mov r7, lr            \n\t"           \
                 "stmia r1!, {r3-r7}        "           \
                  : "=r"(pspReg) :: "r1");              \
                                                        \
    posCurrentTask_g->stackptr = pspReg;                \
    posCurrentTask_g->critical = portSchedStatus();     \
                                                        \
    if (POSCFG_ARGCHECK > 1)                                              \
      P_ASSERT("TStk", (posCurrentTask_g->stack[0] == PORT_STACK_MAGIC)); \
}

#endif


/**
 * Restore context for current task.
 */
#define portRestoreContext() asm volatile("b portRestoreContextImpl")

extern void portRestoreContextImpl(void);
extern unsigned char *portIrqStack;

void portInitClock(void);
void portInitConsole(void);
void portSystemInit(void);
void portRestoreClocksAfterWakeup(void);

typedef void (* const PortExcHandlerFunc)(void);

#ifndef __CODE_RED
void Reset_Handler(void);
#endif

// Helpful macro to define weak interrupt handler definitions.

#define PORT_WEAK_HANDLER(n) void __attribute__((weak, alias("Default_Handler"))) n()

void SVC_Handler(void);
void PendSV_Handler(void);
void SysTick_Handler(void);
void HardFault_Handler(void);
void UsageFault_Handler(void);
void Uart_Handler(void);

// Prototypes for malloc wrapping.

void* __wrap_malloc(size_t s);
void* __wrap_realloc(void* p, size_t s);
void  __wrap_free(void* p);

void* __real_malloc(size_t s);
void* __real_realloc(void* p, size_t s);
void  __real_free(void* p);

#if PORTCFG_NVIC_SCHED_LOCK
extern bool portNvicSchedLock;
extern uint32_t portNvicEnabledInterrupts;
#endif

/**
 * Get current scheduler lock status. Used during context save.
 */
static inline __attribute__((always_inline)) uint32_t portSchedStatus(void)
{
#if __CORTEX_M >= 3
  return __get_BASEPRI();
#else
#if PORTCFG_NVIC_SCHED_LOCK

  return portNvicSchedLock;

#else
  return __get_PRIMASK();
#endif
#endif
}

/**
 * Lock task scheduler by blocking interrupts (selectively if possible).
 * This is implementation for POS_SCHED_LOCK.
 * @return previous lock status. Used as argument for ::portSchedUnlock.
 */
static inline __attribute__((always_inline)) uint32_t portSchedLock(void)
{
  register uint32_t flags;

#if __CORTEX_M >= 3

  flags = __get_BASEPRI();
  __set_BASEPRI(portCmsisPrio2HW(PORT_API_MAX_PRI));

#else
#if PORTCFG_NVIC_SCHED_LOCK

  __disable_irq();

  flags = portNvicSchedLock;
  if (!portNvicSchedLock) {

    portNvicSchedLock = true;
    portNvicEnabledInterrupts = (NVIC->ICER[0] & PORTCFG_NVIC_SCHED_LOCK_IRQS);
    NVIC->ICER[0] = PORTCFG_NVIC_SCHED_LOCK_IRQS;
  }

  __enable_irq();

#else

  flags = __get_PRIMASK();
  __disable_irq();

#endif
#endif

  return flags;
}

/**
 * Unlock task scheduler by returning interrupt blocking status to previous state.
 * This is implementation for POS_SCHED_UNLOCK.
 * @param flags previous interrupt status (obtained from ::portSchedStatus or ::portSchedLock). 
 */
static inline __attribute__((always_inline)) void portSchedUnlock(uint32_t flags)
{
#if __CORTEX_M >= 3

  __set_BASEPRI(flags);

#else
#if PORTCFG_NVIC_SCHED_LOCK

  if (portNvicSchedLock && !flags) {

    __disable_irq();
    NVIC->ISER[0] = portNvicEnabledInterrupts;
    portNvicSchedLock = false;
    __enable_irq();
  }

#else

  if (!flags)
    __enable_irq();

#endif
#endif
}

#endif /* _PORT_EX_H */
