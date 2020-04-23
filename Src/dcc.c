#include "dcc.h"
#include "bits.h"

#ifndef NULL
#define NULL 0
#endif

const DCC_Packet DCC_Packet_Idle  = {1, -1, 0xFF, {0x00, 0x00, 0x00, 0x00, 0x00}, 0xFF};
// TEST PATTERN of alternin 1 and 0 in adress, data and crc
// const DCC_Packet DCC_Packet_Idle  = {1, -1, 0x50, {0x05, 0x00, 0x00, 0x00, 0x00}, 0x55};
const DCC_Packet DCC_Packet_Reset = {1,  0, 0x00, {0x00, 0x00, 0x00, 0x00, 0x00}, 0x00};
const DCC_Packet DCC_Packet_Stop  = {1,  0, 0x00, {0x40, 0x00, 0x00, 0x00, 0x00}, 0x00};


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


void DCC_Packet_Queue_init(DCC_Packet_Queue *q) {
	q->front = -1;
	q->rear  = -1;
	q->pivot = -1;
	DCC_Packet_Queue_Add_DCC_Packet(q, (DCC_Packet *) &DCC_Packet_Idle);
}

int  DCC_Packet_Queue_Add_DCC_Packet(DCC_Packet_Queue *q, DCC_Packet *packet) {

	  if((q->front == 0 && q->rear == FINAL_NODE) ||
	     (q->front == (q->rear+1))) {
	    return QUEUE_ERR_FULL;
	  }

	  if(q->front == -1) {
	    q->front = 0;
	    q->rear  = 0;
	    q->pivot = 0;
	  }
	  else {
	    inc_index(&q->rear);
	  }

	  q->list[q->rear] = packet;

	  return QUEUE_OK;
}

int DCC_Packet_Queue_delete(DCC_Packet_Queue *q, DCC_Packet *p) {
  if(q->front == -1) {
    return QUEUE_ERR_EMPTY;
  }

  if(q->front == q->rear) {
    if(p == q->list[q->front]) {
      q->list[q->front] = NULL;
      q->front = -1;
      q->rear  = -1;
      q->pivot = -1;
      return QUEUE_OK;
    }
    else {
      return QUEUE_ERR_NOT_FOUND;
    }
  }

  int found = -1;
  for(int i = q->front; i != q->rear; i = (i+1) % MAX_NODES) {
    if (p == q->list[i])
      found = 1;
  }
  if(found > 0) {
    for(int src = (found-1) % MAX_NODES, dst = found;
	src != q->front; dec_index(&dst), dec_index(&src)) {
      q->list[dst] = q->list[src];
    }
    q->list[q->front] = NULL;
    if (q->pivot == q->front) {
      inc_index(&q->pivot);
    }
    inc_index(&q->front);
    return QUEUE_OK;
  }
  else {
    return QUEUE_ERR_NOT_FOUND;
  }
}

DCC_Packet *DCC_Packet_Queue_next(DCC_Packet_Queue *q) {
	  DCC_Packet *packet = NULL;
	  if (q->pivot != -1) {
	    packet = q->list[q->pivot];
	    q->pivot = (q->pivot == q->rear) ? q->front : q->pivot+1;
	    q->pivot %= MAX_NODES;
	  }
	  return packet;
}

DCC_Packet *DCC_Packet_Queue_peek(DCC_Packet_Queue *q) {
	DCC_Packet *packet = NULL;
	if (q->pivot != -1)
		packet = q->list[q->pivot];
	return packet;
}

void DCC_Packet_Pump_init(DCC_Packet_Pump *pump, DCC_Packet_Queue *queue) {
  pump->queue = queue;
  pump->status = DCC_PACKET_PREAMBLE;
  pump->bit = 0;
  pump->data_count = 0;
}

unsigned int DCC_Packet_Pump_next(DCC_Packet_Pump *pump) {
  unsigned int emit = DCC_ZERO;
  DCC_Packet *packet = DCC_Packet_Queue_peek(pump->queue);
  // printf("STATUS: %u, BIT: %u, DATA_COUNT: %u\n", pump->status, pump->bit, pump->data_count);
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
      emit = ((packet->address >> (pump->bit)) & (0x01)) ? DCC_ONE_BIT_FREQ : DCC_ZERO_BIT_FREQ;
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
      emit = ((packet->data[pump->data_count] >> (pump->bit)) & (0x01)) ? DCC_ONE_BIT_FREQ : DCC_ZERO_BIT_FREQ;
      pump->bit++;
      if (pump->bit >= DCC_PACKET_DATA_LEN) {
        pump->bit = 0;
        pump->data_count++;
        pump->status = DCC_PACKET_DATA_START;
        if (pump->data_count >= packet->data_len) {
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
      emit = ((packet->crc >> (pump->bit)) & (0x01)) ? DCC_ONE_BIT_FREQ : DCC_ZERO_BIT_FREQ;
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
      if (packet->count > 0) {
        packet->count--;
      }
      // Grab next packet.
      if (packet->count == 0) {
    	  DCC_Packet_Queue_delete(pump->queue, packet);
      }
      else {
        packet = DCC_Packet_Queue_next(pump->queue);
      }
      break;
  }
  return emit;
}
