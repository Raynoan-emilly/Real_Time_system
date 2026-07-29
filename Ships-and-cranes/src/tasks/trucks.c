#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>

#include "system_handler.h"
#include "types.h"
#include "yard.h"

QueueHandle_t xQueueYardTrucks = NULL;

void vTaskTrucks(void *pvParameters) {
    uint32_t truck_id = 1;

    for (;;) {
        Truck_t main_truck = { .id = truck_id++, .is_loaded = 0 };
        Truck_t yard_truck = { .id = truck_id++, .is_loaded = 0 };

        // Envia caminhões para o cais e para o pátio
        xQueueSend(xQueueTrucksArrival, &main_truck, 0);

        if (xQueueYardTrucks != NULL) {
            xQueueSend(xQueueYardTrucks, &yard_truck, 0);
        }

        // Se o pátio passar de 50 contêineres, acelera a chegada na portaria para 150ms!
        uint16_t yard_count = yard_get_count();
        uint32_t delay_ms = (yard_count > 50) ? 150 : 400;

        vTaskDelay(pdMS_TO_TICKS(delay_ms));
    }
}