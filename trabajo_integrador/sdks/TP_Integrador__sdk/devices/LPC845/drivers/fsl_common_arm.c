/*
 * Copyright (c) 2015-2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2021,2023,2024 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_common.h"

/* Component ID definition, used by tools. */
#ifndef FSL_COMPONENT_ID
#define FSL_COMPONENT_ID "platform.drivers.common_arm"
#endif

#ifndef __GIC_PRIO_BITS
#if defined(ENABLE_RAM_VECTOR_TABLE)
uint32_t InstallIRQHandler(IRQn_Type irq, uint32_t irqHandler)
{
#ifdef __VECTOR_TABLE
#undef __VECTOR_TABLE
#endif

    if (((int32_t)irq + 16) < 0)
    {
        return MSDK_INVALID_IRQ_HANDLER;
    }

/* Addresses for VECTOR_TABLE and VECTOR_RAM come from the linker file */
#if defined(__CC_ARM) || defined(__ARMCC_VERSION)
    extern uint32_t Image$$VECTOR_ROM$$Base[];
    extern uint32_t Image$$VECTOR_RAM$$Base[];
    extern uint32_t Image$$VECTOR_RAM$$ZI$$Limit[];

#define __VECTOR_TABLE          Image$$VECTOR_ROM$$Base
#define __VECTOR_RAM            Image$$VECTOR_RAM$$Base
#define __RAM_VECTOR_TABLE_SIZE (((uint32_t)Image$$VECTOR_RAM$$ZI$$Limit - (uint32_t)Image$$VECTOR_RAM$$Base))
#elif defined(__ICCARM__)
    extern uint32_t __RAM_VECTOR_TABLE_SIZE[];
    extern uint32_t __VECTOR_TABLE[];
    extern uint32_t __VECTOR_RAM[];
#elif defined(__GNUC__)
    extern uint32_t __VECTOR_TABLE[];
    extern uint32_t __VECTOR_RAM[];
    extern uint32_t __RAM_VECTOR_TABLE_SIZE_BYTES[];
    uint32_t __RAM_VECTOR_TABLE_SIZE = (uint32_t)(__RAM_VECTOR_TABLE_SIZE_BYTES);
#endif /* defined(__CC_ARM) || defined(__ARMCC_VERSION) */
    uint32_t n;
    uint32_t ret;
    uint32_t irqMaskValue;

    irqMaskValue = DisableGlobalIRQ();
    if (SCB->VTOR != (uint32_t)__VECTOR_RAM)
    {
        /* Copy the vector table from ROM to RAM */
        for (n = 0; n < ((uint32_t)__RAM_VECTOR_TABLE_SIZE) / sizeof(uint32_t); n++)
        {
            __VECTOR_RAM[n] = __VECTOR_TABLE[n];
        }
        /* Point the VTOR to the position of vector table */
        SCB->VTOR = (uint32_t)__VECTOR_RAM;
    }

    ret = __VECTOR_RAM[(int32_t)irq + 16];
    /* make sure the __VECTOR_RAM is noncachable */
    __VECTOR_RAM[(int32_t)irq + 16] = irqHandler;

    EnableGlobalIRQ(irqMaskValue);

    return ret;
}
#endif /* ENABLE_RAM_VECTOR_TABLE. */
#endif /* __GIC_PRIO_BITS. */

#if (defined(FSL_FEATURE_SOC_SYSCON_COUNT) && (FSL_FEATURE_SOC_SYSCON_COUNT > 0))

/*
 * When FSL_FEATURE_POWERLIB_EXTEND is defined to non-zero value,
 * powerlib should be used instead of these functions.
 */
#if !(defined(FSL_FEATURE_POWERLIB_EXTEND) && (FSL_FEATURE_POWERLIB_EXTEND != 0))

/*
 * When the SYSCON STARTER registers are discontinuous, these functions are
 * implemented in fsl_power.c.
 */
#if !(defined(FSL_FEATURE_SYSCON_STARTER_DISCONTINUOUS) && FSL_FEATURE_SYSCON_STARTER_DISCONTINUOUS)

void EnableDeepSleepIRQ(IRQn_Type interrupt)
{
    uint32_t intNumber = (uint32_t)interrupt;

    uint32_t index = 0;

    while (intNumber >= 32u)
    {
        index++;
        intNumber -= 32u;
    }

    SYSCON->STARTERSET[index] = 1UL << intNumber;
    (void)EnableIRQ(interrupt); /* also enable interrupt at NVIC */
}

void DisableDeepSleepIRQ(IRQn_Type interrupt)
{
    uint32_t intNumber = (uint32_t)interrupt;

    (void)DisableIRQ(interrupt); /* also disable interrupt at NVIC */
    uint32_t index = 0;

    while (intNumber >= 32u)
    {
        index++;
        intNumber -= 32u;
    }

    SYSCON->STARTERCLR[index] = 1UL << intNumber;
}
#endif /* FSL_FEATURE_SYSCON_STARTER_DISCONTINUOUS */
#endif /* FSL_FEATURE_POWERLIB_EXTEND */
#endif /* FSL_FEATURE_SOC_SYSCON_COUNT */

