#ifndef SYSTEM_HANDLER_H
#define SYSTEM_HANDLER_H

#include <stdint.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/semphr.h>
#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

extern QueueHandle_t xQueueShipsHigh;
extern QueueHandle_t xQueueShipsNormal;
extern QueueHandle_t xQueueShipsLow;

extern QueueHandle_t xQueueDirectTrucks;
extern QueueHandle_t xQueueYardTrucks;
extern QueueHandle_t xQueueCraneCommands;
extern QueueHandle_t xQueueLog;

extern SemaphoreHandle_t xYardMutex;
extern SemaphoreHandle_t xBerthSemaphore;

extern volatile SystemMode_t g_system_mode;
extern volatile uint32_t g_total_ships_processed;
extern volatile uint32_t g_total_direct_dispatches;
extern volatile uint32_t g_total_yard_dispatches;
extern volatile uint32_t g_dropped_logs;

void log_msg(const char *format, ...);
uint16_t system_get_waiting_ships(void);

#ifdef __cplusplus
}
#endif

#endif
