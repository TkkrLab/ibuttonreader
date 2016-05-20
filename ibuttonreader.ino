/* 
 *  USB iButton reader for the barsystem
 *  Renze Nicolai 2016
 */

#include <OneWire.h>
#include "sha256.h"

/*** Configuration ***/
#define FW_VERSION 1

/*** Pins ***/
#define ONEWIRE_PIN 8
#define ONEWIRE_LED 9
#define INTERNAL_LED 13

/*** Variables ***/
String sha256string = "";

/*** Objects ***/
OneWire ds(ONEWIRE_PIN);

/*** Main program ***/
void setup() {
  Serial.begin(115200); //Start serial port for configuration and debugging
  pinMode(ONEWIRE_LED, OUTPUT);
  pinMode(INTERNAL_LED, OUTPUT);
  /*for (uint8_t i = 0; i<8; i++) {
    digitalWrite(INTERNAL_LED, HIGH);
    delay(50);
    digitalWrite(INTERNAL_LED, LOW);
    delay(50);    
  }*/
  //delay(200); //Give the powersupply time to stabilize
  Serial.println("debug{iButton reader - Renze Nicolai 2016}");
  Serial.println("debug{fwversion="+String(FW_VERSION)+"}");
  Serial.println("debug{ready}");
}

unsigned long heartbeat_time = 0;
uint8_t heartbeat = 0;
bool newdevice = true;

void loop() {
  if (heartbeat_time<millis()) {
    heartbeat_time=millis()+100;
    heartbeat++;
  }
  
  //OneWire
  bool present = ds.reset();
  ds.skip();
  if (present) { //Device present on the bus
    uint8_t addr_in[8] = {0};
    uint8_t addr[8] = {0};
    if (ds.search(addr_in)) {
      if (newdevice) {
        digitalWrite(ONEWIRE_LED, true);
        digitalWrite(INTERNAL_LED, true);
        if (ds.crc8( addr_in, 7) == addr_in[7]) {       
          for (uint8_t i = 0; i<8; i++) {
            addr[i] = addr_in[7-i];
          }
          String idstring = "";
          for (uint8_t i = 0; i<7; i++) { idstring = idstring+String(addr[i]>>4,HEX)+String(addr[i]&0x0F,HEX); }
          Sha256.init();
          //Serial.println("real{"+idstring+"}");
          //Sha256.print(idstring);
          //Serial.print("x{");
          Sha256.print("42SUPERSECRETSALT1337");
          for (uint8_t i = 0; i<7; i++) { Sha256.write(addr[i]); }//Serial.print(String(addr[i], HEX)+","); }
          //Serial.println("}");
          sha256string = "";
          uint8_t* hash = Sha256.result();
          for (uint8_t i = 0; i<32; i++) {
            sha256string = sha256string+("0123456789abcdef"[hash[i]>>4])+("0123456789abcdef"[hash[i]&0xf]);
          }
          Serial.println("sha256{"+sha256string+"}");
          digitalWrite(ONEWIRE_LED, HIGH);
          digitalWrite(INTERNAL_LED, HIGH);
          delay(2000);
          digitalWrite(ONEWIRE_LED, LOW);
          digitalWrite(INTERNAL_LED, LOW);
        } else {
          Serial.println("error{Invalid CRC}");
        }
      }
      newdevice = false;
    } else {
      ds.reset_search();
    }
  } else {
    newdevice = true;
    bool heartbeat_led = (heartbeat&0b00001000)>>3;
    digitalWrite(ONEWIRE_LED, heartbeat_led);
    digitalWrite(INTERNAL_LED, heartbeat_led);
  }
}
