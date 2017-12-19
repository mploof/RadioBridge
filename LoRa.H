#ifndef LORA_H
#define LORA_H

#include <RH_RF95.h>
#define RADIO_BUFF_SIZE (RH_RF95_MAX_MESSAGE_LEN + 1)
#define DEFAULT_CLIENT_ADDRESS 1
#define DEFAULT_SERVER_ADDRESS 2
#define DEFAULT_TINKERTECH_PROTOCOL_STATE 1

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
