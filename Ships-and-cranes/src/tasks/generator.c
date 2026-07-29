#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <esp_system.h>

#include "system_handler.h"
#include "config.h"
#include "types.h"

static QueueHandle_t queue_for_priority(ShipPriority_t priority) {
    switch (priority) {
        case SHIP_PRIORITY_HIGH:
            return xQueueShipsHigh;
        case SHIP_PRIORITY_NORMAL:
            return xQueueShipsNormal;
        case SHIP_PRIORITY_LOW:
        default:
            return xQueueShipsLow;
    }
}

static const char *priority_name(ShipPriority_t priority) {
    switch (priority) {
        case SHIP_PRIORITY_HIGH:
            return "ALTA";
        case SHIP_PRIORITY_NORMAL:
            return "NORMAL";
        case SHIP_PRIORITY_LOW:
        default:
            return "BAIXA";
    }
}

void vTaskGenerator(void *pvParameters) {
    (void)pvParameters;

    uint32_t ship_id = 1;
    vTaskDelay(pdMS_TO_TICKS(1000));

    for (;;) {
        Ship_t ship = {0};

        uint32_t random_value = esp_random();
        uint16_t range =
            SHIP_MAX_CONTAINERS -
            SHIP_MIN_CONTAINERS +
            1;

        ship.id = ship_id++;
        ship.total_containers =
            SHIP_MIN_CONTAINERS +
            (uint16_t)(random_value % range);
        ship.remaining_containers = ship.total_containers;

        ship.count_hazmat =
            (uint16_t)(esp_random() % 5);

        ship.count_reefer =
            (uint16_t)(3 + (esp_random() % 9));

        if (ship.count_hazmat + ship.count_reefer >
            ship.total_containers) {
            ship.count_reefer =
                ship.total_containers -
                ship.count_hazmat;
        }

        ship.count_standard =
            ship.total_containers -
            ship.count_hazmat -
            ship.count_reefer;

        uint8_t priority_draw =
            (uint8_t)(esp_random() % 100);

        if (ship.count_hazmat > 0 || priority_draw < 20) {
            ship.priority = SHIP_PRIORITY_HIGH;
        } else if (priority_draw < 65) {
            ship.priority = SHIP_PRIORITY_NORMAL;
        } else {
            ship.priority = SHIP_PRIORITY_LOW;
        }

        ship.arrival_tick = xTaskGetTickCount();

        QueueHandle_t destination =
            queue_for_priority(ship.priority);

        if (xQueueSend(destination, &ship, 0) == pdPASS) {
            log_msg(
                "[MAR] Navio #%u chegou: %u conteineres, prioridade %s.",
                ship.id,
                ship.total_containers,
                priority_name(ship.priority)
            );
        } else {
            log_msg(
                "[MAR] Navio #%u recusado: fila %s cheia.",
                ship.id,
                priority_name(ship.priority)
            );
        }

        vTaskDelay(
            pdMS_TO_TICKS(SHIP_ARRIVAL_INTERVAL_MS)
        );
    }
}
