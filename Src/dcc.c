#include "dcc.h"
#include "bits.h"
#include "cmsis_os.h"

#ifndef NULL
#define NULL 0
#endif

//const DCC_Packet DCC_Packet_Idle  = {1, -1, 0xFF, {0x00, 0x00, 0x00, 0x00, 0x00}, 0xFF};
// TEST PATTERN of alternin 1 and 0 in adress, data and crc
// const DCC_Packet DCC_Packet_Idle  = {1, -1, 0x50, {0x05, 0x00, 0x00, 0x00, 0x00}, 0x55};
//const DCC_Packet DCC_Packet_Reset = {1,  0, 0x00, {0x00, 0x00, 0x00, 0x00, 0x00}, 0x00};
//const DCC_Packet DCC_Packet_Stop  = {1,  0, 0x00, {0x40, 0x00, 0x00, 0x00, 0x00}, 0x00};


void inc_index(int *index) {
  *index += 1;
  *index %= MAX_NODES;
}

void dec_index(int *index) {
  *index -= 1;
  *index %= MAX_NODES;
}


void DCC_Packet_adjust_crc(DCC_Packet *p) {
  p->crc = p->address;
  for (int i = 0; i < p->data_len; i++) {
	  p->crc ^= p->data[i];
  }
}

void DCC_Packet_set_address(DCC_Packet *p, unsigned char addr) {
  p->address = addr;
  DCC_Packet_adjust_crc(p);
}

void DCC_Packet_set_speed(DCC_Packet *p, unsigned char speed, unsigned char dir) {
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

osStatus_t DCC_Packet_Pump_init(DCC_Packet_Pump *pump, osMessageQueueId_t mq_id) {
  osStatus_t ost = osErrorNoMemory;
  pump->status = DCC_PACKET_PREAMBLE;
  pump->bit = 0;
  pump->data_count = 0;
  pump->queue = mq_id;
  pump->packet = pvPortMalloc(sizeof(DCC_Packet));
  if(NULL!=pump->packet)
	  ost = osOK;
  if(ost != osOK)
	  assert_failed((uint8_t *)__FILE__, __LINE__);
  return ost;
}

unsigned int DCC_Packet_Pump_next(DCC_Packet_Pump *pump) {
  unsigned int emit = DCC_ZERO;
  osStatus_t status;
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
        pump->bit = 0;
      }
      break;
    case DCC_PACKET_DATA_START:
      emit = DCC_ZERO_BIT_FREQ;
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
      emit = DCC_ZERO_BIT_FREQ;
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
      emit = DCC_ONE_BIT_FREQ;
      pump->status = DCC_PACKET_PREAMBLE;
      pump->bit = 0;
      pump->data_count = 0;
      if (pump->packet->count > 0) {
        pump->packet->count--;
      }
      // Grab next packet.
      if (pump->packet->count == 0) {
    	  //DCC_Packet_Queue_delete(pump->queue, packet);
      }
      else {
        status = osMessageQueuePut(pump->queue, pump->packet, 0U, 0U);
        if(status != osOK)
      	  assert_failed((uint8_t *)__FILE__, __LINE__);
      }
      status = osMessageQueueGet(pump->queue, pump->packet, 0U, 0U);
      if(status != osOK) {
    	  if (0 == osMessageQueueGetCount(pump->queue))
    		  assert_failed((uint8_t *)__FILE__, __LINE__);
    	  // SHOW MUST GO ON
    	  // pump->packet = &DCC_Packet_Idle;
      }
      break;
  }
  return emit;
}
