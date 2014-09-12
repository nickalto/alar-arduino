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
  unsigned char on = 0xff;
  
  while ( ble_available() ) {
    
    currentCommand = ble_read();
    if ( ble_available() ) {
      on = ble_read() == 1;
    }
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
    case 'a':
      Serial.println("Recieved a!");
      break;
      
    case 'b':
      Serial.println("Recieved b!");
      break;
	
  }

  ble_do_events();
}
