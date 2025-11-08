#include "FreeRTOS.h"
#include "task.h"
#include "app_tasks.h"
#include "uart.h"
#include "systick.h"
#include "w5500_spi.h"

#define ETHERNET_INIT 1   // Set to 1 to initialize W5500 Ethernet

int main(void)
{
    SPI1_Init();
    SystemInit();                        // Initialize system
    uart2_tx_init();                    // Initialize UART2 for transmission
    systickDelayMs(100);              // Short delay for stability

#if ETHERNET_INIT
    uart2_write_string("\r\n=== W5500 Network Init ===\r\n");
    if (W5500_Init() == 0){
        uart2_write_string("Network ready.\r\n");
    }else{
        uart2_write_string("Init failed.\r\n");
    }
#endif
    App_CreateTasks();                  // Create application tasks
    vTaskStartScheduler();              // Start the FreeRTOS scheduler
    int count=1;
    while (1){
        DHCP_run();  // keep DHCP alive
        // uart2_write_string("Main loop heartbeat... ");
        // uart2_write_int(count++);
        // uart2_write_string("\r\n");
        systickDelayMs(1000);
    }
}

