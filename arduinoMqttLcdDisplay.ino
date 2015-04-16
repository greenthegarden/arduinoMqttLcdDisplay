/*--------------------------------------------------------------------------------------
  Includes
--------------------------------------------------------------------------------------*/
#include <Wire.h>
#include <LiquidCrystal.h>   // include LCD library

#include <SPI.h>
#include <Ethernet.h>

#include <PubSubClient.h>
#include <Time.h>

#include "config.h"

#if ( DEBUG || !LCD_PRINT )
void debug(const __FlashStringHelper * console_text)
{
  Serial.println(console_text);
}
#endif



/*-------- NTP code ----------
 Get the time from a Network Time Protocol (NTP) time server
 Demonstrates use of UDP sendPacket and ReceivePacket
 For more on NTP time servers and the messages needed to communicate with them,
 see http://en.wikipedia.org/wiki/Network_Time_Protocol

 Source: TimeNTP from Time library
*/
time_t prevDispTime = 0;

#include <EthernetUdp.h>

// A UDP instance to let us send and receive packets over UDP
EthernetUDP Udp;
unsigned int localPort = 8888;  // local port to listen for UDP packets

IPAddress SNTP_server_IP( 192, 168, 1, 55 );   // Pi
//IPAddress SNTP_server_IP( 192, 168, 1, 100 );  // Storology
//IPAddress SNTP_server_IP( 203, 0, 178, 191 );  // iinet ntp server 203.0.178.191

const int NTP_PACKET_SIZE = 48; // NTP time stamp is in the first 48 bytes of the message
byte packetBuffer[ NTP_PACKET_SIZE]; //buffer to hold incoming and outgoing packets

// send an NTP request to the time server at the given address
void sendNTPpacket(IPAddress &address)
{
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;
  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  Udp.beginPacket(address, 123); //NTP requests are to port 123
  Udp.write(packetBuffer, NTP_PACKET_SIZE);
  Udp.endPacket();
}

time_t getNtpTime()
{
  while (Udp.parsePacket() > 0) ; // discard any previously received packets
#if DEBUG
  debug(F("Transmit NTP Request"));
#endif
  sendNTPpacket(SNTP_server_IP);
  uint32_t beginWait = millis();
  while (millis() - beginWait < 1500)
  {
    int size = Udp.parsePacket();
    if (size >= NTP_PACKET_SIZE)
    {
#if DEBUG
      debug(F("Receive NTP Response"));
#endif
      Udp.read(packetBuffer, NTP_PACKET_SIZE);  // read packet into the buffer
      unsigned long secsSince1900;
      // convert four bytes starting at location 40 to a long integer
      secsSince1900 =  (unsigned long)packetBuffer[40] << 24;
      secsSince1900 |= (unsigned long)packetBuffer[41] << 16;
      secsSince1900 |= (unsigned long)packetBuffer[42] << 8;
      secsSince1900 |= (unsigned long)packetBuffer[43];
      return secsSince1900 - 2208988800UL + (TZ_OFFSET_HOURS * SECS_PER_HOUR + TZ_OFFSET_MINUTES * SECS_PER_MIN);
    }
  }
#if DEBUG
  debug(F("No NTP Response :-("));
#endif
  return 0; // return 0 if unable to get the time
}

void reportTimeString()
{
  weather_report_time[0] = '\0';
  sprintf(weather_report_time, "%02d:%02d", hour(), minute());
}

// use TimeAlarms to switch off lcd backlight after a delay
//#include <TimeAlarms.h>

#if USE_MQTT
void callback(char* topic, uint8_t* payload, unsigned int payload_length)
{
  // Copy the payload to the new buffer
  char* message = (char*)malloc((sizeof(char)*payload_length)+1); // get the size of the bytes and store in memory 
  memcpy(message, payload, payload_length*sizeof(char));          // copy the memory
  message[payload_length*sizeof(char)] = '\0';                    // add terminating character

  // since only subscribing to weather messages 
  if (  strcmp(topic, "all/control/dst") == 0 )
  {
    DST = true;
  }
  else if ( strcmp(topic, "weather/measurement/SHT15_temp") == 0 )
  {
    sprintf(weather_report_temp, "%4s", message);
    reportTimeString();
    weather_report_new = true;
  }
  else if ( strcmp(topic, "weather/measurement/SHT15_humidity") == 0 )
  {
    sprintf(weather_report_hum, "%4s", message);
    reportTimeString();
    weather_report_new = true;
  }
//  else if ( strcmp(topic, "weather/measurement/BMP085_pressure") == 0 )
//  {
//    sprintf(weather_report_pres, "%4s", message);
//    reportTimeString();
//    weather_fields_set = true;
//  }
//  else if ( strcmp(topic, "weather/status/battery") == 0 )
//  {
//    sprintf(weather_report_pwr, "%4s", message);
//    reportTimeString();
//    weather_fields_set = true;
//  }
  free(message);
}

