#include "dcc.h"
#include "bits.h"
#include "cmsis_os.h"
#include "printf-stdarg.h"

#ifndef NULL
#define NULL 0
#endif

#define DCC_ERROR(n, s) printf("\nE: %s: %u\n", (s), (n));

//const DCC_Packet DCC_Packet_Idle  = {1, -1, 0xFF, {0x00, 0x00, 0x00, 0x00, 0x00}, 0xFF};
// TEST PATTERN of alternin 1 and 0 in adress, data and crc
// const DCC_Packet DCC_Packet_Idle  = {1, -1, 0x50, {0x05, 0x00, 0x00, 0x00, 0x00}, 0x55};
//const DCC_Packet DCC_Packet_Reset = {1,  0, 0x00, {0x00, 0x00, 0x00, 0x00, 0x00}, 0x00};
//const DCC_Packet DCC_Packet_Stop  = {1,  0, 0x00, {0x40, 0x00, 0x00, 0x00, 0x00}, 0x00};


void DCC_Packet_adjust_crc(DCC_Packet *p) {
  p->crc = p->address;
  for (int i = 0; i < p->data_len; i++) {
	  p->crc ^= p->data[i];
  }
}

void DCC_Packet_set_address(DCC_Packet *p, uint16_t addr) {
  p->address = addr;
  if(p->address > 127) {
  }
  DCC_Packet_adjust_crc(p);
}

void DCC_Packet_set_speed(DCC_Packet *p, uint8_t speed, uint8_t dir) {
  p->data_len = 2;
  p->data[0] = DCC_PACKET_SPEED_128;
  unsigned char adjust_speed = (speed > 127) ? 127 : speed;
  p->data[1] = ((dir & 0x01) << DCC_PACKET_SPEED_128_DIR_BIT) | adjust_speed;
  DCC_Packet_adjust_crc(p);
}

void DCC_Packet_to_DCC_Stream(DCC_Packet *packet, DCC_Stream *stream) {
  stream->nbits = DCC_PACKET_PREAMBLE_LEN
	+  DCC_PACKET_ADDRESS_START_BIT_LEN + DCC_PACKET_ADDRES_LEN
    + (DCC_PACKET_DATA_START_BIT_LEN + DCC_PACKET_ADDRES_LEN ) * packet->data_len
    +  DCC_PACKET_CRC_START_BIT_LEN + DCC_PACKET_CRC_LEN
    +  DCC_PACKET_END_BIT_LEN;
  stream->data[0] = 0xFF; // 8 - Preamble Bits
  stream->data[1] = 0xFC + __READ_BIT(packet->address, 7);
  stream->data[2] = (packet->address << 1); // 6 bit Address + Zero bit
  
  uint8_t  packet_byte = 0, packet_bit = 7, data_byte = 3, data_bit = 7, stream_bit = 23;
  for(int i=stream_bit; i < stream->nbits; i++) {
    if(__READ_BIT(packet->data[packet_byte], packet_bit)) {
      __SET_BIT(&stream->data[data_byte], data_bit);
    }
    else{
      __UNSET_BIT(&stream->data[data_byte], data_bit);
    }
    if(packet_bit == 0) {
      packet_byte++;
      packet_bit = 7;
    }
    else {
      packet_bit--;
    }
    if(data_bit == 0) {
      data_byte++;
      data_bit = 7;
    }
    else {
      data_bit--;
    }
  }
//  assert(packet_byte == packet->data_len);
//  assert(packet_bit == 7);
//  assert(data_byte == (stream->nbits/8));
//  assert(data_bit == (stream->nbits % 8));
}

osStatus DCC_Packet_Pump_init(DCC_Packet_Pump *pump, osMessageQId mq_id) {
  osStatus ost = osErrorNoMemory;
  pump->status = DCC_PACKET_PREAMBLE;
  pump->bit = 0;
  pump->data_count = 0;
  pump->queue = mq_id;
  pump->packet = (DCC_Packet *) pvPortMalloc(sizeof(DCC_Packet));
  if(NULL != pump->packet) {
	  ost = osOK;
	  *pump->packet = DCC_PACKET_IDLE;
	  //printf("\nFirst packet OK: %u\n", ost);
  }
//  if(ost != osOK)
//	  printf("\nERROR: No memory: %u\n", ost);
  return ost;
}

