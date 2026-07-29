#ifndef YARD_H
#define YARD_H

#include <stdint.h>
#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

uint8_t yard_add_container(ContainerType_t type);
uint8_t yard_remove_container(void);
uint16_t yard_get_count(void);

#ifdef __cplusplus
}
#endif

#endif // YARD_H