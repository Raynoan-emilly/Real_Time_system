#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <stdlib.h>

#include "system_handler.h"
#include "config.h"
#include "types.h"

void vTaskGenerator(void *pvParameters) {
    vTaskDelay(pdMS_TO_TICKS(1000));
    uint32_t ship_id = 1;

    for (;;) {
        Ship_t *new_ship = (Ship_t *)malloc(sizeof(Ship_t));
        if (new_ship != NULL) {
            new_ship->id = ship_id++;
            new_ship->total_containers = 50;
            new_ship->remaining_containers = 50;

            new_ship->count_hazmat = rand() % 6;
            new_ship->count_reefer = (rand() % 10) + 5;
            new_ship->count_standard = new_ship->total_containers - 
                                      (new_ship->count_hazmat + new_ship->count_reefer);

            if (xQueueSend(xQueueShipsArrival, &new_ship, 0) == pdPASS) {
                UBaseType_t ships_in_queue = uxQueueMessagesWaiting(xQueueShipsArrival);
                UBaseType_t berths_free = uxSemaphoreGetCount(xBerthSemaphore);

                log_msg("[MARÍTIMO] Navio #%u atracou/ancorou. [Docas Livres: %u/10 | Fila no Mar: %u Navios]", 
                        new_ship->id, berths_free, ships_in_queue);
            } else {
                free(new_ship);
            }
        }

        // Navios chegam a cada 3.5s para forçar acúmulo no mar quando o cais travar
        vTaskDelay(pdMS_TO_TICKS(3500));
    }
}