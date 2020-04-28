#ifndef __DCC__
#define __DCC__

#include "cmsis_os.h"
#include "main.h"

#ifdef __cplusplus
extern "C" {
#endif

#define DCC_MAX_DATA_BYTES 5u

#define DCC_ZERO_BIT_HALF_TIME_MS (100u)
#define DCC_ONE_BIT_HALF_TIME_MS   (58u)
// == 1/(2*100)*1000000
#define DCC_ZERO_BIT_FREQ (5000u)
// == 1/(2*58)*1000000
#define DCC_ONE_BIT_FREQ  (8621u)

#define DCC_PACKET_PREAMBLE_LEN             (14)
#define DCC_PACKET_ADDRESS_START_BIT_LEN     (1)
#define DCC_PACKET_ADDRES_LEN                (8)
#define DCC_PACKET_DATA_START_BIT_LEN        (1)
#define DCC_PACKET_DATA_LEN                  (8)
#define DCC_PACKET_CRC_START_BIT_LEN         (1)
#define DCC_PACKET_CRC_LEN                   (8)
#define DCC_PACKET_END_BIT_LEN               (1)

/* Default repeat for DCC Packets */
#define DCC_PACKET_DEFAULT_REPEAT            (5)
/* DCC Packet count permanent */
#define DCC_PACKET_PERMANENT                (-1)

#define DCC_PACKET_SPEED_128         (0x3F)
#define DCC_PACKET_SPEED_128_DIR_BIT    (7)

#define DCC_MAX_STREAM_BITS  77 // = 14+1+(8+1)+(8+1)*5+8+1 =13+63  = 78
#define DCC_MAX_STREAM_BYTES 10 // = 78/8+1

#define MAX_NODES  120
#define FINAL_NODE 119 // MAX_NODES-1)

#define QUEUE_OK             (0)
#define QUEUE_ERR_FULL      (-1)
#define QUEUE_ERR_EMPTY     (-2)
#define QUEUE_ERR_NOT_FOUND (-3)

#ifndef NULL
#define NULL 0
#endif

typedef enum {
  DCC_ZERO = DCC_ZERO_BIT_FREQ,
  DCC_ONE = DCC_ONE_BIT_FREQ,
} DCC_Bit;

typedef enum {
  DCC_PACKET_PREAMBLE = 0,
  DCC_PACKET_ADDRESS_START,
  DCC_PACKET_ADDRESS,
  DCC_PACKET_DATA_START,
  DCC_PACKET_DATA,
  DCC_PACKET_CRC_START,
  DCC_PACKET_CRC,
  DCC_PACKET_END
} DCC_Packet_State;

typedef struct DCC_Packet {
  uint8_t  data_len;
  int      count;
  uint16_t address;
  uint8_t  data[DCC_MAX_DATA_BYTES];
  uint8_t  crc;
} DCC_Packet;

typedef struct DCC_Stream {
  int nbits;
  unsigned char data[DCC_MAX_STREAM_BYTES];
} DCC_Stream;

void DCC_Packet_adjust_crc(DCC_Packet *p);
void DCC_Packet_set_address(DCC_Packet *p, unsigned char addr);
void DCC_Packet_set_speed(DCC_Packet *p, unsigned char speed, unsigned char direction);
void DCC_Packet_to_DCC_Stream(DCC_Packet *packet, DCC_Stream *stream);

//extern const DCC_Packet DCC_Packet_Idle;
//extern const DCC_Packet DCC_Packet_Reset;
//extern const DCC_Packet DCC_Packet_Stop;
#define DCC_PACKET_IDLE  (DCC_Packet) {.data_len = 1, .count = -1, .address = 0xFF, .data = {0x00, 0x00, 0x00, 0x00, 0x00}, .crc = 0xFF}
#define DCC_PACKET_RESET {1,  0, 0x00, {0x00, 0x00, 0x00, 0x00, 0x00}, 0x00}
#define DCC_PACKET_STOP  {1,  0, 0x00, {0x00, 0x00, 0x00, 0x00, 0x00}, 0x00}

typedef struct DCC_Packet_Pump {
    DCC_Bit next_bit;
    DCC_Packet_State status;
    uint8_t bit;
    uint8_t data_count;
    osMessageQId queue;
    osPoolId pool;
    DCC_Packet *packet;
} DCC_Packet_Pump;

osStatus DCC_Packet_Pump_init(DCC_Packet_Pump *pump, osMessageQId mq_id);
unsigned int DCC_Packet_Pump_next(DCC_Packet_Pump *pump);

// void dcc_pretty_print(DCC_Packet packet, const char *string);

#ifdef __cplusplus
}
#endif

#endif // __DCC__
