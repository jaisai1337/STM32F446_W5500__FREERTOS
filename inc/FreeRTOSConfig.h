#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

#include "stm32f4xx.h"   // For SystemCoreClock

/*-----------------------------------------------------------
 * Application specific definitions.
 * Adjust these to your hardware and application.
 *----------------------------------------------------------*/

/*---------------- System ----------------*/
#define configCPU_CLOCK_HZ                 ( SystemCoreClock )
#define configTICK_RATE_HZ                 ( (TickType_t)1000 )
#define configUSE_PREEMPTION               1
#define configUSE_PORT_OPTIMISED_TASK_SELECTION 1
#define configMAX_PRIORITIES               ( 5 )
#define configMINIMAL_STACK_SIZE           ( (unsigned short)128 )
#define configTOTAL_HEAP_SIZE              ( (size_t)(5 * 1024) )
#define configMAX_TASK_NAME_LEN            ( 16 )
#define configUSE_16_BIT_TICKS             0
#define configIDLE_SHOULD_YIELD            1
#define configUSE_MUTEXES                  1
#define configUSE_RECURSIVE_MUTEXES        1
#define configUSE_COUNTING_SEMAPHORES      1
#define configQUEUE_REGISTRY_SIZE          10
#define configUSE_NEWLIB_REENTRANT         0

/*---------------- Hook & Error Handling ----------------*/
#define configCHECK_FOR_STACK_OVERFLOW     0
#define configUSE_MALLOC_FAILED_HOOK       0
#define configUSE_IDLE_HOOK                0
#define configUSE_TICK_HOOK                0

/*---------------- Software Timers ----------------*/
#define configUSE_TIMERS                   0
#define configTIMER_TASK_PRIORITY          ( 2 )
#define configTIMER_QUEUE_LENGTH           5
#define configTIMER_TASK_STACK_DEPTH       ( configMINIMAL_STACK_SIZE * 2 )

/*---------------- Run-time & Debug ----------------*/
#define configGENERATE_RUN_TIME_STATS      0
#define configUSE_TRACE_FACILITY           0
#define configUSE_STATS_FORMATTING_FUNCTIONS 0
#define configUSE_APPLICATION_TASK_TAG     0

/*---------------- API Function Inclusion ----------------*/
#define INCLUDE_vTaskPrioritySet           1
#define INCLUDE_uxTaskPriorityGet          1
#define INCLUDE_vTaskDelete                1
#define INCLUDE_vTaskDelay                 1
#define INCLUDE_vTaskDelayUntil            1
#define INCLUDE_vTaskSuspend               1
#define INCLUDE_vTaskResume                1
#define INCLUDE_vTaskCleanUpResources      0
#define INCLUDE_xTaskGetSchedulerState     1
#define INCLUDE_xTaskGetCurrentTaskHandle  1
#define INCLUDE_xTaskGetIdleTaskHandle     0
#define INCLUDE_xQueueGetMutexHolder       1
#define INCLUDE_xSemaphoreGetMutexHolder   1
#define INCLUDE_xTaskGetHandle             1
#define INCLUDE_uxTaskGetStackHighWaterMark 1
#define INCLUDE_eTaskGetState              1
#define INCLUDE_xTimerPendFunctionCall     1

/*---------------- Cortex-M specific ----------------*/
#define configPRIO_BITS                    __NVIC_PRIO_BITS   // usually 4 for STM32F4
#define configLIBRARY_LOWEST_INTERRUPT_PRIORITY     15
#define configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY 5

#define configKERNEL_INTERRUPT_PRIORITY         ( configLIBRARY_LOWEST_INTERRUPT_PRIORITY << (8 - configPRIO_BITS) )
#define configMAX_SYSCALL_INTERRUPT_PRIORITY    ( configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY << (8 - configPRIO_BITS) )

/*-----------------------------------------------------------
 * Assert
 *----------------------------------------------------------*/
#define configASSERT(x) if((x) == 0) { taskDISABLE_INTERRUPTS(); for( ;; ); }

/*-----------------------------------------------------------
 * Map FreeRTOS interrupt handlers to CMSIS names
 *----------------------------------------------------------*/
#define vPortSVCHandler     SVC_Handler
#define xPortPendSVHandler  PendSV_Handler
#define xPortSysTickHandler SysTick_Handler

#endif /* FREERTOS_CONFIG_H */
