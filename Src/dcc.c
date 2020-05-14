#include "dcc.h"
#include "cmsis_os.h"
// #include "printf-stdarg.h"

#ifndef NULL
#define NULL 0
#endif

void DCC_Packet_adjust_crc(DCC_Packet *p) {
  p->crc = p->address_high;
  if (p->address_high != DCC_BROADCAST_ADDRESS ) {
	  if (p->address_high > DCC_SHORT_ADDRESS_MAX) {
		  p->crc ^= p->address_low;
	  }
  }
  for (int i = 0; i < p->data_len; i++) {
	  p->crc ^= p->data[i];
  }
}

void DCC_Packet_set_address(DCC_Packet *p, uint16_t addr) {
  if(addr == DCC_BROADCAST_ADDRESS) {
	  p->address_high = DCC_BROADCAST_ADDRESS;
	  p->address_low = 0x00u;
  }
  else {
	  if(addr <= DCC_ADDRESS_MAX) {
		  p->address = addr;
		  if(p->address > DCC_SHORT_ADDRESS_MAX) {
			  p->address |= 0xC000u; // 0b11000000
		  }
	  }
  }
  DCC_Packet_adjust_crc(p);
}

void DCC_Packet_set_speed(DCC_Packet *p, uint8_t speed, uint8_t dir) {
  p->data_len = 2;
  p->data[0] = DCC_PACKET_SPEED_128;
  unsigned char adjust_speed = (speed > 126) ? 126 : speed;
  p->data[1] = ((dir & 0x01) << DCC_PACKET_SPEED_128_DIR_BIT) | adjust_speed;
  DCC_Packet_adjust_crc(p);
}

void DCC_Packet_set_speed_28(DCC_Packet *p, uint8_t speed, uint8_t dir) {
  // Speed = 01DCSSSS = 01 D = Direction, C = First-Bit,  S= 4-High-bits 
  p->data_len = 1;
  p->data[0] = DCC_PACKET_SPEED_28_START;
  // Direction bit is bit 5
  if(dir)
    p->data[0] |= DCC_PACKET_SPEED_28_DIRECTION_BIT;
  // Step 0 == Stop
  if (speed != 0) {
    // Step 1 == Emergency Stop
    speed = (speed > 26) ? 26 : speed;
    // least significant speed bit 0 goes to bit 4
    if(speed & 0x01)
      p->data[0] |= DCC_PACKET_SPEED_28_FIRST_BIT;
    // the rest goes to bits 3-0, starting at 4, so we shift
    p->data[0] |= (speed+4) >> 1;
  }
  DCC_Packet_adjust_crc(p);
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
  static unsigned long emit = DCC_ZERO;
//
// Debug Signal:
//  define DCC_PUMP_DEBUG_ZERO to
// produce a pure squared wave with period 200us
// define DCC_PUMP_DEBUG_ONE to
// produce a pure squared wave with period 116us
// #define DCC_PUMP_DEBUG_ALTERN
#ifdef DCC_PUMP_DEBUG_ZERO
  emit = DCC_ZERO;
#elif defined(DCC_PUMP_DEBUG_ONE)
  emit = DCC_ONE;
#elif defined(DCC_PUMP_DEBUG_ALTERN)
  emit = (emit == DCC_ONE) ? DCC_ZERO : DCC_ONE;
#else
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
      emit = ((pump->packet->address_high << (pump->bit)) & (0x80u)) ? DCC_ONE : DCC_ZERO;
      pump->bit++;
      if (pump->bit >= DCC_PACKET_ADDRES_LEN) {
    	  pump->status = (
    			  (pump->packet->address_high == DCC_BROADCAST_ADDRESS) ||
				  (pump->packet->address_high <= DCC_SHORT_ADDRESS_MAX)
    	  ) ? DCC_PACKET_DATA_START: DCC_PACKET_ADDRESS_LOW_START;
        pump->data_count = 0;
        pump->bit = 0;
      }
      break;
    case DCC_PACKET_ADDRESS_LOW_START:
      emit = DCC_ZERO;
      pump->status = DCC_PACKET_ADDRESS_LOW;
      pump->bit = 0;
      break;
    case DCC_PACKET_ADDRESS_LOW:
      emit = ((pump->packet->address_low << (pump->bit)) & (0x80u)) ? DCC_ONE : DCC_ZERO;
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
      emit = ((pump->packet->data[pump->data_count] << (pump->bit)) & (0x80u)) ? DCC_ONE : DCC_ZERO;
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
      emit = ((pump->packet->crc >> (pump->bit)) & (0x01)) ? DCC_ONE : DCC_ZERO;
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
    	// printf("%s\n", "Count is zero. Freeing");
    	vPortFree(pump->packet);
      }
      else {
    	// printf("%s\n", "Returning packet to queue");
        status = osMessageQueuePut(pump->queue, (void *) pump->packet, 0U, 0U);
        // if(status != osOK)
      	//  DCC_ERROR(status, "Can't put on queue.");
      }
      // printf("%s\n","Getting new packet.");
      status = osMessageQueueGet(pump->queue, (void *) pump->packet, 0U, 0U);
      // if(status != osOK) {
      //  if (osErrorResource == status)
      //	  DCC_ERROR(status, "Nothing to get from queue.");
    	  // printf("%s\n", "No message in queue");
      // }
      break;
  }
#endif // DCC_PUMP_DEBUG_ZERO
  return emit;
}
