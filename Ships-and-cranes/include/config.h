#ifndef CONFIG_H
#define CONFIG_H

// Pátio
#define YARD_SIDE_LEN              16
#define MAX_YARD_CAPACITY          (YARD_SIDE_LEN * YARD_SIDE_LEN) // 256

// Filas
#define MAX_TRUCKS_QUEUE           50
#define MAX_SHIPS_QUEUE            10

// Lotes de Escoamento
#define NORMAL_TRUCK_BATCH         5
#define CRITICAL_TRUCK_BATCH       10 // Dobra o escoamento no sufoco

// Thresholds para lógica de Backpressure
#define YARD_WARNING_THRESHOLD     128 // 50% de 256
#define YARD_CRITICAL_THRESHOLD    205 // ~80% de 256

// Tempos de Processamento por Tipo (em milissegundos)
#define TIME_CONTAINER_STANDARD_MS 1000
#define TIME_CONTAINER_REEFER_MS   2500
#define TIME_CONTAINER_HAZMAT_MS   4000

// Configurações de Hardware (Pinos do ESP32)
#define LED_GREEN_PIN              18
#define LED_YELLOW_PIN             19
#define LED_RED_PIN                21

#define BTN_BATCH_YARD_PIN         4
#define BTN_SHIP_TRUCK_PIN         5
#define BTN_YARD_TRUCK_PIN         12

// Prioridades das Tasks FreeRTOS
#define PRIO_TASK_LOGGER           1
#define PRIO_TASK_DASHBOARD        1
#define PRIO_TASK_TRUCKS           2
#define PRIO_TASK_GENERATOR        2
#define PRIO_TASK_CRANES           3

#endif