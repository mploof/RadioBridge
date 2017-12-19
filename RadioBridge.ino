// Main Arduino Header
#include <Arduino.h>
#include <EEPROM.h>

// LoRa Headers
#include <SPI.h>
#include <SoftwareSerial.h>
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

#define FIRMWARE_VERSION "1.0.0"

void setup(void)
{
  Serial.begin(9600);
  
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