#if defined(DWT)
/* Use WDT. */
void MSDK_EnableCpuCycleCounter(void)
{
    /* Make sure the DWT trace fucntion is enabled. */
    if (CoreDebug_DEMCR_TRCENA_Msk != (CoreDebug_DEMCR_TRCENA_Msk & CoreDebug->DEMCR))
    {
        CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    }

    /* CYCCNT not supported on this device. */
    assert(DWT_CTRL_NOCYCCNT_Msk != (DWT->CTRL & DWT_CTRL_NOCYCCNT_Msk));

    /* Read CYCCNT directly if CYCCENT has already been enabled, otherwise enable CYCCENT first. */
    if (DWT_CTRL_CYCCNTENA_Msk != (DWT_CTRL_CYCCNTENA_Msk & DWT->CTRL))
    {
        DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
    }
}

uint32_t MSDK_GetCpuCycleCount(void)
{
    return DWT->CYCCNT;
}
#endif /* defined(DWT) */

#if !(defined(SDK_DELAY_USE_DWT) && defined(DWT))
/* Use software loop. */
#if defined(__CC_ARM) /* This macro is arm v5 specific */
/* clang-format off */
__ASM static void DelayLoop(uint32_t count)
{
loop
    SUBS R0, R0, #1
    CMP  R0, #0
    BNE  loop
    BX   LR
}
#elif defined(__ARM_ARCH_8A__) /* This macro is ARMv8-A specific */
static void DelayLoop(uint32_t count)
{
    __ASM volatile("    MOV    X0, %0" : : "r"(count));
    __ASM volatile(
        "loop%=:                        \n"
        "    SUB    X0, X0, #1          \n"
        "    CMP    X0, #0              \n"

        "    BNE    loop%=              \n"
        :
        :
        : "r0");
}
/* clang-format on */
#elif defined(__ARMCC_VERSION) || defined(__ICCARM__) || defined(__GNUC__)
/* Cortex-M0 has a smaller instruction set, SUBS isn't supported in thumb-16 mode reported from __GNUC__ compiler,
 * use SUB and CMP here for compatibility */
static void DelayLoop(uint32_t count)
{
    __ASM volatile("    MOV    R0, %0" : : "r"(count));
    __ASM volatile(
        "loop%=:                        \n"
#if defined(__GNUC__) && !defined(__ARMCC_VERSION)
        "    SUB    R0, R0, #1          \n"
#else
        "    SUBS   R0, R0, #1          \n"
#endif
        "    CMP    R0, #0              \n"

        "    BNE    loop%=              \n"
        :
        :
        : "r0");
}
#endif /* defined(__CC_ARM) */
#endif /* defined(SDK_DELAY_USE_DWT) && defined(DWT) */

/*!
 * @brief Delay at least for some time.
 *  Please note that, if not uses DWT, this API will use while loop for delay, different run-time environments have
 *  effect on the delay time. If precise delay is needed, please enable DWT delay. The two parmeters delayTime_us and
 *  coreClock_Hz have limitation. For example, in the platform with 1GHz coreClock_Hz, the delayTime_us only supports
 *  up to 4294967 in current code. If long time delay is needed, please implement a new delay function.
 *
 * @param delayTime_us  Delay time in unit of microsecond.
 * @param coreClock_Hz  Core clock frequency with Hz.
 */
void SDK_DelayAtLeastUs(uint32_t delayTime_us, uint32_t coreClock_Hz)
{
    uint64_t count;

    if (delayTime_us > 0U)
    {
        count = USEC_TO_COUNT(delayTime_us, coreClock_Hz);

        assert(count <= UINT32_MAX);

#if defined(SDK_DELAY_USE_DWT) && defined(DWT) /* Use DWT for better accuracy */

        MSDK_EnableCpuCycleCounter();
        /* Calculate the count ticks. */
        count += MSDK_GetCpuCycleCount();

        if (count > UINT32_MAX)
        {
            count -= UINT32_MAX;
            /* Wait for cyccnt overflow. */
            while (count < MSDK_GetCpuCycleCount())
            {
            }
        }

        /* Wait for cyccnt reach count value. */
        while (count > MSDK_GetCpuCycleCount())
        {
        }
#else
#if defined(__CORTEX_Axx) && ((__CORTEX_Axx == 53) || (__CORTEX_Axx == 55))
        /*
         * Cortex-A53/A55 execution throughput:
         *  - SUB/CMP: 2 instructions per cycle
         *  - BNE:     1 instruction per cycle
         * So, each loop takes 2 CPU cycles.
         */
        count = count / 2U;
#elif (__CORTEX_M == 7)
        /* Divide value may be different in various environment to ensure delay is precise.
         * Every loop count includes three instructions, due to Cortex-M7 sometimes executes
         * two instructions in one period, through test here set divide 1.5. Other M cores use
         * divide 4. By the way, divide 1.5 or 4 could let the count lose precision, but it does
         * not matter because other instructions outside while loop is enough to fill the time.
         */
        count = count / 3U * 2U;
#else
        count = count / 4U;
#endif
        DelayLoop((uint32_t)count);
#endif /* defined(SDK_DELAY_USE_DWT) && defined(DWT) */
    }
}

