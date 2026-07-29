#include <freertos/FreeRTOS.h>
#include <stddef.h>
#include <freertos/semphr.h>

#include "yard.h"
#include "config.h"
#include "system_handler.h"

static uint8_t yard_grid[YARD_SIDE_LEN][YARD_SIDE_LEN];
static uint16_t total_yard_containers = 0;

uint8_t yard_add_container(ContainerType_t type) {
    if (xYardMutex == NULL) {
        return 0;
    }

    if (xSemaphoreTake(xYardMutex, portMAX_DELAY) != pdTRUE) {
        return 0;
    }

    if (total_yard_containers >= MAX_YARD_CAPACITY) {
        xSemaphoreGive(xYardMutex);
        return 0;
    }

    uint16_t slot = total_yard_containers;
    uint8_t row = (uint8_t)(slot / YARD_SIDE_LEN);
    uint8_t col = (uint8_t)(slot % YARD_SIDE_LEN);

    yard_grid[row][col] = (uint8_t)type + 1;
    total_yard_containers++;

    xSemaphoreGive(xYardMutex);
    return 1;
}

uint8_t yard_remove_container(void) {
    if (xYardMutex == NULL) {
        return 0;
    }

    if (xSemaphoreTake(xYardMutex, portMAX_DELAY) != pdTRUE) {
        return 0;
    }

    if (total_yard_containers == 0) {
        xSemaphoreGive(xYardMutex);
        return 0;
    }

    total_yard_containers--;

    uint16_t slot = total_yard_containers;
    uint8_t row = (uint8_t)(slot / YARD_SIDE_LEN);
    uint8_t col = (uint8_t)(slot % YARD_SIDE_LEN);

    yard_grid[row][col] = 0;

    xSemaphoreGive(xYardMutex);
    return 1;
}

uint16_t yard_get_count(void) {
    uint16_t count = 0;

    if (xYardMutex == NULL) {
        return 0;
    }

    if (xSemaphoreTake(xYardMutex, portMAX_DELAY) == pdTRUE) {
        count = total_yard_containers;
        xSemaphoreGive(xYardMutex);
    }

    return count;
}
