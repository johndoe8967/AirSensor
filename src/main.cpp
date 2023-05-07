// Fronius Smartmeter, Inverter and Battery
#include "WifiManager_Helper.h"
#include <Arduino.h>
#include "mqtt_wifi.h"
#include <ArduinoJson.h>
#include "Version.h"


#define serialConsole
Stream *console;

#define debug false
//#define _DEBUG_ 1


unsigned long UpdateIntervall = 10000; // 10 seconds update intervall
unsigned long nextUpdateTime = 0;     // absolut timestamp of next update
unsigned long getNextUpdateTime() { return millis() + UpdateIntervall; };

StaticJsonDocument<512> data;
String inputString = "";          // a String to hold incoming data
bool stringComplete = false;      // whether the string is complete
bool valueAvailable = false;

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
  data["name"]=DEVICENAME;
  data["field"]="Air";
  data["Value"]=count;
  data["time"]=getStringTimeWithMS();
  serializeJson(data,message);
  Serial.println(message);

/*  message = "{\"name\":\"" DEVICENAME "\",\"field\":\"Air\",\"Value\":";
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
  message += ",\"time\":";
  message += getStringTimeWithMS();
  message += "}";*/
  MQTTClient.publish("sensors", message); // You can activate the retain flag by setting the third parameter to true
}


// -----------------------------------------------------------------
// cyclic loop
// -----------------------------------------------------------------
void loop()
{
  // read all available character from the serial and add to inputString, a string is complete if a CR is receaved
  #define MAX_MESSAGE_LENGTH 100
  static char message[MAX_MESSAGE_LENGTH];

  while (Serial.available() > 0)
  {
    //Create a place to hold the incoming message
    static unsigned int message_pos = 0;

    //Read the next available byte in the serial receive buffer
    char inByte = Serial.read();

    //Message coming in (check not terminating character) and guard for over message size
    if ( inByte != '\n' && (message_pos < MAX_MESSAGE_LENGTH - 1) )
     {
      //Add the incoming byte to our message
      message[message_pos] = inByte;
      message_pos++;
    }
    //Full message received...
    else
   {
      //Add null character to string
      message[message_pos] = '\0';

      //Print the message (or do other things)
      DeserializationError err = deserializeJson(data, message);

      if (err == DeserializationError::Ok) 
      {
        Serial.println("Got Values");
        valueAvailable = true;
      } 
      else 
      {
        // Print error to the "debug" serial port
        Serial.print("deserializeJson() returned ");
        Serial.println(err.c_str());
      }


      //Reset for the next message
      message_pos = 0;
   }
 }

  if (loopMQTT()) // process mqtt
  {
    if (valueAvailable)
    {
        sendNewData();
        valueAvailable=false;
    }
  }
  delay(10);
}