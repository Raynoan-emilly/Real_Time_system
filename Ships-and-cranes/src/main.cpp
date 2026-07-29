#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <freertos/semphr.h>
#include <stdarg.h>
#include <stdio.h>

#include "system_handler.h"
#include "tasks.h"
#include "hardware.h"
#include "types.h"

// Instanciação dos Handles e Globais
QueueHandle_t xQueueShipsArrival = NULL;
QueueHandle_t xQueueTrucksArrival = NULL;
QueueHandle_t xQueueCraneCommands = NULL;
QueueHandle_t xQueueLog = NULL;

SemaphoreHandle_t xYardMutex = NULL;
SemaphoreHandle_t xBerthSemaphore = NULL;

uint32_t g_total_ships_processed = 0;

// =========================================================================
// IMPLEMENTAÇÃO DE LOG_MSG COM EXTERNAL LINKAGE C
// =========================================================================
extern "C" void log_msg(const char *format, ...) {
    char buffer[128];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    if (xQueueLog != NULL) {
        xQueueSend(xQueueLog, buffer, 0);
    }
}

void setup() {
    Serial.begin(115200);
    hardware_init();

    // 1. Semáforos e Mutexes
    xYardMutex = xSemaphoreCreateMutex();
    xBerthSemaphore = xSemaphoreCreateCounting(10, 10); // 10 docas

    // 2. Queues
    xQueueShipsArrival  = xQueueCreate(50, sizeof(Ship_t *));
    xQueueTrucksArrival = xQueueCreate(50, sizeof(Truck_t));
    xQueueCraneCommands = xQueueCreate(10, sizeof(CraneCommand_t));
    xQueueLog          = xQueueCreate(20, sizeof(char) * 128);

    // 3. Tasks
    if (xQueueShipsArrival && xQueueTrucksArrival && xQueueCraneCommands && 
        xQueueLog && xYardMutex && xBerthSemaphore) {
        
        xTaskCreate(vTaskGenerator, "Generator", 2048, NULL, 2, NULL);
        xTaskCreate(vTaskTrucks,    "Trucks",    2048, NULL, 2, NULL);
        xTaskCreate(vTaskCranes,    "Cranes",    3072, NULL, 3, NULL);
        xTaskCreate(vTaskDashboard, "Dashboard", 2048, NULL, 1, NULL);
        xTaskCreate(vTaskLogger,    "Logger",    2048, NULL, 1, NULL);
        xTaskCreate(vTaskButtons,   "Buttons",   2048, NULL, 2, NULL);

        log_msg(">>> Sistema FreeRTOS das 10 Docas Inicializado com Sucesso <<<");
    } else {
        Serial.println("Erro crítico: Falha ao alocar recursos do FreeRTOS.");
    }
}

void loop() {
    vTaskDelay(pdMS_TO_TICKS(500));
}