EthernetClient ethClient;
PubSubClient mqttClient(mqtt_server_addr, mqtt_port, callback, ethClient);

void publishConnected()
{
  prog_buffer_1[0] = '\0';
  strcpy_P(prog_buffer_1, (char*)pgm_read_word(&(status_topics[0])));
  mqttClient.publish(prog_buffer_1, "Connected");
}
#endif

//void timeString()
//{
//  char_buffer[0] = '\0';
//  sprintf(char_buffer,
//          "%02d:%02d:%02d",
//          hour(),
//          minute(),
//          second()
//          );
//}
//
//void dateString()
//{
//  char_buffer[0] = '\0';
//  sprintf(char_buffer,
//          "%02d-%02d-%02d",
//          day(),
//          month(),
//          year()
//          );
//}
//

#if USE_MQTT
boolean connect_mqtt()
{
#if DEBUG
  debug(F("Connecting"));
#endif
  if (mqttClient.connect(mqtt_client_id))
  {
#if DEBUG
    debug(F("Connected"));
#endif
    publishConnected();

    // subscribe to topics
    mqttClient.subscribe("all/#");
    mqttClient.subscribe("weather/#");
    //      mqttClient.subscribe("relayduino/request/#");
    
    return true;
  }
  else
  {
    return false;
#if DEBUG
    debug(F("Failed"));
#endif
  }
}
#endif

/*--------------------------------------------------------------------------------------
  Init the LCD library with the LCD pins to be used
--------------------------------------------------------------------------------------*/
//Pins for the freetronics 16x2 LCD shield. LCD: ( RS, E, LCD-D4, LCD-D5, LCD-D6, LCD-D7 )
LiquidCrystal lcd( 8, 9, 4, 5, 6, 7 );   

boolean lcd_backlight_state()
{
  if ( digitalRead(LCD_BACKLIGHT_PIN) )
    return true;
  else
    return false;
}

void initialScreen()
{
  /*
    ------------------
    |  MQTT DISPLAY  |
    |   \|/    /|\   | <- use up and down arrows to change display
    ------------------
  */
#if LCD_PRINT
  //Print some initial text to the LCD.
  lcd.setCursor( 0, 0 );   //top left
  //          1234567890123456
  lcd.print( "  MQTT DISPLAY  " );
  //
  lcd.setCursor( 0, 1 );   //bottom left
  //          1234567890123456
  lcd.print( "   |    |   " );
#else
  strcpy_P(char_buffer, (char*)pgm_read_word(&(initial_screen_lines[0])));
  Serial.println(char_buffer);
  strcpy_P(char_buffer, (char*)pgm_read_word(&(initial_screen_lines[1])));
  Serial.println(char_buffer);
//  debug(F("------------------"));
//  debug(F("|  MQTT DISPLAY  |"));
//  debug(F("|   |    |       |"));
//  debug(F("------------------"));
#endif
}

// which initial_screen do not allow left and right buttons
boolean initial_screen = true;

/*--------------------------------------------------------------------------------------
Create structures to store sources
--------------------------------------------------------------------------------------*/

byte idx_previous(byte idx, byte idx_length)
{
  if ( idx == 0 )
    idx = idx_length - 1;
  else
    idx -= 1;
  return idx;
}

byte idx_next(byte idx, byte idx_length)
{
  if ( idx == ( idx_length - 1 ) )
    idx = 0;
  else
    idx += 1;
  return idx;
}

/*
line 1:  0000000000111111
line 2:  0123456789012345
        ------------------    ------------------
        |Display     Time|    |Display     Time|
        |  12:34:56 DST  |    |    12:34:56    |
        ------------------    ------------------
*/
void timeLcdLine()
{
  tmp_buffer[0] = '\0';
  if (DST)
  {
    sprintf(tmp_buffer, "  %02d:%02d:%02d %s  ", hour(), minute(), second(), "DST");
  }
  else
  {
    sprintf(tmp_buffer, "    %02d:%02d:%02d    ", hour(), minute(), second());
  }
#if LCD_PRINT  
  LCD_LINE(2)
  lcd.print(tmp_buffer);
#else
  Serial.println(tmp_buffer);
#endif
}

