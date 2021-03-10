
// Drag Racing "Christmas Tree" - using an old remote that worked with the IR receiver on the Arduino
// Buttons on the remote control if wanting to use "Pro" (all 3 yellows at once) or "Full" (each yellow individually)
// Configured the Arduino to just do a serial print of the value received when a button is pushed on the remote.
// Once you have all of the values you can then assign them to do something.
#include "DragRacingChristmasTree.h"

void setup()
{
    Serial.begin(9600);

    _loopIter = 0;
    _buttonState = LOW;
    _buttonPreviousState = LOW;
    _buttonLastPressTime = 0;
    _rcv_key_value = 0; // variable to store the pressed key value
    _receiver.enableIRIn();
    _receiver.blink13(true);

    pinMode(BREADBOARD_BUTTON_PIN, INPUT);
    pinMode(RECEIVER_PIN, INPUT);
    pinMode(PRESTAGE_LED_PIN, OUTPUT);
    pinMode(STAGE_LED_PIN, OUTPUT);
    pinMode(YELLOW_TOP_LED_PIN, OUTPUT);
    pinMode(YELLOW_MID_LED_PIN, OUTPUT);
    pinMode(YELLOW_BOT_LED_PIN, OUTPUT);
    pinMode(GREEN_LED_PIN, OUTPUT);

    randomSeed(analogRead(0));

    // sanity check delay - allows reprogramming if accidentally blowing power w/LEDs
    delay(2000);

    LED_OnOff_Delay(PRESTAGE_LED, 200);
    LED_OnOff_Delay(STAGE_LED, 200);
    LED_OnOff_Delay(YELLOW_TOP_LED, 200);
    LED_OnOff_Delay(YELLOW_MID_LED, 200);
    LED_OnOff_Delay(YELLOW_BOT_LED, 200);
    LED_OnOff_Delay(GREEN_LED, 200);
    LED_AllOff();

    Serial.println("Setup completed");
}

void loop()
{
    ULONG buttonTapped = 0;
    long randomLightDelay = 0;

    _loopIter++;

    // Just show a slow blinking Yellow light in "ready" mode
    LED_On(YELLOW_MID_LED);

    // Every 3rd blink is slower because Cars is a great movie and I will not be taking questions about this, only praise about how this is awesome
    if (_loopIter % 3 != 0)
    {
        buttonTapped = DelayAndButtonCheck(DELAY_YELLOW_BLINK_REGULAR);
    }
    else
    {
        buttonTapped = DelayAndButtonCheck(DELAY_YELLOW_BLINK_SLOWER);
        _loopIter = 0;
    }

    TempPrintButtonTappedState(buttonTapped, 0);

    LED_Off(YELLOW_MID_LED);
    if (buttonTapped == NO_BUTTON_PRESSED)
    {
        // Wasn't tapped so do the blink
        buttonTapped = DelayAndButtonCheck(DELAY_YELLOW_BLINK_OFF);
        TempPrintButtonTappedState(buttonTapped, 1);
    }

    // do that only runs once just so we can use the break in case the button is tapped
    do
    {
        if (buttonTapped != NO_BUTTON_PRESSED && buttonTapped != REMOTE_BUTTON_RESET)
        {
            // A button has been pressed! record which one and start the sequence
            ULONG treeSequenceSelected = buttonTapped;

            LED_AllOff();

            // Turn on the Top LED... delay 3 seconds? dunno.
            LED_On(PRESTAGE_LED);
            buttonTapped = DelayAndButtonCheck(DELAY_PRESTAGE_LIGHT);

            TempPrintButtonTappedState(buttonTapped, 2);
            if (buttonTapped == REMOTE_BUTTON_RESET)
                break;

            randomLightDelay = random(DELAY_STAGE_LIGHT_TIME_MIN, DELAY_STAGE_LIGHT_TIME_MAX);
            Serial.print("Stage light will show for this many ms: ");
            Serial.println(randomLightDelay);
            LED_On(STAGE_LED);
            buttonTapped = DelayAndButtonCheck(randomLightDelay);

            TempPrintButtonTappedState(buttonTapped, 3);
            if (buttonTapped == REMOTE_BUTTON_RESET)
                break;

            LED_AllOff();
            switch(treeSequenceSelected)
            {
                case REMOTE_BUTTON_PRO_TREE_4:
                    buttonTapped = DoProTreeSequence(DELAY_PRO_TREE_4);
                    break;

                case REMOTE_BUTTON_PRO_TREE_5:
                    buttonTapped = DoProTreeSequence(DELAY_PRO_TREE_5);
                    break;

                case REMOTE_BUTTON_PRO_TREE_RANDOM:
                    randomLightDelay = random(DELAY_PRO_TREE_RANDOM_MIN, DELAY_PRO_TREE_RANDOM_MAX);
                    Serial.print("Pro Tree light will show for this many ms: ");
                    Serial.println(randomLightDelay);
                    buttonTapped = DoProTreeSequence(randomLightDelay);
                    break;

                case REMOTE_BUTTON_FULL_TREE:
                    buttonTapped = DoFullTreeSequence(DELAY_FULL_TREE);
                    break;

                case REMOTE_BUTTON_FULL_TREE_RANDOM:
                    randomLightDelay = random(DELAY_FULL_TREE_RANDOM_MIN, DELAY_FULL_TREE_RANDOM_MAX);
                    Serial.print("Full Tree light will show for this many ms total: ");
                    Serial.print(randomLightDelay);
                    Serial.print(" -- each light: ");
                    Serial.println(randomLightDelay / 3);
                    buttonTapped = DoFullTreeSequence(randomLightDelay);
                    break;

                default:
                    // What was this button?  Just pretend it's the reset button
                    buttonTapped = REMOTE_BUTTON_RESET;
                    break;
            }

            if (buttonTapped != REMOTE_BUTTON_RESET)
            {
                LED_On(GREEN_LED);

                // Leave it green for the defined number of seconds, but let the button reset that if desired
                buttonTapped = DelayAndButtonCheck(DELAY_GREEN_LIGHT);
                TempPrintButtonTappedState(buttonTapped, 8);
            }
        }

        break;
    } while (true); // the break immediately above this means it will have run only once

    if (buttonTapped == REMOTE_BUTTON_RESET)
    {
        LED_AllOff();
        for (int i = 0; i < 3; i++)
        {
            LED_OnOff_Delay(PRESTAGE_LED|STAGE_LED, 200);
            LED_OnOff_Delay(YELLOW_ALL_LED, 200);
            LED_OnOff_Delay(GREEN_LED, 200);
        }
    }

    LED_AllOff();
}

