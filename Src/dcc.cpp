#include "dcc.h"
#include "Arduino.h"

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

void DCC_Packet::adjust_crc() {
  crc = address;
  for (int i = 0; i < data_len; i++) {
    crc ^= data[i];
  }
}

void DCC_Packet::set_address(unsigned char addr) {
  address = addr;
  adjust_crc();
}

void DCC_Packet::set_speed(unsigned char speed, unsigned char dir) {
  data_len = 2;
  data[0] = DCC_PACKET_SPEED_128;
  unsigned char adjust_speed = (speed > 127) ? 127 : speed;
  data[1] = ((dir & 0x01) << DCC_PACKET_SPEED_128_DIR_BIT) | adjust_speed;
  adjust_crc();
}


DCC_Packet_Queue::DCC_Packet_Queue() {
  this->first = this;
  this->next = nullptr;
  this->packet = &DCC_Packet_Idle;
  this->count = -1;
}

DCC_Packet_Queue::DCC_Packet_Queue(DCC_Packet_Queue *first, const DCC_Packet *packet, unsigned char count) :
  first(first), packet(packet), count(count)
{
}

int  DCC_Packet_Queue::Add_DCC_Packet(const DCC_Packet *packet, short count) {
  DCC_Packet_Queue *node = new DCC_Packet_Queue(this->first, packet, count);
  node->first = this->first;
  node->packet = packet;
  node->count = count;
  if (node->count == 0) {
    node->count = DCC_PACKET_DEFAULT_REPEAT;
  }
  DCC_Packet_Queue *pivot;
  for (pivot = this->first; pivot->next != nullptr; pivot = pivot->next);
  pivot->next = node;
  node->next  = nullptr;
  return 0;
}

DCC_Packet_Queue::~DCC_Packet_Queue() {
  DCC_Packet_Queue *prev;
  for (prev = first; prev->next != this; prev = prev->next);
  prev->next = this->next;
}

void DCC_Packet_Pump::DCC_Emit(DCC_Bit bit) {
}

DCC_Packet_Pump::DCC_Packet_Pump(DCC_Packet_Queue *queue) : queue(queue) {
  status = DCC_PACKET_PREAMBLE;
  bit = 1;
  data_count = 0;
}


unsigned int DCC_Packet_Pump::next() {
  unsigned int emit = DCC_ZERO;
  switch (status) {
    case DCC_PACKET_PREAMBLE:
      emit = DCC_ZERO;
      bit++;
      if (bit >= DCC_PACKET_PREAMBLE_LEN) {
        status = DCC_PACKET_ADDRESS_START;
        bit = 0;
      }
      break;
    case DCC_PACKET_ADDRESS_START:
      emit = DCC_ZERO;
      status = DCC_PACKET_ADDRESS;
      bit = 0;
      break;
    case DCC_PACKET_ADDRESS:
      break;
    case DCC_PACKET_DATA_START:
      break;
    case DCC_PACKET_DATA:
      next_bit = DCC_ONE;
      emit = next_bit;
      bit++;
      if (bit >= DCC_PACKET_DATA_LEN) {
        bit = 0;
        data_count++;
        status = DCC_PACKET_DATA_START;
        if (data_count >= queue->packet->data_len) {
          status = DCC_PACKET_CRC_START;
          data_count = 0;
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
