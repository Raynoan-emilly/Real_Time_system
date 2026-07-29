#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <freertos/semphr.h>

#include "system_handler.h"
#include "hardware.h"
#include "yard.h"

void vTaskDashboard(void *pvParameters) {
    for (;;) {
        UBaseType_t ships_waiting_sea = uxQueueMessagesWaiting(xQueueShipsArrival);
        UBaseType_t free_berths = uxSemaphoreGetCount(xBerthSemaphore);
        uint8_t docks_occupied = 10 - free_berths;
        
        UBaseType_t trucks_waiting = uxQueueMessagesWaiting(xQueueTrucksArrival);
        uint16_t yard_count = yard_get_count();

        // Atualiza os LEDs com base na lotação do ancoradouro
        hardware_update_status_leds(ships_waiting_sea, 20);

        log_msg("\n>>> [PANORAMA DO PORTO] <<<");
        log_msg(" | Docas Ocupadas: %u/10  | Fila no Mar: %u navios", docks_occupied, ships_waiting_sea);
        log_msg(" | Ocupação Pátio: %u/256 | Caminhões Esperando: %u", yard_count, trucks_waiting);
        log_msg(" | Total de Navios Atendidos no Dia: %u\n", g_total_ships_processed);

        vTaskDelay(pdMS_TO_TICKS(3000));
    }
}