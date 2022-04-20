
//*************************************************//
//                  Includes
//*************************************************//

#include <Arduino.h>
#if defined(ADAFRUIT_FEATHER_M0) || defined(ADAFRUIT_FEATHER_M0_EXPRESS) || defined(ARDUINO_SAMD_FEATHER_M0)
  #include <FlashAsEEPROM.h>
  #include <FlashStorage.h>
#else  
  #include <EEPROM.h>
#endif
#include "LoRa.h"
#include "BLE.H"

// LoRa Headers
#include <SPI.h>
#include <RHReliableDatagram.h>
#include <RH_RF95.h>

#include "EEPROM_Map.h"

//*************************************************//
//                Constants
//*************************************************//

#if defined(ESP8266)
  /* for ESP w/featherwing */ 
  #define RFM95_CS  2    // "E"
  #define RFM95_RST 16   // "D"
  #define RFM95_INT 15   // "B"

#elif defined(ADAFRUIT_FEATHER_M0) || defined(ADAFRUIT_FEATHER_M0_EXPRESS) || defined(ARDUINO_SAMD_FEATHER_M0)
  // Feather M0 w/Radio
  #define RFM95_CS      8
  #define RFM95_INT     3
  #define RFM95_RST     4

#elif defined(ARDUINO_ADAFRUIT_FEATHER_ESP32S2) || defined(ARDUINO_NRF52840_FEATHER) || defined(ARDUINO_NRF52840_FEATHER_SENSE)
  #define RFM95_INT     9  // "A"
  #define RFM95_CS      10  // "B"
  #define RFM95_RST     11  // "C"
  
#elif defined(ESP32)  
  /* ESP32 feather w/wing */
  #define RFM95_RST     27   // "A"
  #define RFM95_CS      33   // "B"
  #define RFM95_INT     12   //  next to A

#elif defined(ARDUINO_NRF52832_FEATHER)
  /* nRF52832 feather w/wing */
  #define RFM95_RST     7   // "A"
  #define RFM95_CS      11   // "B"
  #define RFM95_INT     31   // "C"
  
#elif defined(TEENSYDUINO)
  /* Teensy 3.x w/wing */
  #define RFM95_RST     9   // "A"
  #define RFM95_CS      10   // "B"
  #define RFM95_INT     4    // "C"
#else
  /* TinkerTech BLE Radio Bridge */
  #define RFM95_CS  4
  #define RFM95_RST 7
  #define RFM95_INT 3
#endif

// Change to 434.0 or other frequency, must match RX's freq!
#define RF95_FREQ 915.0
#define START_CHAR  '<'
#define STOP_CHAR   '>'


//*************************************************//
//              Local Variables
//*************************************************//

// Singleton instance of the radio driver
static RH_RF95 driver(RFM95_CS, RFM95_INT);

static int this_addr    = 0;
static int target_addr  = 0;

static bool waiting_for_response = false;

// Class to manage message delivery and receipt, using the driver declared above
static RHReliableDatagram manager(driver, this_addr);

static bool tt_protocol;

static const char* no_reply_str_0 = "No reply."; 
static const char* no_reply_str_1 = "Is receiver with address ";
static const char* no_reply_str_2 = " on and in range?";
static const char* send_failed_str= "Send failed";

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
    tt_protocol   = EEPROM.read(EE_TT_PROTOCOL);
    this_addr     = EEPROM.read(EE_THIS_ADDRESS + 1) << 8;
    this_addr    |= EEPROM.read(EE_THIS_ADDRESS);
    target_addr   = EEPROM.read(EE_TARGET_ADDRESS + 1) << 8;
    target_addr  |= EEPROM.read(EE_TARGET_ADDRESS);
    manager.setThisAddress(this_addr);
    Serial.print("This addr: ");Serial.println(this_addr);
    Serial.print("Target addr: ");Serial.println(target_addr);
}

void checkLoRa(void){
    char in_packet[RADIO_BUFF_SIZE];
    if (manager.available()){
        uint8_t len = sizeof(in_packet);
        uint8_t from;
        if(waiting_for_response){
            // Now wait for a reply from the server
            if (manager.recvfromAckTimeout((uint8_t*)in_packet, &len, 2000, &from))
            {
                Serial.println(F("*************"));
                Serial.print(F("got reply from address: 0x"));
                Serial.println(from, HEX);
                Serial.print(F("RSSI: "));
                Serial.println(driver.lastRssi(), DEC);
                Serial.println((char*)in_packet);
                if(bleActive()){
                  ble.print("Reply from addr: 0x");
                  ble.println(from, HEX);
                  ble.println((char*)in_packet);  
                }
                
            }
            else
            {
                Serial.print(no_reply_str_0);
                Serial.print(no_reply_str_1);
                Serial.print(target_addr);
                Serial.println(no_reply_str_2);
                if(bleActive()){
                  ble.print(no_reply_str_0);
                  ble.print(no_reply_str_1);
                  ble.print(target_addr);
                  ble.println(no_reply_str_2);  
                }                
            }
            Serial.println(F("*************\n"));
            waiting_for_response = false;
        }
        // Looking for additional info sent during response
        else{
            // Keep looking for packets till we run out
            if(manager.recvfromAckTimeout((uint8_t*)in_packet, &len, 2000, &from)){
                Serial.println((char*)in_packet);
                if(bleActive()){
                  ble.println((char*)in_packet);
                }
            }          
        }
    }
}

void sendRadioPacket(char* data){
  Serial.print(F("Sending to address "));
  Serial.println(target_addr, HEX);
  
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
  if (manager.sendtoWait((uint8_t*)data, RH_RF95_MAX_MESSAGE_LEN, target_addr))
  {
      waiting_for_response = true;
       Serial.println(F("OK!"));
  }
  else
  {
      waiting_for_response = false;
  
      Serial.println(send_failed_str);
      Serial.print(no_reply_str_1);
      Serial.print(target_addr);
      Serial.println(no_reply_str_2);

      if(bleActive()){
        ble.println(send_failed_str);
        ble.print(no_reply_str_1);
        ble.print(target_addr);
        ble.println(no_reply_str_2);
      }
  }
}

// Local Functions
void setTargetAddress(int new_address){
    target_addr = new_address;
}

int getTargetAddress(void){
    return target_addr;
}

void setThisAddress(int new_address){
    manager.setThisAddress(new_address);
    EEPROM.write(EE_THIS_ADDRESS, new_address);
}

int getThisAddress(void){
    return manager.thisAddress();
}

void getRSSI(void){
    static const char* rssi_str = "RSSI: ";
    //Serial.print(rssi_str);Serial.println(driver.lastRssi(), DEC);
    if(bleActive()){
      ble.print(rssi_str);ble.println(driver.lastRssi(), DEC);
    }
    Serial.print(rssi_str);Serial.println(driver.lastRssi(), DEC);
}

void setTTProtocol(bool enabled){
    tt_protocol = enabled;
    static const char* enabled_str  = "TT protocol enabled";
    static const char* disabled_str = "TT protocol disabled";
    if(enabled){
        if(bleActive()){
          ble.println(enabled_str);
        }
        Serial.println(enabled_str);
    }
    else{
        if(bleActive()){
          ble.println(disabled_str);
        }
        Serial.println(disabled_str);
    }
}
