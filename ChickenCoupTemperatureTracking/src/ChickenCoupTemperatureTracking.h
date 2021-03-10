// Use PlatformIO to include the required libaries -- see the platformio.ini file
// Include the libraries:
// Connection pin for the Temperature Sensor things
#include <TM1637Display.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <SoftwareSerial.h>
#include "pitches.h"

// Pins for the Temperature reader
#define DHTPIN 2
#define DHTTYPE DHT11

// Buzzer connection
#define BUZZERPIN 3

// Pins for the 7-seg display
#define CLK 5
#define DIO 6

// For the Serial connection with the Wifi-duino thing
#define ENDBUFFERCHAR 10

// 7 Segment Display values
// AAAA
// F  B
// GGGG
// E  C
// DDDD

// Create degree Celsius symbol:
const uint8_t celsiussymbol[] =
{
    // SEG_A | SEG_B | SEG_F | SEG_G, // Circle
    SEG_A | SEG_D | SEG_E | SEG_F // C
};

// Create degree Fahrenheit symbol:
const uint8_t fahrenheitsymbol[] =
{
    // SEG_A | SEG_B | SEG_F | SEG_G, // Circle
    SEG_A | SEG_E | SEG_F | SEG_G // F
};

const uint8_t humiditysymbol[] =
{
    SEG_F | SEG_E | SEG_B | SEG_C | SEG_G, 0x0, 0x0, 0x0
};

const uint8_t dashessymbol[] =
{
    SEG_G, SEG_G, SEG_G, SEG_G
};

const uint8_t errsymbol[] =
{
    0x0                                  , // blank
    SEG_A | SEG_F | SEG_G | SEG_E | SEG_D, // E
    SEG_E | SEG_G                        , // r
    SEG_E | SEG_G                          // r
};

// Function declarations
void soundAlarm();

// Global variable declarations
int _temperature_celsius;
int _temperature_fahrenheit;
int _humidity;
int _iteration;
int _alarmIterations;

// Software Serial for communication with the Wifi NodeMCU
SoftwareSerial _mySerial(7,8); // RX, TX

// 4 digit 7-segment display
TM1637Display _display = TM1637Display(CLK, DIO);

// Temperature/Humidity sensor
DHT _dht = DHT(DHTPIN, DHTTYPE);
