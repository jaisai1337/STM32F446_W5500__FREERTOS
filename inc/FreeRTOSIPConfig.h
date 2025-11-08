#ifndef FREERTOS_IP_CONFIG_H
#define FREERTOS_IP_CONFIG_H

#include "stm32f4xx.h"   // For SystemCoreClock
/* Network settings (static fallback if DHCP fails) */
#define ipconfigBYTE_ORDER                          pdFREERTOS_LITTLE_ENDIAN
#define ipconfigUSE_DHCP                            1
#define ipconfigUSE_DNS                             1
#define ipconfigUSE_NBNS                            0
#define ipconfigUSE_LLMNR                           1

#define ipconfigNETWORK_MTU                         1500
//#define ipconfigNUM_NETWORK_BUFFER_DESCRIPTORS      8
//#define ipconfigEVENT_QUEUE_LENGTH                  10

#define ipconfigIP_TASK_STACK_SIZE_WORDS            ( 512 )
#define ipconfigIP_TASK_PRIORITY                    ( 3 )

#define ipconfigMAC_INTERRUPT_PRIORITY              ( configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY )

#define ipconfigUSE_TCP                             1
#define ipconfigUSE_UDP                             1
#define ipconfigTCP_MSS                             1460
#define ipconfigTCP_TX_BUFFER_LENGTH                ( 4 * ipconfigTCP_MSS )
#define ipconfigTCP_RX_BUFFER_LENGTH                ( 4 * ipconfigTCP_MSS )

//#define ipconfigRAND32()                            ulRand()

/* Optional debugging */
#define ipconfigHAS_DEBUG_PRINTF                    1
#define ipconfigHAS_PRINTF                          1
#define FreeRTOS_debug_printf(x)                    printf x
#define FreeRTOS_printf(x)                          printf x

#endif