// Do the Yellow and for the "Pro" tree
ULONG DoProTreeSequence(ULONG yellowLightLength)
{
    Serial.print("*** Pro Tree Sequence:  yellowLightLength:");
    Serial.println(yellowLightLength);
    ULONG buttonTapped = NO_BUTTON_PRESSED;
    // Show the Yellow LEDs
    buttonTapped = LED_OnOff_DelayAndButtonCheck(YELLOW_ALL_LED, yellowLightLength);
    TempPrintButtonTappedState(buttonTapped, 4);

    return buttonTapped;
}

// Do the Yellow for the "Full" tree 
ULONG DoFullTreeSequence(ULONG yellowLightLength)
{
    ULONG yellowIndividualLightLength = yellowLightLength / 3;
    ULONG buttonTapped = NO_BUTTON_PRESSED;

    Serial.print("*** Full Tree Sequence: total yellowLightLength:");
    Serial.print(yellowLightLength);
    Serial.print("  individual light length:");
    Serial.println(yellowIndividualLightLength);

    while(true) // Will only execute once, this allows the use of break if the reset was tapped
    {
        buttonTapped = LED_OnOff_DelayAndButtonCheck(YELLOW_TOP_LED, yellowIndividualLightLength);

        if (buttonTapped == REMOTE_BUTTON_RESET)
            break;

        TempPrintButtonTappedState(buttonTapped, 5);

        buttonTapped = LED_OnOff_DelayAndButtonCheck(YELLOW_MID_LED, yellowIndividualLightLength);

        if (buttonTapped == REMOTE_BUTTON_RESET)
            break;

        TempPrintButtonTappedState(buttonTapped, 6);

        buttonTapped = LED_OnOff_DelayAndButtonCheck(YELLOW_BOT_LED, yellowIndividualLightLength);

        // break out of the while loop
        break;
    }

    TempPrintButtonTappedState(buttonTapped, 7);

    return buttonTapped;
}

void LED_AllOff()
{
    LED_Control(0xFF,  LOW);
}

void LED_On(int ledMask)
{
    LED_Control(ledMask, HIGH);
}

void LED_Off(int ledMask)
{
    LED_Control(ledMask, LOW);
}

void LED_OnOff_Delay(int ledMask, unsigned long delayMS)
{
    LED_On(ledMask);
    delay(delayMS);
    LED_Off(ledMask);
}

ULONG LED_OnOff_DelayAndButtonCheck(int ledMask, ULONG delayMS)
{
    int buttonPressed = 0;
    LED_On(ledMask);
    buttonPressed = DelayAndButtonCheck(delayMS);
    LED_Off(ledMask);

    return buttonPressed;
}

