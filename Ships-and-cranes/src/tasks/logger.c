#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>

#include "system_handler.h"
#include "config.h"

void vTaskLogger(void *pvParameters) {
    (void)pvParameters;

    char log_buffer[LOG_MESSAGE_LENGTH];

    for (;;) {
        if (xQueueReceive(
                xQueueLog,
                log_buffer,
                portMAX_DELAY
            ) == pdTRUE) {
            printf("%s\n", log_buffer);
        }
    }
}
