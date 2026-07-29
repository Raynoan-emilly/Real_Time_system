#ifndef TYPES_H
#define TYPES_H

#include <stdint.h>

typedef enum {
    CONTAINER_STANDARD = 0,
    CONTAINER_REEFER,
    CONTAINER_HAZMAT
} ContainerType_t;

typedef struct {
    uint32_t id;
    uint16_t total_containers;
    uint16_t remaining_containers;
    uint16_t count_standard;
    uint16_t count_reefer;
    uint16_t count_hazmat;
} Ship_t;

typedef struct {
    uint32_t id;
    uint8_t is_loaded;
} Truck_t;

typedef enum {
    CMD_TRIGGER_STRESS = 1, // Botão 1: Induz estresse (gargalo de caminhões + pausa guindastes)
    CMD_RELIEVE_STRESS,     // Botão 2: Alivia estresse (liga 2ª fila de escoamento + 10 guindastes)
    CMD_RESET_NORMAL        // Botão 3: Retorna ao modo normal de cruzeiro
} CraneCommand_t;

#endif