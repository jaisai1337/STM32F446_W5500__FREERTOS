#include "app_tasks.h"
#include "uart.h"
#include <stdio.h>

#define QUEUE_LENGTH      5
#define QUEUE_ITEM_SIZE   sizeof(char *)

QueueHandle_t msgQueue;

// Task 1 — sends messages
static void vTaskSender1(void *pvParameters)
{
    const char *msg = "Message from Task 1";
    while (1)
    {
        if (xQueueSend(msgQueue, &msg, portMAX_DELAY) == pdPASS)
        {
            //printf("Task 1 sent message\n");
            uart2_write_string("Task 1 sent message\r\n");
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

// Task 2 — sends messages
static void vTaskSender2(void *pvParameters)
{
    const char *msg = "Message from Task 2";
    while (1)
    {
        if (xQueueSend(msgQueue, &msg, portMAX_DELAY) == pdPASS)
        {
            //printf("Task 2 sent message\n");
            uart2_write_string("Task 2 sent message\r\n");
        }
        vTaskDelay(pdMS_TO_TICKS(1500));
    }
}

// Task 3 — receives and prints messages
static void vTaskReceiver(void *pvParameters)
{
    char *recvMsg;
    while (1)
    {
        if (xQueueReceive(msgQueue, &recvMsg, portMAX_DELAY) == pdPASS)
        {
            //printf("Received: %s\n", recvMsg);
            uart2_write_string("Received: ");
            uart2_write_string(recvMsg);
            uart2_write_string("\r\n");
        }
    }
}

/* Public function to initialize queue and create tasks */
void App_CreateTasks(void)
{
    msgQueue = xQueueCreate(QUEUE_LENGTH, QUEUE_ITEM_SIZE);
    if (msgQueue == NULL)
    {
        //printf("Queue creation failed!\n");
        uart2_write_string("Queue creation failed!\r\n");
        while (1);
    }

    xTaskCreate(vTaskSender1, "Sender1", 256, NULL, 1, NULL);
    xTaskCreate(vTaskSender2, "Sender2", 256, NULL, 1, NULL);
    xTaskCreate(vTaskReceiver, "Receiver", 256, NULL, 2, NULL);
}
