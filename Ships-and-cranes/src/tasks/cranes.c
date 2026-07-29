#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <freertos/semphr.h>
#include <stdlib.h>

#include "system_handler.h"
#include "types.h"
#include "yard.h"
#include "config.h"

extern QueueHandle_t xQueueYardTrucks;

void vTaskCranes(void *pvParameters) {
    CraneCommand_t cmd;
    Ship_t *currentShip = NULL;
    Truck_t currentTruck;

    uint8_t active_cranes = 10;
    uint8_t worker_capacity = 8;
    uint32_t cycle_delay_ms = 300; // Tempo normal de ciclo

    for (;;) {
        // -------------------------------------------------------------
        // 1. TRATAMENTO DOS BOTÕES
        // -------------------------------------------------------------
        if (xQueueReceive(xQueueCraneCommands, &cmd, 0) == pdTRUE) {
            switch (cmd) {
                case CMD_TRIGGER_STRESS: // BOTÃO 1
                    active_cranes = 1;        // Apenas 1 guindaste operando
                    worker_capacity = 0;      // GREVE NO PÁTIO: 0 remoções!
                    cycle_delay_ms = 1200;    // Operação extremamente lenta
                    log_msg("=================================================");
                    log_msg("[ESTRESSE TOTAL!] Greve de Caminhões e 1 Guindaste!");
                    log_msg("=================================================");
                    break;

                case CMD_RELIEVE_STRESS: // BOTÃO 2
                    active_cranes = 10;
                    worker_capacity = 30;     // MUTIRÃO MONSTRO: 30 cargas por ciclo
                    cycle_delay_ms = 200;     // Operação ultra-rápida
                    log_msg("=================================================");
                    log_msg("[MUTIRÃO DE ALÍVIO!] Operação em Capacidade Máxima!");
                    log_msg("=================================================");

                    if (xQueueYardTrucks != NULL) {
                        for (int i = 0; i < 50; i++) {
                            Truck_t emergency_truck = { .id = rand() % 9000 + 1000, .is_loaded = 0 };
                            xQueueSend(xQueueYardTrucks, &emergency_truck, 0);
                        }
                    }
                    break;

                case CMD_RESET_NORMAL: // BOTÃO 3
                    active_cranes = 10;
                    worker_capacity = 8;
                    cycle_delay_ms = 300;
                    log_msg("[MODO CRUZEIRO] Retornado ao balanço normal.");
                    break;
            }
        }

        // -------------------------------------------------------------
        // 2. DRENAGEM DO PÁTIO PELA EQUIPE DE SOLO
        // -------------------------------------------------------------
        if (worker_capacity > 0 && xSemaphoreTake(xYardMutex, 0) == pdTRUE) {
            uint16_t yard_count = yard_get_count();

            if (yard_count > 0) {
                uint8_t moved_count = 0;

                for (uint8_t i = 0; i < worker_capacity; i++) {
                    if (yard_get_count() > 0) {
                        if (xQueueYardTrucks != NULL && xQueueReceive(xQueueYardTrucks, &currentTruck, 0) == pdTRUE) {
                            yard_remove_container();
                            moved_count++;
                        } else if (worker_capacity > 15) { 
                            // No modo Mutirão (Alívio), se faltar caminhão na fila, injeta e drena direto!
                            yard_remove_container();
                            moved_count++;
                        } else {
                            break;
                        }
                    }
                }

                if (moved_count > 0) {
                    log_msg("[PÁTIO -> CAMINHÕES] Drenados %u contêineres. (Saldo Pátio: %u/256)", 
                            moved_count, yard_get_count());
                }
            }
            xSemaphoreGive(xYardMutex);
        }

        // -------------------------------------------------------------
        // 3. ATRACAMENTO NAS DOCAS
        // -------------------------------------------------------------
        if (uxQueueMessagesWaiting(xQueueShipsArrival) > 0 && uxSemaphoreGetCount(xBerthSemaphore) > 0) {
            if (xQueuePeek(xQueueShipsArrival, &currentShip, 0) == pdTRUE) {
                if (xSemaphoreTake(xBerthSemaphore, 0) == pdTRUE) {
                    // Doca ocupada
                }
            }
        }

        // -------------------------------------------------------------
        // 4. DESCARREGAMENTO DO NAVIO ATRACADO
        // -------------------------------------------------------------
        if (xQueuePeek(xQueueShipsArrival, &currentShip, 0) == pdTRUE) {
            
            for (uint8_t i = 0; i < active_cranes; i++) {
                if (currentShip->remaining_containers > 0) {
                    
                    ContainerType_t type = CONTAINER_STANDARD;
                    if (currentShip->count_hazmat > 0) {
                        type = CONTAINER_HAZMAT;
                        currentShip->count_hazmat--;
                    } else if (currentShip->count_reefer > 0) {
                        type = CONTAINER_REEFER;
                        currentShip->count_reefer--;
                    } else if (currentShip->count_standard > 0) {
                        currentShip->count_standard--;
                    }

                    // Tenta mandar direto para o pátio (ou caminhão se houver)
                    if (xSemaphoreTake(xYardMutex, 0) == pdTRUE) {
                        if (yard_add_container(type)) {
                            currentShip->remaining_containers--;
                        } else {
                            log_msg("[ALERTA CRÍTICO] Pátio 256/256 PARALISADO! Sem espaço!");
                            xSemaphoreGive(xYardMutex);
                            break;
                        }
                        xSemaphoreGive(xYardMutex);
                    }
                }
            }

            if (currentShip->remaining_containers == 0) {
                xQueueReceive(xQueueShipsArrival, &currentShip, 0);
                xSemaphoreGive(xBerthSemaphore);
                g_total_ships_processed++;

                log_msg("----------------------------------------------------------------");
                log_msg("[CAIS] NAVIO #%u FINALIZADO E ZARPOU! (Atendidos: %u)", 
                        currentShip->id, g_total_ships_processed);
                log_msg("----------------------------------------------------------------");
                free(currentShip);
            }
        }

        vTaskDelay(pdMS_TO_TICKS(cycle_delay_ms)); // Tempo dinâmico do ciclo
    }
}