/*
line 1:  0000000000111111
line 2:  0123456789012345
        ------------------    ------------------
        |Display     Date|    |Display     Date|
        | Mon 30-06-2015 |    |   30-06-2015   |
        ------------------    ------------------
*/
void dateLcdLine()
{
  tmp_buffer[0] = '\0';
  sprintf(tmp_buffer, "   %02d-%02d-%02d   ", day(), month(), year());
#if LCD_PRINT  
  LCD_LINE(2)
  lcd.print(tmp_buffer);
#else
  Serial.println(tmp_buffer);
#endif
}

#if LCD_PRINT
void lcdClearLine(byte lineToClear)
{
  LCD_LINE(lineToClear);
  lcd.print("                ");
  LCD_LINE(lineToClear);
}

void printIp()
{
  lcdClearLine(2);
  lcd.setCursor( 2, 1 );
  for (int i = 0; i < 4; i++)
  {
    lcd.print(ip_address[i]);
    if (i < 3)
      lcd.print(".");
  }
}
#endif

void displayFieldsSwitch()
{
  switch ( display_fields_idx )
  {
    case 0 :
      dynamicDisplayRef = DISPLAY_TIME;
#if DEBUG
      debug(F("displayTime"));
#endif
      break;
    case 1 :
      dynamicDisplayRef = DISPLAY_DATE;
#if DEBUG
      debug(F("displayDate"));
#endif
      break;
    case 2 :
      dynamicDisplayRef = DISPLAY_STATIC;
#if LCD_PRINT
      printIp();
#endif
#if DEBUG
      debug(F("IP"));
#endif
      break;
    default :
      break;
  }
}

/*
  switches and displays the following display parameters
  display screen examples

line 1:  0000000000111111
line 2:  0123456789012345
        ------------------    ------------------
        |Display     Time|    |Display     Time|
        |  12:34:56 DST  |    |    12:34:56    |
        ------------------    ------------------
        ------------------
        |Display     Date|
        | Mon 30-06-2015 |
        ------------------
        ------------------
        |Display       IP|
        | 192.168.1.255  |
        ------------------
        ------------------    ------------------
        |Display       BL|    |Display       BL|
        |   Timer  ON    |    |   Timer OFF    |
        ------------------    ------------------
line 1: sprintf(char_buffer, "%7s %8s", prog_buffer_1, prog_buffer_2);
*/
void lcdDisplayFields(byte cmd)
{
  switch(cmd)
  {
    case PREVIOUS :
      display_fields_idx = idx_previous(display_fields_idx, display_fields_length );
      break;
    case START :
      display_fields_idx = 0;
      break;
    case NEXT :
      display_fields_idx = idx_next(display_fields_idx, display_fields_length );
      break;
  }
  // create string for line 1
  LCD_LINE(1);
  prog_buffer_2[0] = '\0';
  strcpy_P(prog_buffer_2, (char*)pgm_read_word(&(display_fields[display_fields_idx])));
#if DEBUG
  debug(F("prog_buffer_2: "));
  Serial.println(prog_buffer_2);
#endif
  char_buffer[0] = '\0';
  sprintf(char_buffer, "%7s %8s", prog_buffer_1, prog_buffer_2);  // note prog_buffer_1 already printed should not do again.
#if LCD_PRINT
  lcd.print(char_buffer);
#else
  debug(F("char_buffer: "));
  Serial.println(char_buffer);
#endif
  // create string for line 2
  LCD_LINE(2);
  displayFieldsSwitch();
}

