#ifndef APP_TASKS_H
#define APP_TASKS_H

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

extern QueueHandle_t msgQueue;

void App_CreateTasks(void);

#endif
