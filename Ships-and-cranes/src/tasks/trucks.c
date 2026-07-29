#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>

#include "system_handler.h"
#include "types.h"
#include "config.h"

void vTaskTrucks(void *pvParameters) {
    (void)pvParameters;

    uint32_t truck_id = 1;

    for (;;) {
        SystemMode_t mode = g_system_mode;

        uint32_t interval_ms = TRUCK_INTERVAL_NORMAL_MS;
        uint8_t batch = 1;

        if (mode == SYSTEM_MODE_STRESS) {
            interval_ms = TRUCK_INTERVAL_STRESS_MS;
        } else if (mode == SYSTEM_MODE_RELIEF) {
            interval_ms = TRUCK_INTERVAL_RELIEF_MS;
            batch = 2;
        }

        for (uint8_t i = 0; i < batch; i++) {
            Truck_t direct_truck = {
                .id = truck_id++,
                .is_loaded = 0
            };

            Truck_t yard_truck = {
                .id = truck_id++,
                .is_loaded = 0
            };

            xQueueSend(
                xQueueDirectTrucks,
                &direct_truck,
                0
            );

            xQueueSend(
                xQueueYardTrucks,
                &yard_truck,
                0
            );
        }

        vTaskDelay(pdMS_TO_TICKS(interval_ms));
    }
}
