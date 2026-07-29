#ifndef SYSTEM_HANDLER_H
#define SYSTEM_HANDLER_H

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/semphr.h>

extern QueueHandle_t xQueueShipsArrival;
extern QueueHandle_t xQueueTrucksArrival;
extern QueueHandle_t xQueueCraneCommands;
extern QueueHandle_t xQueueLog;

extern SemaphoreHandle_t xYardMutex;
extern SemaphoreHandle_t xBerthSemaphore;

extern uint32_t g_total_ships_processed;

#ifdef __cplusplus
extern "C" {
#endif

void log_msg(const char *format, ...);

#ifdef __cplusplus
}
#endif

#endif