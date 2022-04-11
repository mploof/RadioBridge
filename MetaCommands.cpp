//*************************************************//
//                 Includes
//*************************************************//

#include <Arduino.h>
#include "MetaCommands.h"
#include "LoRa.h"
#include "BLE.H"
#include "MemoryFree.h"

#define META_CMD_CHAR '#'

//*************************************************//
//              Local Variables
//*************************************************//

static const char* SET_THIS_ADDR            = "sa";
static const char* SET_TARGET_ADDR          = "st";
static const char* GET_THIS_ADDR            = "ga";
static const char* GET_TARGET_ADDR          = "gt";
static const char* GET_RSSI                 = "gr";
static const char* SET_TINKERTECH_PROTOCOL  = "sp";
static const char* GET_TINKERTECH_PROTOCOL  = "gp";

//*************************************************//
//             Interface Functions
//*************************************************//

bool isMetaCommand(char* buff){
    int buff_size = 0;
    if(buff[0] == META_CMD_CHAR){
        return true;
    }
    else{
        return false;
    }
}

void processMetaCommand(char* buff){
    int temp_val;
    char int_str[3];
    char out_str[20];
    memset(int_str, '\0', 3);
    memset(out_str, '\0', 20);
    if((buff[1] == SET_THIS_ADDR[0] && buff[2] == SET_THIS_ADDR[1]) ||
       (buff[1] == SET_TARGET_ADDR[0] && buff[2] == SET_TARGET_ADDR[1]))
    {
        bool valid_address = true;
        if(buff[4] < '0' || buff[4] > '9'){
           Serial.println("Invalid address");
           valid_address = false;
        }
        else{
            Serial.print(F("Buffer: "));Serial.println(buff);
            int_str[0] = buff[4];
            if(buff[5] >= '0' && buff[5] <= '9'){
                int_str[1] = buff[5];
            }    
        }
        temp_val = atoi(int_str);
        if(valid_address && buff[1] == SET_THIS_ADDR[0] && buff[2] == SET_THIS_ADDR[1]){
            setThisAddress(temp_val);
            if(bleActive()) {
              ble.println("Bridge set!");  
            }
            Serial.println("Bridge set!");
        }
        else if(valid_address && (buff[1] == SET_TARGET_ADDR[0] && buff[2] == SET_TARGET_ADDR[1])){
            setTargetAddress(temp_val);
            if(bleActive()) {
              ble.println("Target set!");  
            }
            Serial.println("Target set!");
        }
    }
    else if(buff[1] == GET_THIS_ADDR[0] && buff[2] == GET_THIS_ADDR[1]){
        int addr = getThisAddress();
        char addr_str[5];
        char out_str[20];
        itoa(addr, addr_str, 10);
        strcpy(out_str, "Bridge addr: ");
        strcat(out_str, addr_str);
        if(bleActive()) {
          ble.println(out_str);  
        }
        Serial.println(out_str);
    }
    else if(buff[1] == GET_TARGET_ADDR[0] && buff[2] == GET_TARGET_ADDR[1]){
        int addr = getTargetAddress();
        char addr_str[5];
        char out_str[20];
        itoa(addr, addr_str, 10);
        strcpy(out_str, "Target addr: ");
        strcat(out_str, addr_str);
        if(bleActive()) {
          ble.println(out_str);  
        }
        Serial.println(out_str);
    }
    else if(buff[1] == GET_RSSI[0] && buff[2] == GET_RSSI[1]){
        getRSSI();
    }
    else if(buff[1] == SET_TINKERTECH_PROTOCOL[0] && buff[2] == SET_TINKERTECH_PROTOCOL[1]){
        temp_val = buff[4] == '0' ? false : true;
        setTTProtocol(temp_val);
    }
    else if(buff[1] == GET_RSSI[0] && buff[2] == GET_RSSI[1]){
        getRSSI();
    }
    else{
      if(bleActive()) {
        ble.println("Invalid metacommand");
      }
       Serial.println("Invalid metacommand");
    }
    Serial.print("FM: ");
    Serial.println(freeMemory());
}
