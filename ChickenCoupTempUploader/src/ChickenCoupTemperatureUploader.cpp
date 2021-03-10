#include "ChickenCoupTemperatureUploader.h"

// Embedded Wifi chip
ESP8266WiFiMulti _wifiMulti;

int _bufferMaxLength = 10;
int _bufferIndex = 0;
char _inputBuffer[11];
boolean _bufferReadComplete = false;

void setup()
{
    // The NodeMCU has 2 on-board LEDs, one is red, one is blue.
    pinMode(LED_BUILTIN_RED, OUTPUT);
    pinMode(LED_BUILTIN_BLUE, OUTPUT);
    digitalWrite(LED_BUILTIN_RED, LED_ON);
    digitalWrite(LED_BUILTIN_BLUE, LED_ON);

    Serial.begin(9600);

    for (uint8_t t = 4; t > 0; t--)
    {
        Serial.printf("[SETUP] WAIT %d...\n", t);
        Serial.flush();
        delay(1000);
        if (t % 2 == 0)
        {
            digitalWrite(LED_BUILTIN_RED, LED_ON);
            digitalWrite(LED_BUILTIN_BLUE, LED_OFF);
        }
        else
        {
            digitalWrite(LED_BUILTIN_RED, LED_OFF);
            digitalWrite(LED_BUILTIN_BLUE, LED_ON);
        }
    }

    digitalWrite(LED_BUILTIN_RED, LED_ON);
    digitalWrite(LED_BUILTIN_BLUE, LED_OFF);

    WiFi.mode(WIFI_STA);
    _wifiMulti.addAP(WIFI_NETWORK_NAME, WIFI_NETWORK_PASSWORD);

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }

    digitalWrite(LED_BUILTIN_BLUE, LED_ON);

    Serial.println("");
    Serial.print("Connected to WiFi network with IP Address: ");
    Serial.println(WiFi.localIP());
}

// Loop just exists to read data and once it gets all the data it will then process it and upload it
void loop()
{
    delay(500);
    if (Serial.available())
    {
        ReadSerial();
        if (_bufferReadComplete)
        {
            ProcessNewData();
        }
    }
}

// Read the incoming message from the Arudino connected via the Serial cable
void ReadSerial()
{
    char c = ' ';
    _bufferReadComplete = false;

    if (Serial.available())
    {
        c = Serial.read();
        if (c != ENDBUFFERCHAR)
        {
            _inputBuffer[_bufferIndex] = c;
            _bufferIndex = _bufferIndex + 1;
        }
        else
        {
            _inputBuffer[_bufferIndex] = '\0';
            _bufferIndex = 0;
            _bufferReadComplete = true;
        }
    }
}

void ProcessNewData()
{
    // data sent will be tt|hh - where T is temp and H is humidity
    Serial.print("Process Buffer Data: ");

    String apikeyCode = AZURE_FUNCTION_API_KEY_CODE;
    String clientId = AZURE_FUNCTION_CLIENT_ID;
    String inputData = String(_inputBuffer) + "&clientid=" + clientId;
    String url = AZURE_FUNCTION_SITE_BASE_URL + apikeyCode + "&inputData=" + inputData;

    Serial.println(url);
    UploadTemperature(url);
    _bufferReadComplete = false;
}

// The url is pieced together in ProcessNewData
void UploadTemperature(String url)
{
    WiFiClientSecure client;
    HTTPClient http;

    client.setInsecure();
    client.connect(url, 443);

    if (http.begin(client, url))
    {
        for (int x = 0; x < 4; x++)
        {
            digitalWrite(LED_BUILTIN_BLUE, x % 2);
            delay(150);
        }

        // HTTP
        Serial.println("Attempting to connect to " + url);

        // start connection and send HTTP header
        int httpCode = http.GET();
        Serial.printf("http Get returned %d\n\n", httpCode);

        // httpCode will be negative on error
        if (httpCode > 0)
        {
            // HTTP header has been send and Server response header has been handled
            Serial.printf("[HTTP] GET... code: %d\n", httpCode);
            // file found at server
            if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY)
            {
                String payload = http.getString();
                Serial.println(payload);
            }
        }
        else
        {
            Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
            for (int x = 0; x < 10; x++)
            {
                if (x % 2 == 0)
                {
                    digitalWrite(LED_BUILTIN_RED, LED_OFF);
                    digitalWrite(LED_BUILTIN_BLUE, LED_ON);
                }
                else
                {
                    digitalWrite(LED_BUILTIN_RED, LED_ON);
                    digitalWrite(LED_BUILTIN_BLUE, LED_OFF);
                }

                delay(250);
            }
        }

        http.end();
    }
    else
    {
        // Things didn't go so great, Mr. Stark... make the red light flash
        for (int i = 0; i < 10; i++)
        {
            // the http.begin if returned false
            digitalWrite(LED_BUILTIN_RED, LED_OFF);
            delay(250);
            digitalWrite(LED_BUILTIN_RED, LED_ON);
            delay(250);
        }
    }

    digitalWrite(LED_BUILTIN_RED, LED_ON);
    digitalWrite(LED_BUILTIN_BLUE, LED_ON);
}
