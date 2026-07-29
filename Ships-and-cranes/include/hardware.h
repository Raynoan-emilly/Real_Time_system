#ifndef HARDWARE_H
#define HARDWARE_H

#include <stdint.h>
#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

void hardware_init(void);
void hardware_update_status_leds(
    uint16_t ships_waiting,
    uint16_t yard_count,
    SystemMode_t mode
);

#ifdef __cplusplus
}
#endif

#endif
