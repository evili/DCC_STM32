#include "dcc.h"

/**
    @brief  Pretty print DCC packet.
    @param  DCC_Packet packet
    @param  char *string
    @retval None
*/
void dcc_pretty_print(DCC_Packet packet, const char *string = "") {
  Serial.printf("%s DCC_Packet: {%02X, {%02X", string, packet.address, packet.data[0]);
  for (int i = 1; i < packet.data_len; i++) {
    Serial.printf(", %02X", packet.data[i]);
  }
  Serial.printf("}, %02X}\n", packet.crc);
}

void DCC_Packet_adjust_crc(DCC_Packet *p) {
  p->crc = p->address;
  for (int i = 0; i < p->data_len; i++) {
	  p->crc ^= p->data[i];
  }
}

void DCC_Packet_set_address(DCC_Packet *p, unsigned char addr) {
  p->address = addr;
  adjust_crc();
}

void DCC_Packet_set_speed(DCC_Packet *p, unsigned char speed, unsigned char dir) {
  p->data_len = 2;
  p->data[0] = DCC_PACKET_SPEED_128;
  unsigned char adjust_speed = (speed > 127) ? 127 : speed;
  p->data[1] = ((dir & 0x01) << DCC_PACKET_SPEED_128_DIR_BIT) | adjust_speed;
  p->adjust_crc();
}


void DCC_Packet_Queue_init(DCC_Packet_Queue *q) {
  q->first = q;
  q->next = nullptr;
  q->packet = &DCC_Packet_Idle;
  q->count = -1;
}

int  DCC_Packet_Queue_Add_DCC_Packet(DCC_Packet_Queue *q, const DCC_Packet *packet, short count = 0) {
  DCC_Packet_Queue *node = new DCC_Packet_Queue();
  node->first = q->first;
  node->packet = packet;
  node->count = count;
  if (node->count == 0) {
    node->count = DCC_PACKET_DEFAULT_REPEAT;
  }
  DCC_Packet_Queue *pivot;
  for (pivot = q->first; pivot->next != nullptr; pivot = pivot->next);
  pivot->next = node;
  node->next  = nullptr;
  return 0;
}

void DCC_Packet_Queue_delete(DCC_Packet_Queue *q) {
  if(q == q->first) {
	  return;
  }
  DCC_Packet_Queue *prev;
  for (prev = first; prev->next != q; prev = prev->next);
  prev->next = q->next;
  free(q);
}

void DCC_Packet_Pump::DCC_Emit(DCC_Bit bit) {
}

void DCC_Packet_Pump_init(DCC_Packet_Pump *pump, DCC_Packet_Queue *queue) {
  pump->queue = queue;
  pump->status = DCC_PACKET_PREAMBLE;
  pump->bit = 1;
  pump->data_count = 0;
}


unsigned int DCC_Packet_Pump_next(DCC_Packet_Pump *pump) {
  unsigned int emit = DCC_ZERO;
  switch (pump->status) {
    case DCC_PACKET_PREAMBLE:
      emit = DCC_ZERO;
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
      break;
    case DCC_PACKET_DATA_START:
      break;
    case DCC_PACKET_DATA:
    	pump->next_bit = DCC_ONE;
    	pump->emit = next_bit;
    	pump->bit++;
      if (pump->bit >= DCC_PACKET_DATA_LEN) {
    	  pump->bit = 0;
    	  pump->data_count++;
    	  pump->status = DCC_PACKET_DATA_START;
        if (pump->data_count >= pump->queue->packet->data_len) {
          status = DCC_PACKET_CRC_START;
          pump->data_count = 0;
        }
      }
      break;
    case DCC_PACKET_CRC_START:
      break;
    case DCC_PACKET_CRC:
      break;
    case DCC_PACKET_END:
      break;
  }
  return emit;
}