/*
  switches and displays the following weather parameters
  weather screen examples

line 1:  0000000000111111
line 2:  0123456789012345
        ------------------
        |Weather  @ 12:34|
        |Temp:   18.6  oC|
        ------------------
        ------------------
        |Weather  @ 12:34|
        |Temp:  18.6   oC|
        ------------------
        ------------------
        |Weather  @ 12:34|
        |Hum:   18.6    %|
        ------------------
        ------------------
        |Weather  @ 12:34|
        |Pres:  1024  mPa|
        ------------------
         ------------------
        |Weather  @ 12:34|
        |Light:   11     |
        ------------------
       ------------------
        |Weather  @ 12:34|
        |W-Dir:   SW   mm|
        ------------------
        ------------------
        |Weather  @ 12:34|
        |W-Spd:    5  m/s|
        ------------------
        ------------------
        |Weather  @ 12:34|
        |Rain:     5   mm|
        ------------------
        ------------------
        |Weather  @ 12:34|
        |Pwr:    450   mW|
        ------------------
line 1: sprintf(prog_buffer_2, "%1s %5s", "@", weather_report_time);
line 1: sprintf(char_buffer, "%7s %8s", prog_buffer_1, prog_buffer_2);
line 2: sprintf(char_buffer, "%5s %5s %4s", prog_buffer_1, weather_report_values[weather_fields_idx], prog_buffer_2 );
*/
void lcdWeatherFields(byte cmd)
{
  switch(cmd)
  {
    case PREVIOUS :
      weather_fields_idx = idx_previous(weather_fields_idx, weather_fields_length );
      break;
    case START :
      weather_fields_idx = 0;
      break;
    case NEXT :
      weather_fields_idx = idx_next(weather_fields_idx, weather_fields_length );
      break;
    case SAME :
      weather_fields_idx = weather_fields_idx;
    default :
      break;
  }
  // create string for line 1
  lcdClearLine(1);
  char_buffer[0] = '\0';
  if (weather_report_new)
  {
    prog_buffer_2[0] = '\0';
    sprintf(prog_buffer_2, "%1s %5s", "@", weather_report_time);
#if DEBUG
    Serial.println(prog_buffer_2);
#endif
    sprintf(char_buffer, "%7s %8s", prog_buffer_1, prog_buffer_2);  // note prog_buffer_1 already printed should not do again.
  }
  else
  {
    sprintf(char_buffer, "%7s", prog_buffer_1);
  }
#if LCD_PRINT
  lcd.print(char_buffer);
#else
  Serial.println(char_buffer);
#endif
  // create string for line 2
  LCD_LINE(2);
  if (weather_report_new)
  {
    prog_buffer_1[0] = '\0';
    strcpy_P(prog_buffer_1, (char*)pgm_read_word(&(weather_fields_labels[weather_fields_idx])));
#if DEBUG
    debug(F("prog_buffer_1: "));
    Serial.println(prog_buffer_1);
#endif
    prog_buffer_2[0] = '\0';
    strcpy_P(prog_buffer_2, (char*)pgm_read_word(&(weather_fields_units[weather_fields_idx])));
#if DEBUG
    debug(F("prog_buffer_2: "));
    Serial.println(prog_buffer_2);
#endif
    char_buffer[0] = '\0';
    sprintf(char_buffer, "%5s %5s %4s", prog_buffer_1, weather_report_values[weather_fields_idx], prog_buffer_2 );
#if LCD_PRINT
    lcd.print(char_buffer);
#else
    Serial.println(char_buffer);
#endif
  }
  else {
#if LCD_PRINT
    lcd.print(" No values set! ");
#else
    Serial.println(char_buffer);
#endif
  }
}

void switchParameters(byte cmd)
{
  prog_buffer_1[0] = '\0';
  strcpy_P(prog_buffer_1, (char*)pgm_read_word(&(sources[sources_idx])));
  switch ( sources_idx )
  {
    case 0 :  // display
      lcdDisplayFields(cmd);
      break;
    case 1 :  // weather
      lcdWeatherFields(cmd);
      break;
  }
}

void switchSources(byte cmd)
{
  switch (cmd)
  {
    case PREVIOUS :
      sources_idx = idx_previous( sources_idx, sources_length );
      break;
    case NEXT :
      sources_idx = idx_next( sources_idx, sources_length );
      break;
    default :
      break;
  }
  // print source on LHS of first LCD row
  LCD_LINE(1);
  prog_buffer_1[0] = '\0';
  strcpy_P(prog_buffer_1, (char*)pgm_read_word(&(sources[sources_idx])));
#if LCD_PRINT
  lcd.print(prog_buffer_1);
#else
  Serial.println(prog_buffer_1);
#endif
  switch ( sources_idx )
  {
    case 0 :    // display
      lcdDisplayFields(0);
      break;
    case 1 :    // weather
//      dynamicDisplayRef = DISPLAY_WTHR;
      dynamicDisplayRef = DISPLAY_STATIC;
      lcdWeatherFields(0);
      break;
    default :
      break;
  }
}


