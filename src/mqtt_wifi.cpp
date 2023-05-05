#include "mqtt_wifi.h"
#include "Version.h"

#include "CredentialSetting.h"
#include "CredentialSettingsDefault.h"

extern Stream *console;

const char *devName = DEVICENAME;

void (*_onConnectCB)();

EspMQTTClient MQTTClient(
    MQTTHostname, // MQTT Broker server ip
    MQTTPort,            // The MQTT port, default to 1883
    MQTTUser,
    MQTTPassword,
    devName // Client name that uniquely identify your device
);

void setupMQTT(void (*onConnectCB)(), bool debug)
{
    _onConnectCB = onConnectCB;
    if (debug)
        MQTTClient.enableDebuggingMessages(true);                                  // Enable debugging messages sent to serial output
    MQTTClient.enableLastWillMessage("device/lastwill", DEVICENAME " I am going offline"); // You can activate the retain flag by setting the third parameter to true
    MQTTClient.setMaxPacketSize(2200);
//    MQTTClient.enableOTA();
//    MQTTClient.enableOTA(OTAPassword);
}
// -----------------------------------------------------------------
// This function will setup all services and is called once everything is connected (Wifi and MQTT)
// -----------------------------------------------------------------
void onConnectionEstablished()
{
    DateTime.begin(/* timeout param */);
    console->println("onConn");
    delay(1000);

    // announce that the device is online again in the cloud
    MQTTClient.publish("device/online", devName);

    // subscribe to device scan channel
    MQTTClient.subscribe("device", [](const String &payload)
                         {
    if (payload == "scan") {
      MQTTClient.publish("device/scan", devName);
    } });

    String message;
    static long count = 1;
    message = "{\"name\":\"SENSORS\",\"Value\":";
    message += count;
    message += ",\"field\":\"";
    message += devName;
    message += " ";
    message += VERSION;
    message += "\",\"time\":";
    message += getStringTimeWithMS();
    message += "}";
    MQTTClient.publish("sensors", message);

    (*_onConnectCB)();
}

void streamCommands()
{
    switch (console->read())
    {
    case 'R':
        console->println("bye bye / restart");
        delay(100);
        ESP.restart();
        break;
    case 'V':
        console->print("DeviceName: ");
        console->println(devName);
        console->print("Version: ");
        console->println(VERSION);
        break;
    }
}

bool loopMQTT()
{
    streamCommands();
    MQTTClient.loop(); // Handle MQTT

    if (MQTTClient.isConnected())
    {

        // test if time is still valid
        if (!DateTime.isTimeValid())
        {
            DateTime.begin(/* timeout param */);
        }
        else
        {
            return true;
        }
    }
    return false;
}

void newMessage(char* msg, const char* name, const char* value) 
{
    strcpy(msg,"{\"");
    strcpy(msg+2, name);
    int len = strlen(msg);
    strcpy(msg+len,"\":\"");
    len += 3;
    strcpy(msg+len, value);
    len = strlen(msg);
    strcpy(msg+len,"\"}");
    len = strlen(msg);
    msg[len] = 0;
}

void add2Message(char *msg, const char* name, const char* value,bool quotes) 
{
    int len = strlen(msg);
    strcpy(msg+len - 1,",\"");
    strcpy(msg+len + 1, name);
    len = strlen(msg);
    strcpy(msg+len,"\":");
    len += 2;

    if (quotes) {
        strcpy(msg+len,"\"");
        len++;
    }

    strcpy(msg+len, value);
    len = strlen(msg);

    if (quotes) {
        strcpy(msg+len,"\"");
        len++;
    }

    strcpy(msg+len,"}");
    len = strlen(msg);
    msg[len] = 0;
}


void add2Message(char *msg, const char* name, const long value)
{
    char valueChar[10];
    itoa(value, valueChar,10);
    add2Message(msg,name,valueChar,false);
}