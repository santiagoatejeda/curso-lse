#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H
/*-----------------------------------------------------------
 * Application specific definitions.
 *
 * These definitions should be adjusted for your particular hardware and
 * application requirements.
 *
 * THESE PARAMETERS ARE DESCRIBED WITHIN THE 'CONFIGURATION' SECTION OF THE
 * FreeRTOS API DOCUMENTATION AVAILABLE ON THE FreeRTOS.org WEB SITE.
 *----------------------------------------------------------*/
/* Ensure definitions are only used by the compiler, and not by the assembler. */
#if defined(__ICCARM__) || defined(__CC_ARM) || defined(__GNUC__)
  #include <stdint.h>
  extern uint32_t SystemCoreClock;
#endif
#define configUSE_PREEMPTION							1
#define configUSE_IDLE_HOOK								0
#define configUSE_TICK_HOOK								0
#define configCPU_CLOCK_HZ								( SystemCoreClock )
#define configTICK_RATE_HZ								( ( TickType_t ) 1000 )
#define configMAX_PRIORITIES							( 8 )
#define configMINIMAL_STACK_SIZE					( ( uint16_t ) 64 )
#define configTOTAL_HEAP_SIZE							( ( size_t ) ( 8192 ) )
#define configMAX_TASK_NAME_LEN						( 16 )
#define configUSE_TRACE_FACILITY					1
#define configUSE_16_BIT_TICKS						0
#define configIDLE_SHOULD_YIELD						1
#define configUSE_MUTEXES									1
#define configQUEUE_REGISTRY_SIZE					8
#define configCHECK_FOR_STACK_OVERFLOW		0
#define configUSE_RECURSIVE_MUTEXES				1
#define configUSE_MALLOC_FAILED_HOOK			0
#define configUSE_APPLICATION_TASK_TAG		0
#define configUSE_COUNTING_SEMAPHORES			1
#define configGENERATE_RUN_TIME_STATS			0
#define configRECORD_STACK_HIGH_ADDRESS		1
#define configUSE_TICKLESS_IDLE						1
#define configSUPPORT_DYNAMIC_ALLOCATION	1
#define configSUPPORT_STATIC_ALLOCATION		0
/* Co-routine definitions. */
#define configUSE_CO_ROUTINES 						0
#define configMAX_CO_ROUTINE_PRIORITIES		( 2 )
/* Software timer definitions.  This example uses I2C to write to the LEDs.  As
this takes a finite time, and because a timer callback writes to an LED, the
priority of the timer task is kept to a minimum to ensure it does not disrupt
test tasks that check their own execution times. */
#define configUSE_TIMERS							0
#define configTIMER_TASK_PRIORITY			( 0 )
#define configTIMER_QUEUE_LENGTH			5
#define configTIMER_TASK_STACK_DEPTH	( configMINIMAL_STACK_SIZE * 2 )

/* Set the following definitions to 1 to include the API function, or zero
to exclude the API function. */
#define INCLUDE_vTaskPrioritySet					1
#define INCLUDE_uxTaskPriorityGet					1
#define INCLUDE_vTaskDelete								1
#define INCLUDE_vTaskCleanUpResources			1
#define INCLUDE_vTaskSuspend							1
#define INCLUDE_vTaskDelayUntil						1
#define INCLUDE_vTaskDelay								1
#define INCLUDE_xTaskGetCurrentTaskHandle 1
/* Cortex-M specific definitions. */
#ifdef __NVIC_PRIO_BITS
	/* __BVIC_PRIO_BITS will be specified when CMSIS is being used. */
	#define configPRIO_BITS       		__NVIC_PRIO_BITS
#else
	#define configPRIO_BITS       		2        /* 4 priority levels */
#endif
/* The lowest interrupt priority that can be used in a call to a "set priority"
function. */
#define configLIBRARY_LOWEST_INTERRUPT_PRIORITY		0x3
/* The highest interrupt priority that can be used by any interrupt service
routine that makes calls to interrupt safe FreeRTOS API functions.  DO NOT CALL
INTERRUPT SAFE FREERTOS API FUNCTIONS FROM ANY INTERRUPT THAT HAS A HIGHER
PRIORITY THAN THIS! (higher priorities are lower numeric values. */
#define configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY	5
/* Interrupt priorities used by the kernel port layer itself.  These are generic
to all Cortex-M ports, and do not rely on any particular library functions. */
#define configKERNEL_INTERRUPT_PRIORITY 		( configLIBRARY_LOWEST_INTERRUPT_PRIORITY << (8 - configPRIO_BITS) )
#define configMAX_SYSCALL_INTERRUPT_PRIORITY 	( configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY << (8 - configPRIO_BITS) )
/* Normal assert() semantics without relying on the provision of an assert.h
header file. */
#define configASSERT( x ) if( ( x ) == 0 ) { taskDISABLE_INTERRUPTS(); for( ;; ); }
#define configUSE_CUSTOM_TICK 0
/* Definitions that map the FreeRTOS port interrupt handlers to their CMSIS
standard names - or at least those used in the unmodified vector table. */
#define vPortSVCHandler SVC_Handler
#define xPortPendSVHandler PendSV_Handler
#define xPortSysTickHandler SysTick_Handler
#endif /* FREERTOS_CONFIG_H */