#if defined(FSL_FEATURE_MEASURE_CRITICAL_SECTION) && FSL_FEATURE_MEASURE_CRITICAL_SECTION
/* Use shall define their own IDs, FSL_FEATURE_CRITICAL_SECTION_MAX_ID and FSL_FEATURE_CRITICAL_SECTION_INVALID_ID
   for the critical sections if want to use the critical section measurement.
 */
#ifndef FSL_FEATURE_CRITICAL_SECTION_MAX_ID
#define FSL_FEATURE_CRITICAL_SECTION_MAX_ID 0xFFU
#endif

#ifndef FSL_FEATURE_CRITICAL_SECTION_INVALID_ID
#define FSL_FEATURE_CRITICAL_SECTION_INVALID_ID 0U
#endif

typedef struct
{
    uint32_t id; /*!< The id of the critical section, defined by user. */
    uint32_t startTime; /*!< The timestamp for the start of the critical section. */
    uint32_t dur_max[FSL_FEATURE_CRITICAL_SECTION_MAX_ID]; /*!< The maximum duration of the section's previous executions. */
    uint32_t execution_times[FSL_FEATURE_CRITICAL_SECTION_MAX_ID]; /*!< How many times the section is executed. */
    getTimestamp_t getTimestamp; /*!< Function to get the current time stamp. */
} critical_section_measurement_t;

static critical_section_measurement_t s_critical_section_measurement_context;

/*!
 * brief Initialize the context of the critical section measurement and assign
 * the function to get the current timestamp.
 *
 * param func The function to get the current timestamp.
 */
void InitCriticalSectionMeasurementContext(getTimestamp_t func)
{
    assert(func != NULL);

    (void)memset(&s_critical_section_measurement_context, 0, sizeof(critical_section_measurement_t));

    s_critical_section_measurement_context.getTimestamp = func;
}

/*!
 * brief Disable the global IRQ with critical section ID
 *
 * Extended function of DisableGlobalIRQ. Apart from the standard operation, also check
 * the id of the protected critical section and mark the begining for timer.
 * User is required to provided the primask register for the EnableGlobalIRQEx.
 *
 * param id The id for critical section.
 * return Current primask value.
 */
uint32_t DisableGlobalIRQEx(uint32_t id)
{
    uint32_t primask = DisableGlobalIRQ();
    if (primask != 0U)
    {
#ifdef FSL_FEATURE_MEASURE_CRITICAL_SECTION_DEBUG
        /* Check for the critical section id. */
        assert(id != FSL_FEATURE_CRITICAL_SECTION_INVALID_ID);
        assert(id < FSL_FEATURE_CRITICAL_SECTION_MAX_ID);
        assert(s_critical_section_measurement_context.id == FSL_FEATURE_CRITICAL_SECTION_INVALID_ID);
#endif
        if (s_critical_section_measurement_context.getTimestamp != NULL)
        {
            s_critical_section_measurement_context.id = id;
            s_critical_section_measurement_context.startTime = s_critical_section_measurement_context.getTimestamp();
        }
    }
    return primask;
}

/*!
 * brief Enable the global IRQ and calculate the execution time of critical section
 *
 * Extended function of EnableGlobalIRQ. Apart from the standard operation, also
 * marks the exit of the critical section and calculate the execution time for the section.
 * User is required to use the DisableGlobalIRQEx and EnableGlobalIRQEx in pair.
 *
 * param primask value of primask register to be restored. The primask value is supposed to be provided by the
 * DisableGlobalIRQEx().
 */
void EnableGlobalIRQEx(uint32_t primask)
{
    if (primask != 0U)
    {
#ifdef FSL_FEATURE_MEASURE_CRITICAL_SECTION_DEBUG
        /* Check for the critical section id. */
        assert(s_critical_section_measurement_context.id != FSL_FEATURE_CRITICAL_SECTION_INVALID_ID);
        assert(s_critical_section_measurement_context.id < FSL_FEATURE_CRITICAL_SECTION_MAX_ID);
#endif
        if (s_critical_section_measurement_context.getTimestamp != NULL)
        {
            /* Calculate the critical section duration. */
            uint32_t dur = s_critical_section_measurement_context.getTimestamp() - s_critical_section_measurement_context.startTime;
            if (dur > s_critical_section_measurement_context.dur_max[s_critical_section_measurement_context.id])
            {
                s_critical_section_measurement_context.dur_max[s_critical_section_measurement_context.id] = dur;
            }
            s_critical_section_measurement_context.execution_times[s_critical_section_measurement_context.id]++;
        }
#ifdef FSL_FEATURE_MEASURE_CRITICAL_SECTION_DEBUG
        /* Exit the critical section, set the id to invalid. In this case when entering critical
           section again DisableGlobalIRQEx has to be called first to avoid assertion. */
        s_critical_section_measurement_context.id = FSL_FEATURE_CRITICAL_SECTION_INVALID_ID;
#endif
    }
    EnableGlobalIRQ(primask);
}
#endif /* FSL_FEATURE_MEASURE_CRITICAL_SECTION */