unsigned long DCC_Packet_Pump_next(DCC_Packet_Pump *pump) {
  unsigned int emit;
  osStatus_t status;
  osEvent event;
  switch (pump->status) {
    case DCC_PACKET_PREAMBLE:
      emit = DCC_ONE;
      pump->bit++;
      if (pump->bit >= DCC_PACKET_PREAMBLE_LEN) {
    	  pump->status = DCC_PACKET_ADDRESS_START;
    	  pump->bit = 0;
      }
      break;
    case DCC_PACKET_ADDRESS_START:
      emit = DCC_ZERO;
      pump->status = DCC_PACKET_ADDRESS;
      pump->bit = 0;
      break;
    case DCC_PACKET_ADDRESS:
      emit = ((pump->packet->address >> (pump->bit)) & (0x01)) ? DCC_ONE_BIT_FREQ : DCC_ZERO_BIT_FREQ;
      pump->bit++;
      if (pump->bit >= DCC_PACKET_ADDRES_LEN) {
        pump->status = DCC_PACKET_DATA_START;
        pump->data_count = 0;
        pump->bit = 0;
      }
      break;
    case DCC_PACKET_DATA_START:
      emit = DCC_ZERO;
      pump->bit = 0;
      pump->status = DCC_PACKET_DATA;
      break;
    case DCC_PACKET_DATA:
      emit = ((pump->packet->data[pump->data_count] >> (pump->bit)) & (0x01)) ? DCC_ONE_BIT_FREQ : DCC_ZERO_BIT_FREQ;
      pump->bit++;
      if (pump->bit >= DCC_PACKET_DATA_LEN) {
        pump->bit = 0;
        pump->data_count++;
        pump->status = DCC_PACKET_DATA_START;
        if (pump->data_count >= pump->packet->data_len) {
          pump->status = DCC_PACKET_CRC_START;
          pump->data_count = 0;
        }
      }
      break;
    case DCC_PACKET_CRC_START:
      emit = DCC_ZERO;
      pump->status = DCC_PACKET_CRC;
      pump->bit = 0;
      break;
    case DCC_PACKET_CRC:
      emit = ((pump->packet->crc >> (pump->bit)) & (0x01)) ? DCC_ONE_BIT_FREQ : DCC_ZERO_BIT_FREQ;
      pump->bit++;
      if (pump->bit >= DCC_PACKET_CRC_LEN) {
        pump->status = DCC_PACKET_END;
        pump->bit = 0;
      }
      break;
    case DCC_PACKET_END:
      emit = DCC_ONE;
      pump->status = DCC_PACKET_PREAMBLE;
      pump->bit = 0;
      pump->data_count = 0;
      //printf("\n%s\n", "\nDCC_PACKET_END");
      if (pump->packet->count > 0) {
        pump->packet->count--;
      }
      // Grab next packet.
      if (pump->packet->count == 0) {
    	printf("%s\n", "Count is zero. Freeing");
    	vPortFree(pump->packet);
      }
      else {
    	printf("%s\n", "Returning packet to queue");
        status = osMessageQueuePut(pump->queue, (void *) pump->packet, 0U, 0U);
        if(status != osOK)
      	  DCC_ERROR(status, "Can't put on queue.");
      }
      printf("%s\n","Getting new packet.");
      status = osMessageQueueGet(pump->queue, (void *) pump->packet, 0U, 0U);
      if(event.status != osOK) {
    	  if (osErrorResource == status)
    		  DCC_ERROR(status, "Nothing to get from queue.");
    	  printf("%s\n", "No message in queue");
      }
      break;
  }
  return emit;
}
