#ifndef HARDWARE_H
#define HARDWARE_H

#include <stdint.h>
#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

void hardware_init(void);
void hardware_update_status_leds(uint8_t ships_waiting, uint16_t yard_count);
void send_button_command_from_isr(CraneCommand_t cmd);

#ifdef __cplusplus
}
#endif

#endif // HARDWARE_H