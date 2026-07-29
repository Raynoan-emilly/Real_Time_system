#ifndef TYPES_H
#define TYPES_H

#include <stdint.h>
#include <freertos/FreeRTOS.h>

typedef enum {
    CONTAINER_STANDARD = 0,
    CONTAINER_REEFER,
    CONTAINER_HAZMAT
} ContainerType_t;

typedef enum {
    SHIP_PRIORITY_LOW = 0,
    SHIP_PRIORITY_NORMAL,
    SHIP_PRIORITY_HIGH
} ShipPriority_t;

typedef enum {
    SYSTEM_MODE_NORMAL = 0,
    SYSTEM_MODE_STRESS,
    SYSTEM_MODE_RELIEF
} SystemMode_t;

typedef struct {
    uint32_t id;
    uint16_t total_containers;
    uint16_t remaining_containers;
    uint16_t count_standard;
    uint16_t count_reefer;
    uint16_t count_hazmat;
    ShipPriority_t priority;
    TickType_t arrival_tick;
} Ship_t;

typedef struct {
    uint32_t id;
    uint8_t is_loaded;
} Truck_t;

typedef enum {
    CMD_TRIGGER_STRESS = 1,
    CMD_RELIEVE_STRESS,
    CMD_RESET_NORMAL
} CraneCommand_t;

#endif
