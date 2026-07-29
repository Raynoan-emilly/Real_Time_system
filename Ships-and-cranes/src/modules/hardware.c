#include <Arduino.h>
#include "hardware.h"
#include "system_handler.h"
#include "types.h"
#include "config.h"

void hardware_init(void) {
    pinMode(LED_GREEN_PIN, OUTPUT);
    pinMode(LED_YELLOW_PIN, OUTPUT);
    pinMode(LED_RED_PIN, OUTPUT);

    pinMode(BTN_BATCH_YARD_PIN, INPUT_PULLUP);
    pinMode(BTN_SHIP_TRUCK_PIN, INPUT_PULLUP);
    pinMode(BTN_YARD_TRUCK_PIN, INPUT_PULLUP);

    digitalWrite(LED_GREEN_PIN, LOW);
    digitalWrite(LED_YELLOW_PIN, LOW);
    digitalWrite(LED_RED_PIN, LOW);
}

void hardware_update_status_leds(uint8_t ships_waiting, uint16_t yard_count) {
    float yard_fill = ((float)yard_count / (float)MAX_YARD_CAPACITY) * 100.0f;

    // VERMELHO: Pátio >= 60% (153 itens) OR >= 3 Navios esperando no mar
    if (yard_fill >= 60.0f || ships_waiting >= 3) {
        digitalWrite(LED_GREEN_PIN, LOW);
        digitalWrite(LED_YELLOW_PIN, LOW);
        digitalWrite(LED_RED_PIN, HIGH);
    } 
    // AMARELO: Pátio >= 30% (76 itens) OR >= 1 Navio no mar
    else if (yard_fill >= 30.0f || ships_waiting >= 1) {
        digitalWrite(LED_GREEN_PIN, LOW);
        digitalWrite(LED_YELLOW_PIN, HIGH);
        digitalWrite(LED_RED_PIN, LOW);
    } 
    // VERDE: Pátio < 30% e 0 Navios no mar (Tranquilo)
    else {
        digitalWrite(LED_GREEN_PIN, HIGH);
        digitalWrite(LED_YELLOW_PIN, LOW);
        digitalWrite(LED_RED_PIN, LOW);
    }
}

void vTaskButtons(void *pvParameters) {
    uint8_t last_btn1 = HIGH;
    uint8_t last_btn2 = HIGH;
    uint8_t last_btn3 = HIGH;

    for (;;) {
        uint8_t btn1 = digitalRead(BTN_BATCH_YARD_PIN);
        uint8_t btn2 = digitalRead(BTN_SHIP_TRUCK_PIN);
        uint8_t btn3 = digitalRead(BTN_YARD_TRUCK_PIN);

        // BOTÃO 1: FORÇAR ESTRESSE (Pausa guindastes e simula gargalo)
        if (last_btn1 == HIGH && btn1 == LOW) {
            CraneCommand_t cmd = CMD_TRIGGER_STRESS;
            xQueueSend(xQueueCraneCommands, &cmd, 0);
            vTaskDelay(pdMS_TO_TICKS(250));
        }

        // BOTÃO 2: ALIVIAR ESTRESSE (Ativa 2ª fila de caminhões e 10 guindastes)
        if (last_btn2 == HIGH && btn2 == LOW) {
            CraneCommand_t cmd = CMD_RELIEVE_STRESS;
            xQueueSend(xQueueCraneCommands, &cmd, 0);
            vTaskDelay(pdMS_TO_TICKS(250));
        }

        // BOTÃO 3: RESET MODO NORMAL DE CRUZEIRO
        if (last_btn3 == HIGH && btn3 == LOW) {
            CraneCommand_t cmd = CMD_RESET_NORMAL;
            xQueueSend(xQueueCraneCommands, &cmd, 0);
            vTaskDelay(pdMS_TO_TICKS(250));
        }

        last_btn1 = btn1;
        last_btn2 = btn2;
        last_btn3 = btn3;

        vTaskDelay(pdMS_TO_TICKS(30));
    }
}