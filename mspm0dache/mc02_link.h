#ifndef MC02_LINK_H
#define MC02_LINK_H

#include <stdint.h>

void MC02Link_Init(void);
void MC02Link_Tick(void);
uint16_t MC02Link_Crc16(const uint8_t *data, uint16_t length);

#endif
