#ifndef __CONFIG_H__
#define __CONFIG_H__


#define DEBUG                     false
#define LCD_PRINT                 true
#define USE_MQTT                  true


#if ( DEBUG || !LCD_PRINT )
// Serial parameters
const int BAUD_RATE               = 9600;
#endif


// network details
IPAddress ip_address( 192, 168, 1, 56 );
// the dns server ip
//IPAddress dnServer(192, 168, 1, 254);
// the router's gateway address:
//IPAddress gateway(192, 168, 1, 254);
// the subnet:
//IPAddress subnet(255, 255, 255, 0);
byte mac_address[]                = { 0xDE, 0xED, 0xBA, 0xFE, 0xFE, 0xED };


// MQTT parameters
//byte broker_ip_addr[]             = { 192, 168, 1, 30 };    // Airology
byte mqtt_server_addr[]           = { 192, 168, 1, 55 };    // Pi
const uint16_t mqtt_port          = 1883;
char mqtt_client_id[]             = "lcdDisplay";


// time parameters
// Offset hours from gps time (UTC)
const byte TZ_OFFSET_HOURS        = 9;   // Australian CST + 30 MINS (+10 if DST)
const byte TZ_OFFSET_MINUTES      = 30;
bool DST                          = false;
//const int DST_OFFSET_HOURS        = 1;

unsigned long button_last_pressed = 0;              // stores time (milliseconds) when a button was last pressed
const unsigned int LCD_BACKLIGHT_DURATION = 30000;  // milliseconds

const byte LINE_LENGTH            = 16;
const byte NUM_OF_LINES           = 2;
char char_buffer[LINE_LENGTH + 1];
char prog_buffer_1[LINE_LENGTH + 1];
char prog_buffer_2[LINE_LENGTH + 1];
char tmp_buffer[LINE_LENGTH + 1];

#define PREVIOUS   -1
#define START 0
#define NEXT  1
#define SAME  2

#define DISPLAY_STATIC 0
#define DISPLAY_TIME 1
#define DISPLAY_DATE 2
#define DISPLAY_WTHR 3

byte dynamicDisplayRef            = DISPLAY_STATIC;

const byte sources_length = 2;
const byte display_fields_length = 3;
const byte weather_fields_length = 2;

byte sources_idx = 0;
byte display_fields_idx = 0;
byte weather_fields_idx = 0;

//boolean weather_fields_set = false;
boolean weather_report_new = false;

char weather_report_temp[5];
char weather_report_hum[5];
//char weather_report_wspd[5];
//char weather_report_wdir[5];
//char weather_report_rain[5];

char *weather_report_values[] = {weather_report_temp, weather_report_hum};

//char weather_report_time[] = "12:34";
char weather_report_time[6];



const char initial_screen_line1[]   PROGMEM = "  MQTT DISPLAY  ";
const char initial_screen_line2[]   PROGMEM = "     |    |     ";

PGM_P const initial_screen_lines[]  PROGMEM = {initial_screen_line1,  // idx = 0
                                               initial_screen_line2,  // idx = 1
                                               };
                                               
const char source_display_label[]   PROGMEM = "Display";
const char source_weather_label[]   PROGMEM = "Weather";

PGM_P const sources[]               PROGMEM = {source_display_label,  // idx = 0
                                               source_weather_label,  // idx = 1
                                               };

const char display_time_label[]     PROGMEM = "Time";
const char display_date_label[]     PROGMEM = "Date";
const char display_ip_label[]       PROGMEM = "IP";

PGM_P const display_fields[]        PROGMEM = {display_time_label,    // idx = 0
                                               display_date_label,    // idx = 1
                                               display_ip_label,      // idx = 2
                                               };

// note spaces ensure labels are aligned correctly (alignment to right)
// 5 characters are printed
const char weather_temp_label[]     PROGMEM = "Temp ";
const char weather_humidity_label[] PROGMEM = "Hum  ";
//const char weather_pressure_label[] PROGMEM = "Pres ";
//const char weather_light_label[]    PROGMEM = "Light";
//const char weather_wind_dir_label[] PROGMEM = "W-Dir";
//const char weather_wind_spd_label[] PROGMEM = "W-Spd";
//const char weather_rain_label[]     PROGMEM = "Rain ";
//const char weather_power_label[]    PROGMEM = "Pwr  ";

PGM_P const weather_fields_labels[] PROGMEM = {weather_temp_label,    // idx = 0
                                               weather_humidity_label,// idx = 1
                                               //weather_pressure_label,// idx = 2
                                               //weather_light_label,
                                               //weather_wind_dir_label,
                                               //weather_wind_spd_label,
                                               //weather_rain_label,
                                               //weather_power_label,   // idx = 3
                                               };

const char weather_temp_unit[]      PROGMEM = "oC";
const char weather_humidity_unit[]  PROGMEM = "%";
//const char weather_pressure_unit[]  PROGMEM = "mPa";
//const char weather_light_unit[]     PROGMEM = "   ";
//const char weather_wind_dir_unit[]  PROGMEM = "   ";
//const char weather_wind_spd_unit[]  PROGMEM = "m/s";
//const char weather_rain_unit[]      PROGMEM = "mm";
//const char weather_power_unit[]     PROGMEM = "mW";

PGM_P const weather_fields_units[]  PROGMEM = {weather_temp_unit,     // idx = 0
                                               weather_humidity_unit, // idx = 1
                                               //weather_pressure_unit, // idx = 2
                                               //weather_power_unit,    // idx = 3
                                               };

const char topic_status_ethernet[]  PROGMEM = "lcddisplay/status/ethernet";
const char topic_status_time[]      PROGMEM = "lcddisplay/status/time";

