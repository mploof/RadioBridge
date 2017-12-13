//*************************************************//
//                 Includes
//*************************************************//

#include <Arduino.h>
#include "MetaCommands.h"
#include "LoRa.h"
#include "BLE.H"
#include "MemoryFree.h"

#define META_CMD_CHAR '@'

//*************************************************//
//              Local Variables
//*************************************************//

static const char* SET_THIS_ADDR            = "sb";
static const char* SET_TARGET_ADDR          = "st";
static const char* GET_THIS_ADDR            = "gb";
static const char* GET_TARGET_ADDR          = "gt";
static const char* GET_RSSI                 = "gr";
static const char* SET_TINKERTECH_PROTOCOL  = "tt";

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
    memset(int_str, '\0', 3);
    if((buff[1] == SET_THIS_ADDR[0] && buff[2] == SET_THIS_ADDR[1]) ||
       (buff[1] == SET_TARGET_ADDR[0] && buff[2] == SET_TARGET_ADDR[1]))
    {
        bool valid_address = true;
        if(buff[4] < '0' || buff[4] > '9'){
           ble.println("Invalid address");
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
        Serial.print(F("int_str: "));Serial.println(int_str);
        Serial.print(F("temp_val: "));Serial.println(temp_val);
        if(valid_address && buff[1] == SET_THIS_ADDR[0] && buff[2] == SET_THIS_ADDR[1]){
            setThisAddress(temp_val);
        }
        else if(valid_address && (buff[1] == SET_TARGET_ADDR[0] && buff[2] == SET_TARGET_ADDR[1])){
            setTargetAddress(temp_val);
        }
    }
    else if(buff[1] == GET_THIS_ADDR[0] && buff[2] == GET_THIS_ADDR[1]){
        int addr = getThisAddress();
        ble.println(addr);
    }
    else if(buff[1] == GET_TARGET_ADDR[0] && buff[2] == GET_TARGET_ADDR[1]){
        int addr = getTargetAddress();
        ble.println(getTargetAddress());
    }
    else if(buff[1] == GET_RSSI[0] && buff[2] == GET_RSSI[1]){
        getRSSI();
    }
    else if(buff[1] == SET_TINKERTECH_PROTOCOL[0] && buff[2] == SET_TINKERTECH_PROTOCOL[1]){
        temp_val = buff[4] == '0' ? false : true;
        setTTProtocol(temp_val);
    }
    else{
        ble.println("Invalid metacommand");
    }
    Serial.print("FM: ");
    Serial.println(freeMemory());
}

