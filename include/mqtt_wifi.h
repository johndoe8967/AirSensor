#include "EspMQTTClient.h"
#include "DateTimeMS.h"

extern const char *devName;
extern EspMQTTClient MQTTClient;

void setupMQTT(void (*onConnectCB)(), bool debug);
bool loopMQTT();

void newMessage(char *msg, const char* name, const char* value);
void add2Message(char *msg, const char* name, const char* value, bool quotes=true);
void add2Message(char *msg, const char* name, const long value);



