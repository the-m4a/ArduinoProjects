// This project uses a DHT Temperature Sensor, a 7-segment display, and an active buzzer to record the temperature and humidity every 5 seconds.
// Every 3rd time it records a temperature, send data over a Serial Connection to a NodeMCU arduino that is configured to listen on it's Incoming Serial in order to upload that to an Azure Function.
//
// PlatformIO extension for VSCode was employed to download the various additional libraries required, see the ChickenTemperatureTracking.h and platformio.ini files.
#include "ChickenCoupTemperatureTracking.h"

void setup()
{
    // Set the display brightness (0-7):
    _display.setBrightness(4);
    // Clear the display:
    _display.clear()     ;
    // Setup sensor:
    _dht.begin();

    tone(BUZZERPIN, NOTE_D4, 100);
    // Output the voice after several minutes
    delay(100);

    for (int i = 0; i <= 9999; i = i + 1111)
    {
        _display.showNumberDec(i, true, 4, 0);
        delay(100);
    }

    _display.setSegments(dashessymbol, 4, 0);

    // Begin serial communication at a baud rate of 9600:
    Serial.begin(9600);

    // Wait for console opening:
    delay(2000);
    _mySerial.begin(9600);

    _iteration = 0;
    _alarmIterations = 0;
}

void loop()
{
    // Read the temperature as Celsius and Fahrenheit:
    _temperature_celsius = _dht.readTemperature();
    _temperature_fahrenheit = _dht.readTemperature(true);
    _humidity = _dht.readHumidity();
    // Print the temperature to the Serial Monitor:
    Serial.print("Celcius: ");
    Serial.print(_temperature_celsius);
    Serial.print("  Fahrenheit: ");
    Serial.print(_temperature_fahrenheit);
    Serial.print("  Humidity: ");
    Serial.println(_humidity);
    _iteration++;

    if (_iteration % 3 == 0)
    {
        Serial.println("Sending data over Serial connection");
        // Write the temperature over the serial port in Fahrenheit, so the ESP8266 can send it to the internet
        String serialMsg = String(_temperature_fahrenheit) + "|" + String(_humidity);
        for (int i = 0; i < serialMsg.length(); i++)
        {
            _mySerial.write(serialMsg[i]);
        }

        _mySerial.write(ENDBUFFERCHAR);
        // Reset the iteration counter thingy
        _iteration = 0;
    }

    // Show the temperature on the TM1637 display:
    _display.showNumberDec(_temperature_celsius, false, 3, 0);
    _display.setSegments(celsiussymbol, 3, 3);
    delay(1500);
    _display.showNumberDec(_temperature_fahrenheit, false, 3, 0);
    _display.setSegments(fahrenheitsymbol, 3, 3);
    delay(1500);
    // put the H on the left
    _display.showNumberDec(_humidity, false, 3, 1);
    _display.setSegments(humiditysymbol, 1, 0);
    delay(1500);

    if ((_temperature_fahrenheit <= 60) || (_temperature_fahrenheit > 125))
    {
        _alarmIterations++;
        if (_alarmIterations >= 3)
        {
            soundAlarm();
        }
    }
    else
    {
        _alarmIterations = 0;
    }
}

void soundAlarm()
{
    _display.setSegments(errsymbol, 4, 0);
    for (int j = 300; j < 600; j++)
    {
        tone(BUZZERPIN, j, 5);
        delay(5);
    }
}