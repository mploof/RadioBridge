// Main Arduino Header
#include <Arduino.h>
#if defined(ADAFRUIT_FEATHER_M0) || defined(ADAFRUIT_FEATHER_M0_EXPRESS) || defined(ARDUINO_SAMD_FEATHER_M0)
  #include <FlashAsEEPROM.h>
  #include <FlashStorage.h>
#else  
  #include <EEPROM.h>
#endif


// LoRa Headers
#include <SPI.h>
#include <RHReliableDatagram.h>
#include <RH_RF95.h>

// Bluefruit Headers
#include "Adafruit_BLE.h"
#include "Adafruit_BluefruitLE_SPI.h"
#include "Adafruit_BluefruitLE_UART.h"
#include "BluefruitConfig.h"

// Meta Header
#include "MetaCommands.h"
#include "BLE.h"
#include "LoRa.h"
#include "EEPROM_Map.h"

#define FIRMWARE_VERSION "1.1.0"

void setup(void)
{
  Serial.begin(115200);
  
  Serial.println(F("*********************************"));
  Serial.println(F("TinkerTech BLE/LoRa Radio Bridge"));
  Serial.print(F("Firmware Version: "));Serial.println(FIRMWARE_VERSION);
  Serial.println(F("*********************************"));
  
  if(EEPROM.read(EE_FIRST_POWER) != NOT_FIRST_POWER){
      EEPROM.write(EE_TARGET_ADDRESS,     (uint8_t)DEFAULT_SERVER_ADDRESS);
      EEPROM.write(EE_TARGET_ADDRESS + 1, (uint8_t)(DEFAULT_SERVER_ADDRESS >> 8));
      EEPROM.write(EE_THIS_ADDRESS,       (uint8_t)DEFAULT_CLIENT_ADDRESS);
      EEPROM.write(EE_THIS_ADDRESS + 1,   (uint8_t)(DEFAULT_CLIENT_ADDRESS >> 8));
      EEPROM.write(EE_TT_PROTOCOL,        DEFAULT_TINKERTECH_PROTOCOL_STATE);
  }
  bleSetup();
  radioSetup();
}

void loop(void)
{
  checkBLE();
  checkLoRa();
}
