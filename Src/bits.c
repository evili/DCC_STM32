#include "bits.h"

const uint8_t bits[8] = {BIT_0, BIT_1, BIT_2, BIT_3,
                         BIT_4, BIT_5, BIT_6, BIT_7};

const uint8_t zero = 0x00u;
const uint8_t one  = 0x01u;

uint8_t __READ_BIT(uint8_t byte, uint8_t bit) {
  return ((byte & bits[bit]) != zero);
}

void __SET_BIT(uint8_t *byte, uint8_t bit) {
  *byte |= bits[bit]; 
}

void __UNSET_BIT(uint8_t *byte, uint8_t bit) {
  *byte &= ~bits[bit];
}
