#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <freertos/semphr.h>
#include <stdarg.h>
#include <stdio.h>

#include "config.h"
#include "system_handler.h"
#include "tasks.h"
#include "hardware.h"
#include "types.h"

extern "C" {
QueueHandle_t xQueueShipsHigh = NULL;
QueueHandle_t xQueueShipsNormal = NULL;
QueueHandle_t xQueueShipsLow = NULL;

QueueHandle_t xQueueDirectTrucks = NULL;
QueueHandle_t xQueueYardTrucks = NULL;
QueueHandle_t xQueueCraneCommands = NULL;
QueueHandle_t xQueueLog = NULL;

SemaphoreHandle_t xYardMutex = NULL;
SemaphoreHandle_t xBerthSemaphore = NULL;

volatile SystemMode_t g_system_mode = SYSTEM_MODE_NORMAL;
volatile uint32_t g_total_ships_processed = 0;
volatile uint32_t g_total_direct_dispatches = 0;
volatile uint32_t g_total_yard_dispatches = 0;
volatile uint32_t g_dropped_logs = 0;
}

extern "C" uint16_t system_get_waiting_ships(void) {
    uint16_t total = 0;

    if (xQueueShipsHigh != NULL) {
        total += (uint16_t)uxQueueMessagesWaiting(xQueueShipsHigh);
    }
    if (xQueueShipsNormal != NULL) {
        total += (uint16_t)uxQueueMessagesWaiting(xQueueShipsNormal);
    }
    if (xQueueShipsLow != NULL) {
        total += (uint16_t)uxQueueMessagesWaiting(xQueueShipsLow);
    }

    return total;
}

extern "C" void log_msg(const char *format, ...) {
    char buffer[LOG_MESSAGE_LENGTH];

    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    if (xQueueLog == NULL) {
        return;
    }

    if (xQueueSend(xQueueLog, buffer, pdMS_TO_TICKS(10)) != pdPASS) {
        g_dropped_logs++;
    }
}

static bool create_resources(void) {
    xYardMutex = xSemaphoreCreateMutex();
    xBerthSemaphore = xSemaphoreCreateCounting(MAX_BERTHS, MAX_BERTHS);

    xQueueShipsHigh = xQueueCreate(
        MAX_SHIPS_PER_PRIORITY_QUEUE,
        sizeof(Ship_t)
    );
    xQueueShipsNormal = xQueueCreate(
        MAX_SHIPS_PER_PRIORITY_QUEUE,
        sizeof(Ship_t)
    );
    xQueueShipsLow = xQueueCreate(
        MAX_SHIPS_PER_PRIORITY_QUEUE,
        sizeof(Ship_t)
    );

    xQueueDirectTrucks = xQueueCreate(MAX_TRUCKS_QUEUE, sizeof(Truck_t));
    xQueueYardTrucks = xQueueCreate(MAX_TRUCKS_QUEUE, sizeof(Truck_t));
    xQueueCraneCommands = xQueueCreate(
        COMMAND_QUEUE_LENGTH,
        sizeof(CraneCommand_t)
    );
    xQueueLog = xQueueCreate(
        LOG_QUEUE_LENGTH,
        LOG_MESSAGE_LENGTH
    );

    return xYardMutex != NULL &&
           xBerthSemaphore != NULL &&
           xQueueShipsHigh != NULL &&
           xQueueShipsNormal != NULL &&
           xQueueShipsLow != NULL &&
           xQueueDirectTrucks != NULL &&
           xQueueYardTrucks != NULL &&
           xQueueCraneCommands != NULL &&
           xQueueLog != NULL;
}

static bool create_tasks(void) {
    BaseType_t result = pdPASS;

    result &= xTaskCreate(
        vTaskLogger,
        "Logger",
        2048,
        NULL,
        PRIO_TASK_LOGGER,
        NULL
    );

    result &= xTaskCreate(
        vTaskDashboard,
        "Dashboard",
        2560,
        NULL,
        PRIO_TASK_DASHBOARD,
        NULL
    );

    result &= xTaskCreate(
        vTaskGenerator,
        "Generator",
        2560,
        NULL,
        PRIO_TASK_GENERATOR,
        NULL
    );

    result &= xTaskCreate(
        vTaskTrucks,
        "Trucks",
        2304,
        NULL,
        PRIO_TASK_TRUCKS,
        NULL
    );

    result &= xTaskCreate(
        vTaskButtons,
        "Buttons",
        2048,
        NULL,
        PRIO_TASK_BUTTONS,
        NULL
    );

    result &= xTaskCreate(
        vTaskCranes,
        "Cranes",
        4096,
        NULL,
        PRIO_TASK_CRANES,
        NULL
    );

    return result == pdPASS;
}

void setup() {
    Serial.begin(115200);
    delay(300);

    hardware_init();

    if (!create_resources()) {
        Serial.println("ERRO CRITICO: falha ao criar filas, mutex ou semaforo.");
        return;
    }

    if (!create_tasks()) {
        Serial.println("ERRO CRITICO: falha ao criar uma ou mais tarefas.");
        return;
    }

    log_msg("Sistema portuario FreeRTOS inicializado.");
    log_msg(
        "Recursos: %u docas, patio para %u conteineres.",
        MAX_BERTHS,
        MAX_YARD_CAPACITY
    );
}

void loop() {
    vTaskDelay(pdMS_TO_TICKS(1000));
}
