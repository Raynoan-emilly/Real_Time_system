#include "yard.h"
#include "config.h"

static uint8_t yard_grid[YARD_SIDE_LEN][YARD_SIDE_LEN];
static uint16_t total_yard_containers = 0;

uint8_t yard_add_container(ContainerType_t type) {
    if (total_yard_containers >= MAX_YARD_CAPACITY) {
        return 0; // Pátio cheio (256/256)
    }
    
    // Mapeamento bidimensional via deslocamento de bits (Aritmética de Potência de 2)
    // Exemplo: slot 18 -> Linha (18 >> 4 = 1), Coluna (18 & 0x0F = 2)
    uint8_t row = total_yard_containers >> 4; // Equivalente a dividir por 16
    uint8_t col = total_yard_containers & 0x0F; // Equivalente a resto por 16

    yard_grid[row][col] = (uint8_t)type + 1;
    total_yard_containers++;
    
    return 1;
}

uint8_t yard_remove_container(void) {
    if (total_yard_containers == 0) return 0;
    
    total_yard_containers--;
    uint8_t row = total_yard_containers >> 4;
    uint8_t col = total_yard_containers & 0x0F;
    
    yard_grid[row][col] = 0; // Slot liberado
    return 1;
}

uint16_t yard_get_count(void) {
    return total_yard_containers;
}