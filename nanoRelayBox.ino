/**
 * The MySensors Arduino library handles the wireless radio link and protocol
 * between your home built sensors/actuators and HA controller of choice.
 * The sensors forms a self healing radio network with optional repeaters. Each
 * repeater and gateway builds a routing tables in EEPROM which keeps track of the
 * network topology allowing messages to be routed to nodes.
 *
 * Created by Henrik Ekblad <henrik.ekblad@mysensors.org>
 * Copyright (C) 2013-2015 Sensnology AB
 * Full contributor list: https://github.com/mysensors/Arduino/graphs/contributors
 *
 * Documentation: http://www.mysensors.org
 * Support Forum: http://forum.mysensors.org
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
**/

// Enable debug prints to serial monitor
#define MY_DEBUG


// Comment it out for Auto Node ID #
#define MY_NODE_ID 0x01  

// Enable and select radio type attached
#define MY_RADIO_RFM69


//#define MY_RFM69_FREQUENCY   RFM69_433MHZ
//#define MY_RFM69_FREQUENCY   RFM69_868MHZ
//#define MY_RFM69_FREQUENCY   RFM69_915MHZ


#define MY_IS_RFM69HW

//Enable OTA feature
//#define MY_OTA_FIRMWARE_FEATURE
//#define MY_OTA_FLASH_JDECID 0 //0x2020

//Enable Crypto Authentication to secure the node
//#define MY_SIGNING_ATSHA204
//#define  MY_SIGNING_REQUEST_SIGNATURES

#include <avr/wdt.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif

#include <MySensors.h>
 //#include <SimpleTimer.h>
#include <stdlib.h>

//--------------------- https://github.com/JonHub/Filters
//#include <Filters.h> 
float testFrequency = 50;                     // test signal frequency (Hz)
float windowLength = 20.0/testFrequency;     // how long to average the signal, for statistist

#define RELAY_pin 7 // Digital pin connected to relay

#define RELAY_sensor 1
#define Current_sensor 2
#define TEMP_sensor 3

#define RELAY_ON 1  // GPIO value to write to turn on attached relay
#define RELAY_OFF 0 // GPIO value to write to turn off attached relay


//#define SPIFLASH_BLOCKERASE_32K   0x52
#define SPIFLASH_BLOCKERASE_32K   0xD8 // Redefine erase block and CHIPERASE commands here. so please keep these two lines AFTER #include <MySensors.h>
#define SPIFLASH_CHIPERASE        0x60

#define RELAY_COUNT 8
int relayChildID[8] = {1, 2, 3, 4, 5, 6, 7, 8};
int relayPin[8] = {A2, 7, A0, 6, 5, 4, 3, A1}; 

MyMessage msg[8];

void before() {
    // watchdog sets to 8 secs
    wdt_enable(WDTO_8S);     //wdt_disable();
  
    // in case watchdog resets node - we do RFM69 reset here since VDD (power) is not disconnected while watchdog resets the node. Just in case!
      /*  RFM reset pin is 9
       *  A manual reset of the RFM69HCW\CW is possible even for applications in which VDD cannot be physically disconnected.
       *  Pin RESET should be pulled high for a hundred microseconds, and then released. The user should then wait for 5 ms
       *  before using the module.
       */
      pinMode(9, OUTPUT);
      //reset RFM module
      digitalWrite(9, 1);
      delay(1);
      // set Pin 9 to high impedance
      pinMode(9, INPUT);
      delay(10);

    
      for (int i = 0; i < RELAY_COUNT; i++) {
        pinMode(relayPin[i], OUTPUT);
      }

    
    // Set relay to last known state (using eeprom storage)
    for (int i = 0; i < RELAY_COUNT; i++) {
      if (relayChildID[i] != NULL) {
      digitalWrite(relayPin[i], loadState(i+1)?RELAY_ON:RELAY_OFF);
      }
    }
}

void setup() {
  
}

void presentation() {  
  // Send the sketch version information to the gateway and Controller
  // char  SketchInfo[] = {"Relay node " && MY_NODE_ID};
  sendSketchInfo("Nano Relay Box node","1.0");

  for (int i = 0; i < RELAY_COUNT; i++) {
    if (relayChildID[i] != NULL) {
      msg[i] = MyMessage(i+1, V_LIGHT); //relayChildID[i]
      present(i+1, S_LIGHT); //relayChildID[i]
    }
  }
}

void loop(){
  wdt_reset();
}

void receive(const MyMessage &message) {
  // We only expect one type of message from controller. But we better check anyway.
  if (message.type==V_LIGHT) {
     // Change relay state
     digitalWrite(relayPin[message.sensor-1], message.getBool()?RELAY_ON:RELAY_OFF);
     // Store state in eeprom
     saveState(message.sensor, message.getBool()?RELAY_ON:RELAY_OFF);
     Serial.print("Incoming change for sensor:");
     Serial.print(message.sensor);
     Serial.print(", New status: ");
     Serial.println(message.getBool());
   }
}