/*--------------------------------------------------------------------------------------
setup()
Called by the Arduino framework once, before the main loop begins
--------------------------------------------------------------------------------------*/
void setup()
{
#if ( DEBUG || !LCD_PRINT )
  Serial.begin(BAUD_RATE);
#endif

  //button adc input
  pinMode( BUTTON_ADC_PIN, INPUT );         //ensure A0 is an input
  digitalWrite( BUTTON_ADC_PIN, LOW );      //ensure pullup is off on A0
  //lcd backlight control
  digitalWrite( LCD_BACKLIGHT_PIN, HIGH );  //backlight control pin D3 is high (on)
  pinMode( LCD_BACKLIGHT_PIN, OUTPUT );     //D3 is an output
  //set up the LCD number of columns and rows:

#if LCD_PRINT
  lcd.begin( LINE_LENGTH, NUM_OF_LINES );
#else
  LCD_BACKLIGHT_OFF();
#endif

  initialScreen();

  Ethernet.begin(mac_address, ip_address);

  IPAddress ip_addr_set = Ethernet.localIP();
  if (ip_addr_set[0] == 0)
  {
#if DEBUG    
    debug(F("IP address appears to be set incorrectly"));
    debug(F("Exiting"));
#endif    
    return;  // not worth continuing - should reset after delay
  }

#if DEBUG    
  debug(F("IP: "));
  Serial.println(Ethernet.localIP());
#endif

#if USE_MQTT
  if (connect_mqtt())
  {
    LCD_LINE(2);
    lcd.print(" MQTT Connected ");
  }
#endif

  Udp.begin(localPort);
  
#if DEBUG    
  debug(F("waiting for sync"));
#endif
  setSyncProvider(getNtpTime);
  
// automatically switch to time view if time was set
//  if (timeStatus() != timeNotSet)
//    switchParameters(0);;

  button_last_pressed = millis();
}


/*--------------------------------------------------------------------------------------
loop()
Arduino main loop
--------------------------------------------------------------------------------------*/
void loop()
{
#if USE_MQTT
  mqttClient.loop();
#endif

  // switch backlight off if on for longer than LCD_BACKLIGHT_DURATION
  if(millis() - button_last_pressed >= LCD_BACKLIGHT_DURATION)
  {
    LCD_BACKLIGHT_OFF();
  }   

  // handle display of dynamic elements (time and date) only if backlight is on
  if (lcd_backlight_state())
  {
    if (dynamicDisplayRef == DISPLAY_TIME)
    {
      if (timeStatus() != timeNotSet) {
        if (now() != prevDispTime) { //update the display only if time has changed
          prevDispTime = now();
          timeLcdLine();
        }
      }
      else {
        LCD_LINE(2);
#if LCD_PRINT
      lcd.print(" Time not set!! ");
#else     
      debug(F("Time not set!!"));
#endif
      }
    }
    else if (dynamicDisplayRef == DISPLAY_DATE)
    {
      if (timeStatus() != timeNotSet) {
        if (now() != prevDispTime) { //update the display only if time has changed
          prevDispTime = now();
          dateLcdLine();
        }
      }
      else {
#if LCD_PRINT
      LCD_LINE(2);
      lcd.print(" Date not set!! ");
#else      
      debug(F("Date not set!!"));
#endif
      }
    }
    else if (dynamicDisplayRef == DISPLAY_WTHR)
    {
      lcdWeatherFields(SAME);
      weather_report_new = false;
    }
  }

  byte button = ReadButtons();

  if ( buttonJustPressed )
  {
    button_last_pressed = millis();
    LCD_BACKLIGHT_ON();
    switch ( button )
    {
      case BUTTON_NONE :
        break;
      case BUTTON_UP :
        if (initial_screen) initial_screen = false;
          switchSources(PREVIOUS);
        break;
      case BUTTON_DOWN :
        if (initial_screen) initial_screen = false;
          switchSources(NEXT);
        break;
      case BUTTON_RIGHT :
        if (!initial_screen)
          switchParameters(NEXT);
        break;
      case BUTTON_LEFT :
        if (!initial_screen)
          switchParameters(PREVIOUS);
        break;
      case BUTTON_SELECT :
        break;
      default :
        break;
    }
  }

  //clear the buttonJustPressed or buttonJustReleased flags, they've already done their job now.
  if ( buttonJustPressed )
    buttonJustPressed = false;
  if ( buttonJustReleased )
    buttonJustReleased = false;
}

