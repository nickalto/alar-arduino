#include <Time.h>
#include <TimeAlarms.h>
#include <ble_shield.h>
#include <services.h>
#include <boards.h>
#include <SPI.h>
#include <Timer.h>

String time;
String gmtOffset;
time_t currentTime = 0;
int gmt = 0;
int gmtOffsetSign = 1;
boolean receivingTimeStamp = false;
boolean receivingGMTOffset = false;

//******************************************************************************************
//******************************************************************************************
//
// BLE functions
//
//******************************************************************************************
//******************************************************************************************

//#########################################################################################
// Bluetooth loop
// Read in commands send from iOS app, and send commands out. 
//#########################################################################################

void ble_setup() {
  ble_begin();
}

//#########################################################################################
// Bluetooth Send Data
// Given a string loop through characters and write it out.
//#########################################################################################

void ble_send_data(String data) {
  int buflen = 64;
  char buf[buflen];

  data.toCharArray(buf, buflen);

  for(int i = 0; i <= buflen; i++ ) {
    ble_write(buf[i]);  
  }
}

//#########################################################################################
// Bluetooth loop
// Read in commands send from iOS app, and send commands out. 
//#########################################################################################

void ble_loop() {
  unsigned char currentCommand = 0;
  unsigned char on = 0xff;
  
  while ( ble_available() ) {
   
    currentCommand = ble_read();
    boolean available = !handleTimeStamp(currentCommand) && !handleGMTOffset(currentCommand);
    
    if ( available && ble_available() ) {
      // Otherwise we are looking for an on/off value
      on = ble_read() == 1;
    }
  }
  
  
  if (Serial.available())
  {
    char ch = Serial.read();
    if (ch == '1') {              	  
        ble_send_data("Working!");
    } else if( ch == '2' ) {
        ble_send_data("Hello From Arduino");
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

//******************************************************************************************
//******************************************************************************************
//
// Time functions
//
//******************************************************************************************
//******************************************************************************************
#define TIME_MSG_LEN  11   // time sync to PC is HEADER followed by unix time_t as ten ascii digits
#define TIME_HEADER  'T'   // Header tag for serial time sync message
#define TIME_REQUEST  7    // ASCII bell character requests a time sync message 

//#########################################################################################
// Time Setup
// Set sync provider / function to call when sync is required 
//#########################################################################################

void time_setup() {
   setSyncProvider( time_request_sync ); 
}

//#########################################################################################
// Time Loop
// Update time if it is already set
//#########################################################################################

void time_loop() {
  
  if(timeStatus()!= timeNotSet) {
    digitalWrite(13,timeStatus() == timeSet); // on if synced, off if needs refresh  
    time_digital_clock_display();  
  }
}

//#########################################################################################
// Handle Time Stamp
// Checks to see if we are receiving a time stamp and uses it to set time. 
// Sends TIMESET when completed.
//#########################################################################################

boolean handleTimeStamp(unsigned char currentCommand ) {
  const char *timeChar;
  //If a message starts with TxxxxxxxxT it is a timestamp

  if( currentCommand == 'T' ) {
    
    if(receivingTimeStamp) {
      //If we recieve a 'T' while receivingTimeStamp it signals the end of the message
      receivingTimeStamp = !receivingTimeStamp;
     
      // Now to convert that string to a time_t that we can actually use.
      for(int i = 0; i < time.length(); i++){   
        char c = time.charAt(i);          
         if( c >= '0' && c <= '9'){   
           currentTime = (10 * currentTime) + (c - '0') ; // convert digits to a number    
         }
      }
    
      Serial.println(currentTime);
      setTime(currentTime);
      ble_send_data("TIMESET");

    } else {
      //Otherwise it signals the transmission of a new timestamp - start receiving.
       time = "";
       currentTime = 0;
       receivingTimeStamp = !receivingTimeStamp;
    }
    
    return true;
    
  } else if ( receivingTimeStamp ) {
    
    // If we are receiving store values in time string
    timeChar = (const char*)&currentCommand;
    time += atoi(timeChar);
    
    return true;
    
  } 
  
  return false;
}


//#########################################################################################
// Handle Time Stamp
// Checks to see if we are receiving a time stamp and uses it to set time. 
// Sends TIMESET when completed.
//#########################################################################################

boolean handleGMTOffset(unsigned char currentCommand ) {
  const char *timeChar;
  //If a message starts with GxxG it is a GMT Offset

  if( currentCommand == 'G' ) {
    
    if(receivingGMTOffset) {
      //If we recieve a 'G' while receivingGMTOffset it signals the end of the message
      receivingGMTOffset = !receivingGMTOffset;

      // Now to convert that string to a time_t that we can actually use.
      for(int i = 0; i < gmtOffset.length(); i++){   
        char c = gmtOffset.charAt(i);   
         if( c >= '0' && c <= '9'){   
           gmt = (10 * gmt) + (c - '0') ; // convert digits to a number    
         }
      }
      
      gmt = gmt * gmtOffsetSign;
      ble_send_data("GMTSET");

    } else {
      //Otherwise it signals the transmission of a new offset - start receiving.
       gmt = 0;
       gmtOffset = "";
       receivingGMTOffset = !receivingGMTOffset;
    }
    
    return true;
    
  } else if ( receivingGMTOffset ) {
    
    // If we are receiving store values in time string
    if( currentCommand == '-' ) {
      gmtOffsetSign = -1;
    }
    
    if( currentCommand >= '0' && currentCommand <= '9') {   
      timeChar = (const char*)&currentCommand;
      gmtOffset += atoi(timeChar);
    }
    
    return true;
    
  } 
  
  return false;
}


//#########################################################################################
// Time Digital Clock Display
// Debugging utility to print out the current time
//#########################################################################################

void time_digital_clock_display(){
  // digital clock display of the time
  Serial.print(hour() + gmt);
  time_print_digits(minute());
  time_print_digits(second());
  Serial.print(" ");
  Serial.print(day());
  Serial.print(" ");
  Serial.print(month());
  Serial.print(" ");
  Serial.print(year()); 
  Serial.println(); 
}

//#########################################################################################
// Time Loop
// Debugging utility to print out the current time
//#########################################################################################

void time_print_digits(int digits){
  // utility function for digital clock display: prints preceding colon and leading 0
  Serial.print(":");
  if(digits < 10)
    Serial.print('0');
  Serial.print(digits);
}

//#########################################################################################
// Time Request Sync
// Wrapper to be called during setup
//#########################################################################################

time_t time_request_sync()
{
  Serial.print(byte(TIME_REQUEST));  
  return 0; // the time will be sent later in response to serial mesg
}

//******************************************************************************************
//******************************************************************************************
//
// Arduino functions
//
//******************************************************************************************
//******************************************************************************************

void setup() {
  Serial.begin(57600);
  time_setup();
  ble_setup();
}

void loop()
{ 
  time_loop();
  ble_loop();
}