PGM_P const status_topics[]         PROGMEM = {topic_status_ethernet, // idx = 0
                                               topic_status_time,     // idx = 1
                                               };
                                        
/*
  Configuration code for the Freetronics LCD & Keypad Shield:

    http://www.freetronics.com/products/lcd-keypad-shield

  by Marc Alexander, 7 September 2011
  This example code is in the public domain.

Pins used by LCD & Keypad Shield:
  
    A0: Buttons, analog input from voltage ladder
    D4: LCD bit 4
    D5: LCD bit 5
    D6: LCD bit 6
    D7: LCD bit 7
    D8: LCD RS
    D9: LCD E
    D3: LCD Backlight (high = on, also has pullup high so default is on)
  
  ADC voltages for the 5 buttons on analog input pin A0:
  
    RIGHT:  0.00V :   0 @ 8bit ;   0 @ 10 bit
    UP:     0.71V :  36 @ 8bit ; 145 @ 10 bit
    DOWN:   1.61V :  82 @ 8bit ; 329 @ 10 bit
    LEFT:   2.47V : 126 @ 8bit ; 505 @ 10 bit
    SELECT: 3.62V : 185 @ 8bit ; 741 @ 10 bit
*/

/*--------------------------------------------------------------------------------------
  Defines
--------------------------------------------------------------------------------------*/
// Pins in use
#define BUTTON_ADC_PIN           A0  // A0 is the button ADC input
#define LCD_BACKLIGHT_PIN         3  // D3 controls LCD backlight
// ADC readings expected for the 5 buttons on the ADC input
#define RIGHT_10BIT_ADC           0  // right
#define UP_10BIT_ADC            145  // up
#define DOWN_10BIT_ADC          329  // down
#define LEFT_10BIT_ADC          505  // left
#define SELECT_10BIT_ADC        741  // right
#define BUTTONHYSTERESIS         10  // hysteresis for valid button sensing window
//return values for ReadButtons()
#define BUTTON_NONE               0  // 
#define BUTTON_RIGHT              1  // 
#define BUTTON_UP                 2  // 
#define BUTTON_DOWN               3  // 
#define BUTTON_LEFT               4  // 
#define BUTTON_SELECT             5  // 
//some example macros with friendly labels for LCD backlight/pin control, tested and can be swapped into the example code as you like
#define LCD_BACKLIGHT_OFF()     digitalWrite( LCD_BACKLIGHT_PIN, LOW )
#define LCD_BACKLIGHT_ON()      digitalWrite( LCD_BACKLIGHT_PIN, HIGH )
#define LCD_BACKLIGHT(state)    { if( state ){digitalWrite( LCD_BACKLIGHT_PIN, HIGH );}else{digitalWrite( LCD_BACKLIGHT_PIN, LOW );} }

#define LCD_LINE(line_no)      { lcd.setCursor( 0, line_no-1 ); }

/*--------------------------------------------------------------------------------------
  Variables
--------------------------------------------------------------------------------------*/
byte buttonJustPressed        = false;         //this will be true after a ReadButtons() call if triggered
byte buttonJustReleased       = false;         //this will be true after a ReadButtons() call if triggered
byte buttonWas                = BUTTON_NONE;   //used bybrew update ReadButtons() for detection of button events

/*--------------------------------------------------------------------------------------
  ReadButtons()
  Detect the button pressed and return the value
  Uses global values buttonWas, buttonJustPressed, buttonJustReleased.
--------------------------------------------------------------------------------------*/
byte ReadButtons()
{
   unsigned int buttonVoltage;
   byte button = BUTTON_NONE;   // return no button pressed if the below checks don't write to btn
   
   //read the button ADC pin voltage
   buttonVoltage = analogRead( BUTTON_ADC_PIN );
   //sense if the voltage falls within valid voltage windows
   if( buttonVoltage < ( RIGHT_10BIT_ADC + BUTTONHYSTERESIS ) )
   {
      button = BUTTON_RIGHT;
   }
   else if(   buttonVoltage >= ( UP_10BIT_ADC - BUTTONHYSTERESIS )
           && buttonVoltage <= ( UP_10BIT_ADC + BUTTONHYSTERESIS ) )
   {
      button = BUTTON_UP;
   }
   else if(   buttonVoltage >= ( DOWN_10BIT_ADC - BUTTONHYSTERESIS )
           && buttonVoltage <= ( DOWN_10BIT_ADC + BUTTONHYSTERESIS ) )
   {
      button = BUTTON_DOWN;
   }
   else if(   buttonVoltage >= ( LEFT_10BIT_ADC - BUTTONHYSTERESIS )
           && buttonVoltage <= ( LEFT_10BIT_ADC + BUTTONHYSTERESIS ) )
   {
      button = BUTTON_LEFT;
   }
   else if(   buttonVoltage >= ( SELECT_10BIT_ADC - BUTTONHYSTERESIS )
           && buttonVoltage <= ( SELECT_10BIT_ADC + BUTTONHYSTERESIS ) )
   {
      button = BUTTON_SELECT;
   }
   //handle button flags for just pressed and just released events
   if( ( buttonWas == BUTTON_NONE ) && ( button != BUTTON_NONE ) )
   {
      //the button was just pressed, set buttonJustPressed, this can optionally be used to trigger a once-off action for a button press event
      //it's the duty of the receiver to clear these flags if it wants to detect a new button change event
      buttonJustPressed  = true;
      buttonJustReleased = false;
   }
   if( ( buttonWas != BUTTON_NONE ) && ( button == BUTTON_NONE ) )
   {
      buttonJustPressed  = false;
      buttonJustReleased = true;
   }
   
   //save the latest button value, for change event detection next time round
   buttonWas = button;
   
   return( button );
}

#endif

