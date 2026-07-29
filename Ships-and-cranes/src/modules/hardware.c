#include <Arduino.h>
#include <stddef.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>

#include "hardware.h"
#include "system_handler.h"
#include "types.h"
#include "config.h"

void hardware_init(void) {
    pinMode(LED_GREEN_PIN, OUTPUT);
    pinMode(LED_YELLOW_PIN, OUTPUT);
    pinMode(LED_RED_PIN, OUTPUT);

    pinMode(BTN_TRIGGER_STRESS_PIN, INPUT_PULLUP);
    pinMode(BTN_FORCE_RELIEF_PIN, INPUT_PULLUP);
    pinMode(BTN_RESET_NORMAL_PIN, INPUT_PULLUP);

    digitalWrite(LED_GREEN_PIN, HIGH);
    digitalWrite(LED_YELLOW_PIN, LOW);
    digitalWrite(LED_RED_PIN, LOW);
}

void hardware_update_status_leds(
    uint16_t ships_waiting,
    uint16_t yard_count,
    SystemMode_t mode
) {
    bool red = mode == SYSTEM_MODE_STRESS;

    bool yellow = mode == SYSTEM_MODE_RELIEF ||
                   yard_count >= YARD_WARNING_THRESHOLD ||
                   ships_waiting >= SHIPS_WARNING_THRESHOLD;
    
    bool green = !red && !yellow;

    digitalWrite(LED_GREEN_PIN, green ? HIGH : LOW);
    digitalWrite(LED_YELLOW_PIN, yellow ? HIGH : LOW);
    digitalWrite(LED_RED_PIN, red ? HIGH : LOW);
}

static void send_command(CraneCommand_t command) {
    if (xQueueCraneCommands == NULL) {
        return;
    }

    if (xQueueSend(xQueueCraneCommands, &command, 0) != pdPASS) {
        log_msg("[BOTOES] Fila de comandos cheia.");
    }
}

void vTaskButtons(void *pvParameters) {
    (void)pvParameters;

    const uint8_t pins[3] = {
        BTN_TRIGGER_STRESS_PIN,
        BTN_FORCE_RELIEF_PIN,
        BTN_RESET_NORMAL_PIN
    };

    const CraneCommand_t commands[3] = {
        CMD_TRIGGER_STRESS,
        CMD_RELIEVE_STRESS,
        CMD_RESET_NORMAL
    };

    uint8_t last_state[3] = {HIGH, HIGH, HIGH};
    TickType_t last_press[3] = {0, 0, 0};

    for (;;) {
        TickType_t now = xTaskGetTickCount();

        for (uint8_t i = 0; i < 3; i++) {
            uint8_t current_state = digitalRead(pins[i]);

            bool falling_edge =
                last_state[i] == HIGH &&
                current_state == LOW;

            bool debounce_ok =
                (now - last_press[i]) >=
                pdMS_TO_TICKS(BUTTON_DEBOUNCE_MS);

            if (falling_edge && debounce_ok) {
                send_command(commands[i]);
                last_press[i] = now;
            }

            last_state[i] = current_state;
        }

        vTaskDelay(pdMS_TO_TICKS(25));
    }
}
