#include <freertos/FreeRTOS.h>
#include <stdbool.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <freertos/semphr.h>

#include "system_handler.h"
#include "types.h"
#include "yard.h"
#include "config.h"

typedef struct {
    uint8_t occupied;
    Ship_t ship;
    TickType_t berth_tick;
} Berth_t;

static Berth_t berths[MAX_BERTHS];

static const char *mode_name(SystemMode_t mode) {
    switch (mode) {
        case SYSTEM_MODE_STRESS:
            return "ESTRESSE";
        case SYSTEM_MODE_RELIEF:
            return "ALIVIO";
        case SYSTEM_MODE_NORMAL:
        default:
            return "NORMAL";
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

static uint8_t find_free_berth(void) {
    for (uint8_t i = 0; i < MAX_BERTHS; i++) {
        if (!berths[i].occupied) {
            return i;
        }
    }

    return MAX_BERTHS;
}

static BaseType_t receive_next_ship(Ship_t *ship) {
    if (xQueueReceive(xQueueShipsHigh, ship, 0) == pdTRUE) {
        return pdTRUE;
    }

    if (xQueueReceive(xQueueShipsNormal, ship, 0) == pdTRUE) {
        return pdTRUE;
    }

    return xQueueReceive(xQueueShipsLow, ship, 0);
}

static void promote_front_ship(
    QueueHandle_t source,
    QueueHandle_t destination,
    ShipPriority_t new_priority
) {
    Ship_t ship;

    if (xQueuePeek(source, &ship, 0) != pdTRUE) {
        return;
    }

    TickType_t waited =
        xTaskGetTickCount() -
        ship.arrival_tick;

    if (waited <
        pdMS_TO_TICKS(PRIORITY_AGING_INTERVAL_MS)) {
        return;
    }

    if (uxQueueSpacesAvailable(destination) == 0) {
        return;
    }

    if (xQueueReceive(source, &ship, 0) != pdTRUE) {
        return;
    }

    ship.priority = new_priority;

    if (xQueueSend(destination, &ship, 0) == pdPASS) {
        log_msg(
            "[PRIORIDADE] Navio #%u promovido para %s por tempo de espera.",
            ship.id,
            priority_name(new_priority)
        );
    } else {
        xQueueSend(source, &ship, 0);
    }
}

static void apply_priority_aging(void) {
    promote_front_ship(
        xQueueShipsNormal,
        xQueueShipsHigh,
        SHIP_PRIORITY_HIGH
    );

    promote_front_ship(
        xQueueShipsLow,
        xQueueShipsNormal,
        SHIP_PRIORITY_NORMAL
    );
}

static void berth_waiting_ships(void) {
    for (;;) {
        uint8_t berth_index = find_free_berth();

        if (berth_index >= MAX_BERTHS) {
            return;
        }

        if (system_get_waiting_ships() == 0) {
            return;
        }

        if (xSemaphoreTake(xBerthSemaphore, 0) != pdTRUE) {
            return;
        }

        Ship_t ship;

        if (receive_next_ship(&ship) != pdTRUE) {
            xSemaphoreGive(xBerthSemaphore);
            return;
        }

        berths[berth_index].occupied = 1;
        berths[berth_index].ship = ship;
        berths[berth_index].berth_tick =
            xTaskGetTickCount();

        TickType_t waiting_time =
            berths[berth_index].berth_tick -
            ship.arrival_tick;

        log_msg(
            "[DOCA %u] Navio #%u atracou. Prioridade %s. Espera: %lu ms.",
            berth_index + 1,
            ship.id,
            priority_name(ship.priority),
            (unsigned long)(
                waiting_time *
                portTICK_PERIOD_MS
            )
        );
    }
}

static ContainerType_t next_container_type(
    const Ship_t *ship
) {
    if (ship->count_hazmat > 0) {
        return CONTAINER_HAZMAT;
    }

    if (ship->count_reefer > 0) {
        return CONTAINER_REEFER;
    }

    return CONTAINER_STANDARD;
}

static void decrement_container(
    Ship_t *ship,
    ContainerType_t type
) {
    if (type == CONTAINER_HAZMAT &&
        ship->count_hazmat > 0) {
        ship->count_hazmat--;
    } else if (
        type == CONTAINER_REEFER &&
        ship->count_reefer > 0
    ) {
        ship->count_reefer--;
    } else if (ship->count_standard > 0) {
        ship->count_standard--;
    }

    if (ship->remaining_containers > 0) {
        ship->remaining_containers--;
    }
}

static uint16_t unload_ships(uint8_t active_cranes) {
    static uint8_t round_robin_index = 0;
    uint16_t extra_delay_ms = 0;
    uint8_t yard_full_reported = 0;

    for (uint8_t crane = 0;
         crane < active_cranes;
         crane++) {

        uint8_t selected = MAX_BERTHS;

        for (uint8_t scan = 0;
             scan < MAX_BERTHS;
             scan++) {

            uint8_t candidate =
                (round_robin_index + scan) %
                MAX_BERTHS;

            if (berths[candidate].occupied &&
                berths[candidate].ship
                    .remaining_containers > 0) {
                selected = candidate;
                break;
            }
        }

        if (selected >= MAX_BERTHS) {
            break;
        }

        round_robin_index =
            (selected + 1) % MAX_BERTHS;

        Ship_t *ship = &berths[selected].ship;
        ContainerType_t type =
            next_container_type(ship);

        Truck_t truck;
        uint8_t moved = 0;

        if (xQueueReceive(
                xQueueDirectTrucks,
                &truck,
                0
            ) == pdTRUE) {

            truck.is_loaded = 1;
            moved = 1;
            g_total_direct_dispatches++;
        } else if (yard_add_container(type)) {
            moved = 1;
        }

        if (!moved) {
            if (!yard_full_reported) {
                log_msg(
                    "[BLOQUEIO] Patio cheio e sem caminhao direto."
                );
                yard_full_reported = 1;
            }
            break;
        }

        decrement_container(ship, type);

        if (type == CONTAINER_HAZMAT) {
            extra_delay_ms = EXTRA_HAZMAT_DELAY_MS;
        } else if (
            type == CONTAINER_REEFER &&
            extra_delay_ms < EXTRA_REEFER_DELAY_MS
        ) {
            extra_delay_ms = EXTRA_REEFER_DELAY_MS;
        }
    }

    return extra_delay_ms;
}

static void drain_yard(uint8_t move_limit) {
    uint8_t moved = 0;

    for (uint8_t i = 0; i < move_limit; i++) {
        if (yard_get_count() == 0) {
            break;
        }

        Truck_t truck;

        if (xQueueReceive(
                xQueueYardTrucks,
                &truck,
                0
            ) != pdTRUE) {
            break;
        }

        if (yard_remove_container()) {
            truck.is_loaded = 1;
            moved++;
            g_total_yard_dispatches++;
        }
    }

    if (moved > 0) {
        log_msg(
            "[PATIO] %u conteineres enviados para caminhoes. Saldo: %u.",
            moved,
            yard_get_count()
        );
    }
}

static void finish_completed_ships(void) {
    for (uint8_t i = 0; i < MAX_BERTHS; i++) {
        if (!berths[i].occupied) {
            continue;
        }

        if (berths[i].ship.remaining_containers > 0) {
            continue;
        }

        TickType_t operation_time =
            xTaskGetTickCount() -
            berths[i].berth_tick;

        g_total_ships_processed++;

        log_msg(
            "[DOCA %u] Navio #%u finalizado em %lu ms. Total atendido: %lu.",
            i + 1,
            berths[i].ship.id,
            (unsigned long)(
                operation_time *
                portTICK_PERIOD_MS
            ),
            (unsigned long)g_total_ships_processed
        );

        berths[i].occupied = 0;
        xSemaphoreGive(xBerthSemaphore);
    }
}

static void set_mode(
    SystemMode_t mode,
    const char *reason
) {
    if (g_system_mode == mode) {
        return;
    }

    g_system_mode = mode;

    log_msg(
        "[CONTROLE] Modo %s ativado. Motivo: %s.",
        mode_name(mode),
        reason
    );
}

void vTaskCranes(void *pvParameters) {
    (void)pvParameters;

    CraneCommand_t command;
    TickType_t forced_stress_started = 0;
    uint8_t forced_stress_active = 0;

    for (;;) {
        while (xQueueReceive(
                   xQueueCraneCommands,
                   &command,
                   0
               ) == pdTRUE) {

            if (command == CMD_TRIGGER_STRESS) {
                forced_stress_active = 1;
                forced_stress_started =
                    xTaskGetTickCount();

                set_mode(
                    SYSTEM_MODE_STRESS,
                    "falha provocada pelo botao"
                );
            } else if (
                command == CMD_RELIEVE_STRESS
            ) {
                forced_stress_active = 0;

                set_mode(
                    SYSTEM_MODE_RELIEF,
                    "alivio solicitado pelo botao"
                );
            } else if (
                command == CMD_RESET_NORMAL
            ) {
                forced_stress_active = 0;

                set_mode(
                    SYSTEM_MODE_NORMAL,
                    "retorno manual"
                );
            }
        }

        uint16_t yard_count = yard_get_count();
        uint16_t waiting_ships =
            system_get_waiting_ships();

        if (forced_stress_active) {
            TickType_t elapsed =
                xTaskGetTickCount() -
                forced_stress_started;

            bool stress_timeout =
                elapsed >=
                pdMS_TO_TICKS(
                    FORCED_STRESS_DURATION_MS
                );

            bool safety_override =
                yard_count >=
                YARD_CRITICAL_THRESHOLD;

            if (stress_timeout || safety_override) {
                forced_stress_active = 0;

                set_mode(
                    SYSTEM_MODE_RELIEF,
                    safety_override
                        ? "limite critico do patio"
                        : "fim automatico do teste de estresse"
                );
            }
        } else if (
            g_system_mode == SYSTEM_MODE_NORMAL &&
            (
                yard_count >=
                    YARD_CRITICAL_THRESHOLD ||
                waiting_ships >=
                    SHIPS_CRITICAL_THRESHOLD
            )
        ) {
            set_mode(
                SYSTEM_MODE_RELIEF,
                "sobrecarga detectada automaticamente"
            );
        } else if (
            g_system_mode == SYSTEM_MODE_RELIEF &&
            yard_count <=
                YARD_RECOVERY_THRESHOLD &&
            waiting_ships <=
                SHIPS_RECOVERY_THRESHOLD
        ) {
            set_mode(
                SYSTEM_MODE_NORMAL,
                "sistema recuperado automaticamente"
            );
        }

        apply_priority_aging();
        berth_waiting_ships();

        uint8_t active_cranes = CRANES_NORMAL;
        uint8_t yard_moves = YARD_MOVES_NORMAL;
        uint32_t cycle_ms =
            CRANE_CYCLE_NORMAL_MS;

        if (g_system_mode == SYSTEM_MODE_STRESS) {
            active_cranes = CRANES_STRESS;
            yard_moves = YARD_MOVES_STRESS;
            cycle_ms = CRANE_CYCLE_STRESS_MS;
        } else if (
            g_system_mode == SYSTEM_MODE_RELIEF
        ) {
            active_cranes = CRANES_RELIEF;
            yard_moves = YARD_MOVES_RELIEF;
            cycle_ms = CRANE_CYCLE_RELIEF_MS;
        }

        drain_yard(yard_moves);

        uint16_t extra_delay =
            unload_ships(active_cranes);

        finish_completed_ships();

        vTaskDelay(
            pdMS_TO_TICKS(
                cycle_ms + extra_delay
            )
        );
    }
}
