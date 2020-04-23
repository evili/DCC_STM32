#ifndef __BITS_H__
#define __BITS_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define BIT_7 0x80
#define BIT_6 0x40
#define BIT_5 0x20
#define BIT_4 0x10
#define BIT_3 0x08
#define BIT_2 0x04
#define BIT_1 0x02
#define BIT_0 0x01

extern const uint8_t bits[8];

uint8_t __READ_BIT(uint8_t byte, uint8_t bit);
void __SET_BIT(uint8_t *byte, uint8_t bit);
void __UNSET_BIT(uint8_t *byte, uint8_t bit);

#ifdef __cplusplus
}
#endif


#endif // __BITS_H__
