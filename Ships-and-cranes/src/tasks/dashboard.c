#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <freertos/semphr.h>

#include "system_handler.h"
#include "hardware.h"
#include "yard.h"
#include "config.h"

static const char *mode_name(SystemMode_t mode) {
    switch (mode) {
        case SYSTEM_MODE_STRESS:
            return "ESTRESSE";
        case SYSTEM_MODE_RELIEF:
            return "ALIVIO";
        case SYSTEM_MODE_NORMAL:
        default:
            return "NORMAL";
    }
}

void vTaskDashboard(void *pvParameters) {
    (void)pvParameters;

    for (;;) {
        uint16_t ships_waiting =
            system_get_waiting_ships();

        UBaseType_t free_berths =
            uxSemaphoreGetCount(
                xBerthSemaphore
            );

        uint8_t docks_occupied =
            MAX_BERTHS -
            (uint8_t)free_berths;

        uint16_t direct_trucks =
            (uint16_t)uxQueueMessagesWaiting(
                xQueueDirectTrucks
            );

        uint16_t yard_trucks =
            (uint16_t)uxQueueMessagesWaiting(
                xQueueYardTrucks
            );

        uint16_t yard_count =
            yard_get_count();

        hardware_update_status_leds(
            ships_waiting,
            yard_count,
            g_system_mode
        );

        log_msg(
            ">>> PORTO | Modo: %s <<<",
            mode_name(g_system_mode)
        );

        log_msg(
            "Docas: %u/%u | Navios esperando: %u",
            docks_occupied,
            MAX_BERTHS,
            ships_waiting
        );

        log_msg(
            "Patio: %u/%u | Caminhoes direto: %u | patio: %u",
            yard_count,
            MAX_YARD_CAPACITY,
            direct_trucks,
            yard_trucks
        );

        log_msg(
            "Navios atendidos: %lu | Saidas diretas: %lu | Via patio: %lu | Logs perdidos: %lu",
            (unsigned long)g_total_ships_processed,
            (unsigned long)g_total_direct_dispatches,
            (unsigned long)g_total_yard_dispatches,
            (unsigned long)g_dropped_logs
        );

        vTaskDelay(pdMS_TO_TICKS(3000));
    }
}
