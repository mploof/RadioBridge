// Main Arduino Header
#include <Arduino.h>

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

void setup(void)
{
  Serial.begin(9600);
  bleSetup();
  radioSetup();
}

void loop(void)
{
  checkBLE();
  checkLoRa();
}
