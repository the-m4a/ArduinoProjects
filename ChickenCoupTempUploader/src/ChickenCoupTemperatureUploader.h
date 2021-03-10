// Use PlatformIO Extension for VSCode to pull down the required libraries.  See the platformio.ini file.
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>
#include <WiFiClient.h>

#define LED_BUILTIN_BLUE 2
#define LED_BUILTIN_RED 16
#define LED_ON LOW
#define LED_OFF HIGH
#define ENDBUFFERCHAR 10

#define WIFI_NETWORK_NAME     "Update This With My Actual Wifi Name and Don't Push That to GitHub"
#define WIFI_NETWORK_PASSWORD "ThisIsNotMyWifiPassword"

// These may have been put into a Key Vault...
#define AZURE_FUNCTION_API_KEY_CODE "ThisWouldBeTheAzureFunctionAPIKeyCode"
#define AZURE_FUNCTION_CLIENT_ID "13"
#define AZURE_FUNCTION_SITE_BASE_URL "https://SEECONFIGFILE.azurewebsites.net/api/RecordTemperature?code="

// Function Defintions
void UploadTemperature(String url);
void ReadSerial();
void ProcessNewData();
