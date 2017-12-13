//*************************************************//
//                  Includes
//*************************************************//
#include <Arduino.h>

#include "BLE.H"
#include "LoRa.h"
#include "MetaCommands.h"

// Bluefruit Headers
#include "Adafruit_BLE.h"
#include "Adafruit_BluefruitLE_SPI.h"
#include "Adafruit_BluefruitLE_UART.h"
#include "BluefruitConfig.h"



/*=========================================================================
    APPLICATION SETTINGS

    FACTORYRESET_ENABLE       Perform a factory reset when running this sketch
   
                              Enabling this will put your Bluefruit LE module
                              in a 'known good' state and clear any config
                              data set in previous sketches or projects, so
                              running this at least once is a good idea.
   
                              When deploying your project, however, you will
                              want to disable factory reset by setting this
                              value to 0.  If you are making changes to your
                              Bluefruit LE device via AT commands, and those
                              changes aren't persisting across resets, this
                              is the reason why.  Factory reset will erase
                              the non-volatile memory where config data is
                              stored, setting it back to factory default
                              values.
       
                              Some sketches that require you to bond to a
                              central device (HID mouse, keyboard, etc.)
                              won't work at all with this feature enabled
                              since the factory reset will clear all of the
                              bonding data stored on the chip, meaning the
                              central device won't be able to reconnect.
    MINIMUM_FIRMWARE_VERSION  Minimum firmware version to have some new features
    MODE_LED_BEHAVIOUR        LED activity, valid options are
                              "DISABLE" or "MODE" or "BLEUART" or
                              "HWUART"  or "SPI"  or "MANUAL"
    -----------------------------------------------------------------------*/
    #define FACTORYRESET_ENABLE         1
    #define MINIMUM_FIRMWARE_VERSION    "0.6.6"
    #define MODE_LED_BEHAVIOUR          "MODE"
/*=========================================================================*/

//*************************************************//
//             Global Variables
//*************************************************//

Adafruit_BluefruitLE_SPI ble(BLUEFRUIT_SPI_CS, BLUEFRUIT_SPI_IRQ, BLUEFRUIT_SPI_RST);

//*************************************************//
//             Local Variables
//*************************************************//

//*************************************************//
//             Local Functions
//*************************************************//

// A small helper
static void error(const __FlashStringHelper*err) {
  Serial.println(err);
  while (1);
}

//*************************************************//
//             Interface Functions
//*************************************************//

void bleSetup(void){
 //****** Bluefruit Setup ******//

  /* Initialise the module */
  Serial.print(F("Initialising the Bluefruit LE module: "));

  if ( !ble.begin(VERBOSE_MODE) )
  {
    error(F("Couldn't find Bluefruit, make sure it's in CoMmanD mode & check wiring?"));
  }
  Serial.println( F("OK!") );

  if ( FACTORYRESET_ENABLE )
  {
    /* Perform a factory reset to make sure everything is in a known state */
    Serial.println(F("Performing a factory reset: "));
    if ( ! ble.factoryReset() ){
      error(F("Couldn't factory reset"));
    }
  }

  /* Disable command echo from Bluefruit */
  ble.echo(false);

  Serial.println(F("Requesting Bluefruit info:"));
  /* Print Bluefruit information */
  ble.info();

  Serial.println(F("Please use Adafruit Bluefruit LE app to connect in UART mode"));
  Serial.println(F("Then Enter characters to send to Bluefruit"));
  Serial.println();

  ble.verbose(false);  // debug info is a little annoying after this point!

  /* Wait for connection */
  while (! ble.isConnected()) {
      delay(500);
  }

  Serial.println(F("******************************"));

  // LED Activity command is only supported from 0.6.6
  if ( ble.isVersionAtLeast(MINIMUM_FIRMWARE_VERSION) )
  {
    // Change Mode LED Activity
    Serial.println(F("Change LED activity to " MODE_LED_BEHAVIOUR));
    ble.sendCommandCheckOK("AT+HWModeLED=" MODE_LED_BEHAVIOUR);
  }

  // Set module to DATA mode
  Serial.println( F("Switching to DATA mode!") );
  ble.setMode(BLUEFRUIT_MODE_DATA);

  Serial.println(F("******************************"));
  ble.println("Radio bridge link established!");
}

void checkBLE(void){
    char out_packet[RADIO_BUFF_SIZE];
    // Check for user input
    char n, inputs[BUFSIZE+1];
  
    if (Serial.available())
    {
        n = Serial.readBytes(inputs, RADIO_BUFF_SIZE);
        inputs[n] = 0;
        // Send characters to Bluefruit
        Serial.print(F("Sending: "));
        Serial.println(inputs);
    
        // Send input data to host via Bluefruit
        ble.print(inputs);
    }
  
    if(ble.available())
    {
        // Clear the outgoing radio packet
        int current_char = 0;
        memset(out_packet, '\0', RADIO_BUFF_SIZE);
        // Echo received data
        //Serial.print(F("BLE Message received: "));
        while ( ble.available() )
        {
            int c = ble.read();
      
            //Serial.print((char)c);
            out_packet[current_char] = (char) c;
            current_char++;
        }
        //Serial.println("");
        if(isMetaCommand(out_packet)){
            processMetaCommand(out_packet);
        }
        else{
            sendRadioPacket(out_packet);
        }
    }
}