void LED_Control(int ledMask, int ledState)
{
    // Serial.print("#### LED_Control -- Mask: ");
    // Serial.print(ledMask, HEX);
    // Serial.print("  ledState:");
    // Serial.println(ledState);
    if ((ledMask & PRESTAGE_LED) == PRESTAGE_LED)
        digitalWrite(PRESTAGE_LED_PIN, ledState);
    if ((ledMask & STAGE_LED) == STAGE_LED)
        digitalWrite(STAGE_LED_PIN, ledState);
    if ((ledMask & YELLOW_TOP_LED) == YELLOW_TOP_LED)
        digitalWrite(YELLOW_TOP_LED_PIN, ledState);
    if ((ledMask & YELLOW_MID_LED) == YELLOW_MID_LED)
        digitalWrite(YELLOW_MID_LED_PIN, ledState);
    if ((ledMask & YELLOW_BOT_LED) == YELLOW_BOT_LED)
        digitalWrite(YELLOW_BOT_LED_PIN, ledState);
    if ((ledMask & GREEN_LED) == GREEN_LED)
        digitalWrite(GREEN_LED_PIN, ledState);
}

void TempPrintButtonTappedState(ULONG buttonTapped, int callSpot)
{
    // TEMP
    if (buttonTapped != NO_BUTTON_PRESSED)
    {
        Serial.print("Call Spot:");
        Serial.print(callSpot);
        Serial.print("  Button tapped is: 0x");
        Serial.println(buttonTapped, HEX);
    }
}

ULONG DelayAndButtonCheck(ULONG delayTime)
{
    int reading = 0;
    int delayHop = 5;
    bool buttonPressed = false;
    unsigned long buttonPressedValue = 0;
    long irDebounceTime = 500;

    for (ULONG i = 0; i < delayTime; i = i + delayHop)
    {
        // button on the breadboard
        reading = digitalRead(BREADBOARD_BUTTON_PIN);

        // receiver pin
        if (_receiver.decode(&_results) && (millis() - _buttonLastPressTime > irDebounceTime) )  // Received signal and stored in results
        {
            // The repeated key case will re-set this back to false
            buttonPressed = true;
            Serial.print("Remote button pressed: ");
            switch(_results.value)
            {
                // Buttons from the Header remote
                case REMOTE_HEATER_BUTTON_POWER:
                    Serial.print("REMOTE_HEATER_BUTTON_POWER (REMOTE_BUTTON_RESET) -- ");
                    break;
                case REMOTE_HEATER_BUTTON_UP:
                    Serial.print("REMOTE_HEATER_BUTTON_UP (REMOTE_BUTTON_PRO_TREE_5) -- ");
                    break;
                case REMOTE_HEATER_BUTTON_DOWN:
                    Serial.print("REMOTE_HEATER_BUTTON_DOWN (REMOTE_BUTTON_PRO_TREE_4) -- ");
                    break;
                case REMOTE_HEATER_BUTTON_FANSPEED:
                    Serial.print("REMOTE_HEATER_BUTTON_FANSPEED (REMOTE_BUTTON_FULL_TREE) -- ");
                    break;
                case REMOTE_HEATER_BUTTON_TIMER:
                    Serial.print("REMOTE_HEATER_BUTTON_TIMER (REMOTE_BUTTON_PRO_TREE_RANDOM) -- ");
                    break;
                case REMOTE_HEATER_BUTTON_MODE:
                    Serial.print("REMOTE_HEATER_BUTTON_MODE (REMOTE_BUTTON_FULL_TREE_RANDOM) -- ");
                    break;

                // Whenever the IR Receiver detects the same key has been pressed, it refers to it as a repeated key
                case REMOTE_REPEATED_KEY:
                    Serial.print("(Repeated Key) -- ");
                    buttonPressed = false;
                    break;

                default:
                    Serial.print("Unknown button!  Value: ");
                    // Set to false so we don't interrupt the ongoing delay
                    buttonPressed = false;
            }

            // TODO: Maybe the debounce logic works here as well or something?
            Serial.println(_results.value, HEX);
            _buttonLastPressTime = millis();
            _receiver.resume();

            if (buttonPressed)
            {
                buttonPressedValue = _results.value;
                _rcv_key_value = _results.value;
                break;
            }
        }

        if (reading == HIGH && _buttonPreviousState == LOW && (millis() - _buttonLastPressTime > DEBOUNCE_TIME))
        {
            // I'm not really following the logic from this example, but let's see.
            if (_buttonState == HIGH)
            {
                // temp
                Serial.println("Current buttonstate is HIGH, setting to LOW");
                _buttonState = LOW;
            }
            else
            {
                // temp
                Serial.println("Current buttonstate is HIGH, setting to HIGH");
                _buttonState = HIGH;
            }

            _buttonLastPressTime = millis();
            buttonPressedValue = BREADBOARD_BUTTON;
            break;
        }

        delay(delayHop);
    }

    return buttonPressedValue;
}