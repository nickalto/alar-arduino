#include <ble_shield.h>
#include <services.h>
#include <boards.h>
#include <SPI.h>
#include "Timer.h"


//******************************************************************************************
// Arduino functions
//******************************************************************************************

void setup() {
  Serial.begin(57600);
  ble_begin();
}

void sendData(String data) {
  int buflen = 64;
  char buf[buflen];

  data.toCharArray(buf, buflen);

  for(int i = 0; i <= buflen; i++ ) {
    ble_write(buf[i]);  
  }
}

void loop()
{
  Serial.println("Hello World");

  unsigned char currentCommand = 0;
  
  while ( ble_available() ) {
    currentCommand = ble_read();
  }

  if (Serial.available())
  {
    char ch = Serial.read();
    if (ch == '1') {              	  
        Serial.println("1 Hit.");
        sendData("test");
    } else if( ch == '2' ) {
        Serial.println("1 Hit.");
    }
  }
  
  switch (currentCommand) {
    case 's':
      Serial.println("Signal off");
      break;
      
    case 'l':
      Serial.println("Left");
      break;
	
  }

  ble_do_events();
}
