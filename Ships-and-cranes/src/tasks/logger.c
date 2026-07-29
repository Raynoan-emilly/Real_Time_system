#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <stdio.h>

#include "system_handler.h"

void vTaskLogger(void *pvParameters) {
    char log_buffer[128];

    for (;;) {
        // Bloqueia até haver um novo log enviado por qualquer task
        if (xQueueReceive(xQueueLog, log_buffer, portMAX_DELAY) == pdTRUE) {
            printf("%s\n", log_buffer);
        }
    }
}