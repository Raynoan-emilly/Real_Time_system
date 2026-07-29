#ifndef CONFIG_H
#define CONFIG_H

// Estrutura física simulada
#define MAX_BERTHS                       3
#define YARD_SIDE_LEN                    16
#define MAX_YARD_CAPACITY                (YARD_SIDE_LEN * YARD_SIDE_LEN)

// Capacidades das filas
#define MAX_SHIPS_PER_PRIORITY_QUEUE     10
#define MAX_TRUCKS_QUEUE                 40
#define COMMAND_QUEUE_LENGTH             10
#define LOG_QUEUE_LENGTH                 60
#define LOG_MESSAGE_LENGTH               160

// Limites operacionais
#define YARD_WARNING_THRESHOLD           40
#define YARD_CRITICAL_THRESHOLD          80
#define YARD_RECOVERY_THRESHOLD          20

#define SHIPS_WARNING_THRESHOLD          3
#define SHIPS_CRITICAL_THRESHOLD         7
#define SHIPS_RECOVERY_THRESHOLD         1

// Geração de navios
#define SHIP_MIN_CONTAINERS              24
#define SHIP_MAX_CONTAINERS              60
#define SHIP_ARRIVAL_INTERVAL_MS         3000

// Envelhecimento de prioridade
#define PRIORITY_AGING_INTERVAL_MS       15000

// Geração de caminhões
#define TRUCK_INTERVAL_NORMAL_MS         250
#define TRUCK_INTERVAL_STRESS_MS         1200
#define TRUCK_INTERVAL_RELIEF_MS         80

// Recursos por modo
#define CRANES_NORMAL                    4
#define CRANES_STRESS                    1
#define CRANES_RELIEF                    8

#define YARD_MOVES_NORMAL                2
#define YARD_MOVES_STRESS                0
#define YARD_MOVES_RELIEF                8

#define CRANE_CYCLE_NORMAL_MS            250
#define CRANE_CYCLE_STRESS_MS            1000
#define CRANE_CYCLE_RELIEF_MS            120

// O teste de falha provocado pelo botão não fica permanente
#define FORCED_STRESS_DURATION_MS        20000

// Penalidade pequena para representar cargas mais delicadas
#define EXTRA_REEFER_DELAY_MS            40
#define EXTRA_HAZMAT_DELAY_MS            80

// Hardware
#define LED_GREEN_PIN                    18
#define LED_YELLOW_PIN                   19
#define LED_RED_PIN                      21

#define BTN_TRIGGER_STRESS_PIN           4
#define BTN_FORCE_RELIEF_PIN             5
#define BTN_RESET_NORMAL_PIN             12
#define BUTTON_DEBOUNCE_MS               180

// Prioridades das tarefas
#define PRIO_TASK_LOGGER                 1
#define PRIO_TASK_DASHBOARD              1
#define PRIO_TASK_TRUCKS                 2
#define PRIO_TASK_GENERATOR              2
#define PRIO_TASK_BUTTONS                2
#define PRIO_TASK_CRANES                 3

#endif
