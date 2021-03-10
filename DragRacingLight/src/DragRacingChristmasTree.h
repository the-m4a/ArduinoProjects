#include <Arduino.h>
#include <IRremote.h>

// There's different kinds of modes
// https://timeslipcharts.com/drag-racing-christmas-tree
// Prestage (top light)
// Stage (2nd light)  -- technically, this lights up once you're at the right "spot", but this will just be timer thing
// Yellow -- a "pro" tree lights up all at once, a "sportsman" or "full" tree does them in sequence (top, mid, bot yellow, then green)
// Yellow
// Yellow
// Green
// Red (for faults... but we won't have that here)

// Just guessing on what to set this value
// Just so I don't have to type "unsigned long" all over the joint
#define ULONG unsigned long

// Pin Values
#define PRESTAGE_LED_PIN      12
#define STAGE_LED_PIN         11
#define BREADBOARD_BUTTON_PIN 10
#define RECEIVER_PIN          9
#define YELLOW_TOP_LED_PIN    8
#define YELLOW_MID_LED_PIN    7
#define YELLOW_BOT_LED_PIN    6
#define GREEN_LED_PIN         5

// These should actually be white, not Blue
// The amount of resistance needs to change based on the actual number of LEDs here -- electricity stuff
#define PRESTAGE_LED        0x1
#define STAGE_LED           0x2
#define YELLOW_TOP_LED      0x4
#define YELLOW_MID_LED      0x8
#define YELLOW_BOT_LED      0x10
#define YELLOW_ALL_LED      YELLOW_TOP_LED|YELLOW_MID_LED|YELLOW_BOT_LED
#define GREEN_LED           0x20

// Values from the remote that goes with that broken heater
#define REMOTE_HEATER_BUTTON_POWER     0x8166817E
#define REMOTE_HEATER_BUTTON_UP        0x8166A15E
#define REMOTE_HEATER_BUTTON_DOWN      0x816651AE
#define REMOTE_HEATER_BUTTON_FANSPEED  0x81669966
#define REMOTE_HEATER_BUTTON_TIMER     0x8166F906
#define REMOTE_HEATER_BUTTON_MODE      0x8166D926
#define REMOTE_REPEATED_KEY            0xFFFFFFFF  // This is the value sent if the same key is just held down

// "Friendly name" for each of the buttons
#define REMOTE_BUTTON_RESET            REMOTE_HEATER_BUTTON_POWER // Just for reset to "ready"
#define REMOTE_BUTTON_PRO_TREE_4       REMOTE_HEATER_BUTTON_DOWN // "Pro Tree" where all 3 yellow lights illuminate for 0.4 seconds
#define REMOTE_BUTTON_PRO_TREE_5       REMOTE_HEATER_BUTTON_UP // Pro tree where all 3 yellow lights illuminate for 0.5 seconds
#define REMOTE_BUTTON_FULL_TREE        REMOTE_HEATER_BUTTON_FANSPEED // "Full tree" Each yellow light illuminates for 0.5 seconds, in sequence
#define REMOTE_BUTTON_PRO_TREE_RANDOM  REMOTE_HEATER_BUTTON_TIMER // Pro tree where all 3 yellow lights illuminate for random between 0.370 and 1 seconds
#define REMOTE_BUTTON_FULL_TREE_RANDOM REMOTE_HEATER_BUTTON_MODE // Full tree where the 3 yellow lights illuminate in sequence, some value between 1 and 3 seconds (divided  by 3... so if 1 second, then each light is on for 0.333 seconds)

// Hardware button installed to the breadboard
#define DEBOUNCE_TIME       200
#define BREADBOARD_BUTTON   REMOTE_BUTTON_RESET // Behave the same as the reset button on the remote

// Timing values (in milliseconds) for various light-related things
#define DELAY_PRESTAGE_LIGHT       5000
#define DELAY_STAGE_LIGHT_TIME_MIN 2000
#define DELAY_STAGE_LIGHT_TIME_MAX 5000
#define DELAY_PRO_TREE_4            400
#define DELAY_PRO_TREE_5            500
#define DELAY_PRO_TREE_RANDOM_MIN   370 // 0.370 is the NHRA Drag racing time
#define DELAY_PRO_TREE_RANDOM_MAX  1000 // Arbitrary max
#define DELAY_FULL_TREE            1500 // Will be divided by 3
#define DELAY_FULL_TREE_RANDOM_MIN 1000 // arbitrary min, would be 0.333 per light
#define DELAY_FULL_TREE_RANDOM_MAX 3000 // arbitrary max, 1 sec per light
#define DELAY_GREEN_LIGHT          7000 // show the Green light for 7 seconds

// Code readability
#define NO_BUTTON_PRESSED    0

// Because CARS and I like nerdy things
#define DELAY_YELLOW_BLINK_REGULAR 1500
#define DELAY_YELLOW_BLINK_SLOWER  1850
#define DELAY_YELLOW_BLINK_OFF     1500

void  TempPrintButtonTappedState(ULONG buttonTapped, int callSpot);
void  LED_On(int ledMask);
void  LED_Off(int ledMask);
void  LED_Control(int ledMask, int ledState);
void  LED_OnOff_Delay(int ledMask, ULONG delayMS);
ULONG LED_OnOff_DelayAndButtonCheck(int ledMask, ULONG delayMS);
void  LED_AllOff();
ULONG DelayAndButtonCheck(ULONG delayTime);
ULONG DoProTreeSequence(ULONG yellowLightLength);
ULONG DoFullTreeSequence(ULONG yellowLightLength);

// Hardware button installed to the breadboard
int _buttonState; // Tracking for main button state on the board
int _buttonPreviousState; // Tracking for previous button state (debouncing for timer)
long _buttonLastPressTime; // Tracking the last timestamp the button was pressed

// IR Receiver declarations
IRrecv _receiver(RECEIVER_PIN); // create a receiver object of the IRrecv class
decode_results _results; // create a results object of the decode_results class
ULONG _rcv_key_value; // variable to store the pressed key value

 // Loop iterator counter for the slow 3rd blink (for reasons)
int _loopIter;