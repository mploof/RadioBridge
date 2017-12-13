#ifndef BLE_H
#define BLE_H

//*************************************************//
//                  Includes
//*************************************************//
#include "Adafruit_BLE.h"
#include "Adafruit_BluefruitLE_SPI.h"
#include "Adafruit_BluefruitLE_UART.h"
#include "BluefruitConfig.h"

//*************************************************//
//             Global Variables
//*************************************************//

extern Adafruit_BluefruitLE_SPI ble;

//*************************************************//
//             Interface Functions
//*************************************************//

void bleSetup(void);
void checkBLE(void);

#endif
