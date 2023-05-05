// Fronius Smartmeter, Inverter and Battery
#include "WifiManager_Helper.h"
#include <Arduino.h>
#include "mqtt_wifi.h"
#include <ArduinoJson.h>
#include "Version.h"

#define serialConsole
Stream *console;

#define debug 1


unsigned long UpdateIntervall = 10000; // 10 seconds update intervall
unsigned long nextUpdateTime = 0;     // absolut timestamp of next update
unsigned long getNextUpdateTime() { return millis() + UpdateIntervall; };

String inputString = "";          // a String to hold incoming data
bool stringComplete = false;      // whether the string is complete

// -----------------------------------------------------------------
// initialisations after each reconnect to WIFI
// -----------------------------------------------------------------
void onConnectDB(void)
{
  nextUpdateTime = getNextUpdateTime(); // calculate next update timestamp
}

// -----------------------------------------------------------------
// initialisations at bootup
// -----------------------------------------------------------------
void setup()
{
  Serial.begin(9600);
#ifndef _DEBUG_
  Serial.swap();
#endif
  inputString.reserve(200);       // reserve 200 bytes for the inputString:

  Serial.println("Booting");
  WifiManager_OnSetup();

  console = &Serial;

  setupMQTT(&onConnectDB, debug); // initialize MQTT

  Serial.println("wait for WIFI");

}


long count=0;
float temp=0.0;
float press=0.0;
float humidity=0.0;
float CO2=0.0;

// -----------------------------------------------------------------
// send Data to Cloud (ThingSpeak and MQTT)
// -----------------------------------------------------------------
void sendNewData() {
  String message;                           // will contain the http message to send into cloud
  count++;
  // Publish a message to "mytopic/test"
  message = "{\"name\":\"" DEVICENAME "\",\"field\":\"Air\",\"Value\":";
  message += count;
  if (true) {
    message += ",\"Temperature\":";
    message += temp;
    message += ",\"Pressure\":";
    message += press;
    message += ",\"Humidity\":";
    message += humidity;
    message += ",\"CO2\":";
    message += CO2;
  }
  if (false) {
    message += ",\"CO2\":";
    message += 400.0;
  }
  message += ",\"time\":";
  message += getStringTimeWithMS();
  message += "}";
  MQTTClient.publish("sensors", message); // You can activate the retain flag by setting the third parameter to true
}




// -----------------------------------------------------------------
// cyclic loop
// -----------------------------------------------------------------
void loop()
{
  String PVData;
  while (Serial.available()) {
    // get the new byte:
    char inChar = (char)Serial.read();
    // add it to the inputString:
    inputString += inChar;
    // if the incoming character is a newline, set a flag so the main loop can
    // do something about it:
    if (inChar == '\r') {
      stringComplete = true;
    }
  }
  if (stringComplete) {
    do {
      Serial.println(inputString);
      
      if (inputString.startsWith("HUMI")) {
        humidity = inputString.substring(4).toFloat(); 
      }

      if (inputString.startsWith("TEMP")) {
        temp = inputString.substring(4).toFloat(); 
      }
      if (inputString.startsWith("PRESS")) {
        press = inputString.substring(5).toFloat(); 
      }
      if (inputString.startsWith("CO2")) {
        CO2 = inputString.substring(3).toFloat(); 
      }
      inputString = inputString.substring(inputString.indexOf('\r'));
    } while (inputString.indexOf('\r') > 0);
    inputString="";
    stringComplete=false;
  }

  if (loopMQTT()) // process mqtt
  {
    if (millis() > nextUpdateTime) // check if update timestamp reached
    {
      nextUpdateTime += UpdateIntervall; // calculate next update timestamp
      sendNewData();
    }
  }
  delay(10);
}