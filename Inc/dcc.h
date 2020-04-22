#ifndef __DCC__
#define __DCC__

#ifdef __cplusplus
extern "C" {
#endif


#define DCC_MAX_DATA_BYTES 3u
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
  int data_len;
  unsigned char address;
  unsigned char data[DCC_MAX_DATA_BYTES];
  unsigned char crc;
} DCC_Packet;

void DCC_Packet_adjust_crc();
void DCC_Packet_set_address(unsigned char addr);
void DCC_Packet_set_speed(unsigned char speed, unsigned char direction);

const DCC_Packet DCC_Packet_Idle  = {1, 0xFF, {0x00, 0x00, 0x00}, 0xFF};
const DCC_Packet DCC_Packet_Reset = {1, 0x00, {0x00, 0x00, 0x00}, 0x00};
const DCC_Packet DCC_Packet_Stop  = {1, 0x00, {0x40, 0x00, 0x00}, 0x00};

#define DCC_QUEUE_MAX 40

typedef struct DCC_Packet_Queue {
    unsigned char length;
    DCC_Packet_Queue *first;
    DCC_Packet_Queue *next;
    const DCC_Packet *packet;
    char count;
} DCC_Packet_Queue;

void DCC_Packet_Queue_init(DCC_Packet_Queue *queue);
int DCC_Packet_Queue_Add_DCC_Packet(const DCC_Packet *packet, short count);


typedef struct DCC_Packet_Pump {
    DCC_Bit next_bit;
    DCC_Packet_State status;
    unsigned char      bit;
    unsigned char data_count;
    DCC_Packet_Queue *queue;
} DCC_Packet_Pump;

void DCC_Packet_Pump_Emit(DCC_Bit);
void DCC_Packet_Pump_init(DCC_Packet_Pump *pump, DCC_Packet_Queue *queue);
unsigned int DCC_Packet_Pump_next();

void dcc_pretty_print(DCC_Packet packet, const char *string);

#ifdef __cplusplus
}
#endif

#endif // __DCC__
