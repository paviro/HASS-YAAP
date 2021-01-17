// !!! ------------------------------------ !!!
// COPY THIS FILE TO A NEW FILE NAMED: config.h
// !!! ------------------------------------ !!!


// ------------------------------------ 
// Language
// Available: EN.h, DE.h, NL.h
// ------------------------------------ 

// Use one of the available files or create a new one
#include "translations/EN.h"



// ------------------------------------ 
// WLAN & MQTT Configuration
// ------------------------------------ 

#define WiFi_SSID              "WiFi_SSID"
#define WiFi_PW                "WiFi_PW"
#define MQTT_SERVER_IP         "192.168.x.y"
#define MQTT_USER              "MQTT_USER"
#define MQTT_PW                "MQTT_PW"
#define MQTT_CLIENT_NAME       "AlarmControlPanel"



// ------------------------------------ 
// Timeouts
// ------------------------------------ 

// Timeout for switching back to the main state screen
#define DEFAULT_TIMEOUT         15
// Backlight timeout when on main state screen
#define BACKLIGHT_TIMEOUT       10
// How long to display the names of people coming home
#define WELCOME_MESSAGE_TIMEOUT 60

// Delay before scrolling to the next position in ms (used when the names for the welcome screen are too long)
#define TEXT_SCROLL_SPEED_DELAY 350



// ------------------------------------ 
// LCD Configuration
// ------------------------------------ 

// I2C address of LCD screen
#define SCREEN_ADDRESS          0x27

// LCD size - using a different size will cause UI errors for now
#define SCREEN_WIDTH            20
#define SCREEN_HEIGHT           4

// true = show entered pin - false = replace with *
#define SHOW_PIN                true



// ------------------------------------ 
// Keypad Configuration
// ------------------------------------

// Number of rows and cols on the keypad
#define KEYPAD_ROWS             4
#define KEYPAD_COLS             4

// GPIO Pins of keypad rows and cols
#define KEYPAD_ROW_PINS         D4, D5, D6, D7
#define KEYPAD_COL_PINS         D0, D9, D10, D3

// Keypad layout (Keypad with letters needed for using this code!)
char keypadMap[KEYPAD_ROWS][KEYPAD_COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};