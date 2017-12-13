#ifndef LORA_H
#define LORA_H

#include <RH_RF95.h>
#define RADIO_BUFF_SIZE (RH_RF95_MAX_MESSAGE_LEN + 1)

void radioSetup(void);
void checkLoRa(void);
void sendRadioPacket(char* data);
void setTargetAddress(int new_address);
int getTargetAddress(void);
void setThisAddress(int new_address);
int getThisAddress(void);
void getRSSI(void);
void setTTProtocol(bool enabled);

#endif
