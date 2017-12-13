
//*************************************************//
//                  Includes
//*************************************************//

#include <Arduino.h>
#include "LoRa.h"
#include "BLE.H"

// LoRa Headers
#include <SPI.h>
#include <SoftwareSerial.h>
#include <RHReliableDatagram.h>
#include <RH_RF95.h>


//*************************************************//
//                Constants
//*************************************************//

// Change to 434.0 or other frequency, must match RX's freq!
#define RF95_FREQ 915.0
#define RFM95_CS  4
#define RFM95_RST 7
#define RFM95_INT 3


#define DEFAULT_CLIENT_ADDRESS 1
#define DEFAULT_SERVER_ADDRESS 2

#define START_CHAR  '<'
#define STOP_CHAR   '>'


//*************************************************//
//              Local Variables
//*************************************************//

// Singleton instance of the radio driver
static RH_RF95 driver(RFM95_CS, RFM95_INT);

static int cur_client_addr = DEFAULT_CLIENT_ADDRESS;
static int cur_server_addr = DEFAULT_SERVER_ADDRESS;
static bool waiting_for_response = false;

// Class to manage message delivery and receipt, using the driver declared above
static RHReliableDatagram manager(driver, cur_client_addr);

static bool tt_protocol = true;

//*************************************************//
//             Interface Functions
//*************************************************//

void radioSetup(){
    pinMode(RFM95_RST, OUTPUT);
    digitalWrite(RFM95_RST, HIGH);
    
    // manual reset
    digitalWrite(RFM95_RST, LOW);
    delay(10);
    digitalWrite(RFM95_RST, HIGH);
    delay(10);
    
    if (!manager.init())
        Serial.println(F("init failed"));
    else
        Serial.println(F("init OK!"));
   
    // Defaults after init are 434.0MHz, modulation GFSK_Rb250Fd250, +13dbM
    if (!driver.setFrequency(RF95_FREQ)) {
        Serial.println(F("setFrequency failed"));
        while (1);
    }
    Serial.print(F("Set Freq to: ")); Serial.println(RF95_FREQ);
    
    driver.setTxPower(23, false);
}

void checkLoRa(void){
    char in_packet[RADIO_BUFF_SIZE];
    if (manager.available()){
        uint8_t len = sizeof(in_packet);
        uint8_t from;
        if(waiting_for_response){
            // Now wait for a reply from the server
            if (manager.recvfromAckTimeout(in_packet, &len, 2000, &from))
            {
                Serial.println(F("*************"));
                Serial.print(F("got reply from address: 0x"));
                Serial.println(from, HEX);
                Serial.print(F("RSSI: "));
                Serial.println(driver.lastRssi(), DEC);
                Serial.println((char*)in_packet);
                ble.print("Reply from addr: 0x");
                ble.println(from, HEX);
                ble.println((char*)in_packet);
            }
            else
            {
                Serial.print(F("No reply. Is beacon with address "));
                Serial.print(cur_server_addr);
                Serial.println(F(" on and in range?"));

                ble.print(F("No reply. Is beacon with address "));
                ble.print(cur_server_addr);
                ble.println(F(" on and in range?"));
            }
            Serial.println(F("*************\n"));
            waiting_for_response = false;
        }
        // Looking for additional info sent during response
        else{
            // Keep looking for packets till we run out
            if(manager.recvfromAckTimeout(in_packet, &len, &from)){
                Serial.println((char*)in_packet);
                ble.println((char*)in_packet);
            }          
        }
    }
}

void sendRadioPacket(char* data){
  Serial.print(F("Sending to address "));
  Serial.println(cur_server_addr, HEX);
  
  // Adds start and stop characters to the message
  // if using a TinkerTech protocol
  if(tt_protocol){
      int message_len = 0;
      char* this_char = data;
      while(*this_char != '\0'){
          this_char++;
          message_len++;
      }
      for(int i = message_len; i >= 0; i--){
          if(i == message_len){
              data[i] = STOP_CHAR;
          }
          else if(i == 0){
              data[i] = START_CHAR;
          }
          else{
              data[i] = data[i-1];
          }
      }
  }
  // Send a message to manager_server
  if (manager.sendtoWait((uint8_t*)data, RH_RF95_MAX_MESSAGE_LEN, cur_server_addr))
  {
    waiting_for_response = true;
  }
  else
  {
    waiting_for_response = false;
    Serial.println(F("Send failed"));
  }
}

// Local Functions
void setTargetAddress(int new_address){
    cur_server_addr = new_address;
    static const char* lora_str = "Target addr: ";
    ble.print(lora_str);
    ble.println(cur_server_addr);
//    Serial.print(lora_str);
//    Serial.println(cur_server_addr);
}

int getTargetAddress(void){
    return cur_server_addr;
}

void setThisAddress(int new_address){
    manager.setThisAddress(new_address);
    static const char* bridge_str = "Bridge address: ";
    ble.print(bridge_str);
    ble.println(new_address);
//    Serial.print(bridge_str);
//    Serial.println(new_address);
}

int getThisAddress(void){
    return manager.thisAddress();
}

void getRSSI(void){
    static const char* rssi_str = "RSSI: ";
    //Serial.print(rssi_str);Serial.println(driver.lastRssi(), DEC);
    ble.print(rssi_str);ble.println(driver.lastRssi(), DEC);  
}

void setTTProtocol(bool enabled){
    tt_protocol = enabled;
    static const char* enabled_str  = "TT protocol enabled";
    static const char* disabled_str = "TT protocol disabled";
    if(enabled){
        ble.println(enabled_str);
        //Serial.println(enabled_str);
    }
    else{
        ble.println(disabled_str);
        //Serial.println(disabled_str);
    }
}

