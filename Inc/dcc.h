#ifndef __DCC__
#define __DCC__

#include "cmsis_os.h"
#include "main.h"

#ifdef __cplusplus
extern "C" {
#endif

#define DCC_MAX_DATA_BYTES 5u

#define DCC_ONE  1ul
#define DCC_ZERO 0ul

#define DCC_ZERO_BIT_HALF_TIME_MS (100u)
#define DCC_ONE_BIT_HALF_TIME_MS   (58u)
// == 1/(2*100)*1000000
#define DCC_ZERO_BIT_FREQ (5000u)
// == 1/(2*58)*1000000
#define DCC_ONE_BIT_FREQ  (8621u)

#define DCC_PACKET_PREAMBLE_LEN             (16)
#define DCC_PACKET_ADDRESS_START_BIT_LEN     (1)
#define DCC_PACKET_ADDRES_LEN                (8)
#define DCC_PACKET_DATA_START_BIT_LEN        (1)
#define DCC_PACKET_DATA_LEN                  (8)
#define DCC_PACKET_CRC_START_BIT_LEN         (1)
#define DCC_PACKET_CRC_LEN                   (8)
#define DCC_PACKET_END_BIT_LEN               (1)

#define DCC_ADDRESS_MAX                 (16127u) // == 0x3EFF == 0011 1110 1111 1111, the first (high) packet address byte
                                                 // begins with '11' and can not be all '1' because this will clash with broadcast.
#define DCC_SHORT_ADDRESS_MAX             (127u)

/* Default repeat for DCC Packets */
#define DCC_PACKET_DEFAULT_REPEAT            (5)
/* DCC Packet count permanent */
#define DCC_PACKET_PERMANENT                (-1)
#define DCC_PACKET_SPEED_128              (0x3F)
#define DCC_PACKET_SPEED_128_DIR_BIT         (7)

#define DCC_PACKET_SPEED_28_START         0x40u // == 0b01000000
#define DCC_PACKET_SPEED_28_DIRECTION_BIT 0x20u // == 0b00100000
#define DCC_PACKET_SPEED_28_FIRST_BIT     0x10u // == 0b00010000
  

#define DCC_MAX_STREAM_BITS  80 // = 16+1+(8+1)+(8+1)*5+8+1 =13+63  = 80
#define DCC_MAX_STREAM_BYTES 10 // = 80/8

#ifndef NULL
#define NULL 0
#endif

//typedef enum {
//  DCC_ZERO = DCC_ZERO_BIT_FREQ,
//  DCC_ONE = DCC_ONE_BIT_FREQ,
//} DCC_Bit;

typedef enum {
  DCC_PACKET_PREAMBLE = 0,
  DCC_PACKET_ADDRESS_START,
  DCC_PACKET_ADDRESS,
  DCC_PACKET_ADDRESS_LOW_START,
  DCC_PACKET_ADDRESS_LOW,
  DCC_PACKET_DATA_START,
  DCC_PACKET_DATA,
  DCC_PACKET_CRC_START,
  DCC_PACKET_CRC,
  DCC_PACKET_END
} DCC_Packet_State;

typedef struct DCC_Packet {
  uint8_t  data_len;
  int      count;
  union {
    uint16_t address;
    struct {
      uint8_t address_low;
      uint8_t address_high;
    };
  };
  uint8_t  data[DCC_MAX_DATA_BYTES];
  uint8_t  crc;
} DCC_Packet;

/*
typedef struct DCC_Stream {
  int nbits;
  unsigned char data[DCC_MAX_STREAM_BYTES];
} DCC_Stream;
*/

void DCC_Packet_adjust_crc(DCC_Packet *p);
void DCC_Packet_set_address(DCC_Packet *p, uint16_t addr);
void DCC_Packet_set_speed(DCC_Packet *p, uint8_t speed, uint8_t direction);
// void DCC_Packet_to_DCC_Stream(DCC_Packet *packet, DCC_Stream *stream);

//extern const DCC_Packet DCC_Packet_Idle;
//extern const DCC_Packet DCC_Packet_Reset;
//extern const DCC_Packet DCC_Packet_Stop;
#define DCC_PACKET_IDLE  (DCC_Packet) {.data_len = 1, .count = -1, .address = 0x00FF, .data = {0x00, 0x00, 0x00, 0x00, 0x00}, .crc = 0xFF}
#define DCC_PACKET_RESET (DCC_Packet) {.data_len = 1, .count =  5, .address = 0x0000, .data = {0x00, 0x00, 0x00, 0x00, 0x00}, .crc = 0x00}
#define DCC_PACKET_STOP  (DCC_Packet) {.data_len = 1, .count =  5, .address = 0x0000, .data = {0x41, 0x00, 0x00, 0x00, 0x00}, .crc = 0x41}

typedef struct DCC_Packet_Pump {
    DCC_Packet_State status;
    uint8_t bit;
    uint8_t data_count;
    osMessageQId queue;
    osPoolId pool;
    DCC_Packet *packet;
} DCC_Packet_Pump;

osStatus DCC_Packet_Pump_init(DCC_Packet_Pump *pump, osMessageQId mq_id);
unsigned long DCC_Packet_Pump_next(DCC_Packet_Pump *pump);

// void dcc_pretty_print(DCC_Packet packet, const char *string);

#ifdef __cplusplus
}
#endif

#endif // __DCC__
