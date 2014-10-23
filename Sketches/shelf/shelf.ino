#include <Wire.h>
#include <RTClib.h>
#include <Time.h>
#include <TimeAlarms.h>
#include <ble_shield.h>
#include <services.h>
#include <boards.h>
#include <SPI.h>
#include <Timer.h>

String time;
time_t currentTime = 0;
time_t alarm = 0;
boolean receivingTimeStamp = false;
DateTime rtc_time;
time_t alarms[10];

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
    boolean available = !handleTimeStamp(currentCommand) ;
    
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
RTC_DS1307 RTC;

//#########################################################################################
// Time Setup
// Set sync provider / function to call when sync is required
//#########################################################################################

void time_setup() {
   setSyncProvider( time_request_sync );
   
  Wire.begin();
  RTC.begin();
 
  // This section grabs the current datetime and compares it to
  // the compilation time.  If necessary, the RTC is updated.
  rtc_time = RTC.now();
  DateTime compiled = DateTime(__DATE__, __TIME__);
  if (rtc_time.unixtime() < compiled.unixtime()) {
    Serial.println("RTC is older than compile time! Updating");
    RTC.adjust(DateTime(__DATE__, __TIME__));
  }
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
  
  rtc_time = RTC.now();
  Serial.print("Current time: ");
  Serial.print(rtc_time.year(), DEC);
  Serial.print('/');
  Serial.print(rtc_time.month(), DEC);
  Serial.print('/');
  Serial.print(rtc_time.day(), DEC);
  Serial.print(' ');
  Serial.print(rtc_time.hour(), DEC);
  Serial.print(':');
  Serial.print(rtc_time.minute(), DEC);
  Serial.print(':');
  Serial.print(rtc_time.second(), DEC);
  Serial.println();
 checkAlarm();
}

void checkAlarm() {
 
 boolean is_alarm_today = alarm && rtc_time.year() >= year(alarm) && rtc_time.month() >= month(alarm) && rtc_time.day() >= day(alarm);
 boolean trigger_alarm = is_alarm_today && rtc_time.hour() >= hour(alarm) && rtc_time.minute() >= minute(alarm) && rtc_time.second() >= second(alarm);
 Serial.print("SENT FROM IPHONE!: ");
 Serial.print(year(alarm));
 Serial.print("/");
 Serial.print(month(alarm));
 Serial.print("/");
 Serial.print(day(alarm));
  Serial.print(" ");
 Serial.print(hour(alarm));
 Serial.print(":");
 Serial.print(minute(alarm));
  Serial.print(":");
 Serial.print(second(alarm));
 Serial.println("");
 
 if ( trigger_alarm ) {
   Serial.print("ALARM!!!!!!");
   Serial.println();
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
      
//      setTime(currentTime);
      alarm = currentTime;
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
// Time Digital Clock Display
// Debugging utility to print out the current time
//#########################################################################################

void time_digital_clock_display(){
  // digital clock display of the time
  Serial.print(hour());
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
//  ble.functionPointers[0] = &handleTimeStamp;
  time_setup();
  ble_setup();
}

void loop()
{
  time_loop();
  ble_loop();
}
