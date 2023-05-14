/*

Connecting the BME280 Sensor:
Sensor              ->  Board
-----------------------------
Vin (Voltage In)    ->  3.3V
Gnd (Ground)        ->  Gnd
SDA (Serial Data)   ->  A4 
SCK (Serial Clock)  ->  A5 
DHT anschluß
1 Vcc 
2 IO Pin
3 NC
4 GND
10k zwischen 1 und 2

ESP Anbindung
+5V
GND
Nano D2-> 1K -> ESP D7 ->2k ->GND (5->3,3V)
ESP D8 -> Nano D3

*/

#include <DHT.h>
#include <Wire.h>
#include <BME280I2C.h>
#include <ArduinoJson.h>
#define SERIAL_BAUD 115200

//#define debug
#ifdef debug
#define mySerial Serial
#define DelayCount 50
#else
#include <SoftwareSerial.h>
SoftwareSerial mySerial(3, 2);  //(D3=RX,D2=TX Nano) (D8=TX,D7=RX ESP)
#define DelayCount 140
#endif

const int ledPin = LED_BUILTIN;  // the number of the LED pin
int ledState = LOW;              // ledState used to set the LED


BME280I2C bme;  // Default : forced mode, standby time = 1000 ms
                // Oversampling = pressure ×1, temperature ×1, humidity ×1, filter off,
float Luftdruck = 0.0;

#define DHTPIN 4           // what pin we're connected to
#define DHTTYPE DHT22      // DHT22
DHT dht(DHTPIN, DHTTYPE);  // Initialize DHT sensor for normal 16mhz Arduino
float Feuchte = 0.0;       //Stores humidity value
float dhtTemp = 0.0;     //Stores temperature value
float bmeTemp = 0.0;

float CO_2 = 0.0;  //Stores CO2 concentration

String inputString = "";      // a String to hold incoming data
bool stringComplete = false;  // whether the string is complete
StaticJsonDocument<265> doc;


void setup() {
  // initialize serial communications at 9600 bps:
  Serial.begin(SERIAL_BAUD);
  mySerial.begin(9600);
  inputString.reserve(200);  // reserve 200 bytes for the inputString:
  dht.begin();
  pinMode(ledPin, OUTPUT);
  while (!Serial) {}  // Wait

  Wire.begin();

  while (!bme.begin()) {
    Serial.println("Could not find BME280 sensor!");
    delay(1000);
  }
  switch (bme.chipModel()) {
    case BME280::ChipModel_BME280:
      Serial.println("Found BME280 sensor! Success.");
      break;
    case BME280::ChipModel_BMP280:
      //Serial.println("Found BMP280 sensor! No Humidity available.");
      break;
    default:
      Serial.println("Found UNKNOWN sensor! Error!");
  }
}

void buildJson() {
  doc.clear();
  doc["HUMI"]=Feuchte;
  doc["Temp"]=dhtTemp;
  doc["Temp1"]=bmeTemp;
  doc["CO_2"]=CO_2;
  doc["PRES"]=Luftdruck;
}
void send2ESP8266() {
  // print the results to the ESP8266:
  serializeJson(doc,mySerial);
  mySerial.println();
}


unsigned char counter = 0;
void loop() {
  counter++;
  if (counter == DelayCount) {
    counter = 0;
    readAndCalcBME280Data();
    //Read data and store it to variables hum and temp
    Feuchte = dht.readHumidity();
    if (isnan(Feuchte)) Serial.println("Error reading Feuchte");
    dhtTemp = dht.readTemperature();
    if (isnan(dhtTemp)) Serial.println("Error reading dhtTemp");
    if (ledState == LOW) {
      ledState = HIGH;
    } else {
      ledState = LOW;
    }

    // set the LED with the ledState of the variable:
    digitalWrite(ledPin, ledState);
    buildJson();
    send2ESP8266();
  }

  while (mySerial.available()) {
    // get the new byte:
    char inChar = (char)mySerial.read();
    // add it to the inputString:
    inputString += inChar;
    // if the incoming character is a newline, set a flag so the main loop can
    // do something about it:
    if (inChar == '\n') {
      stringComplete = true;
    }
  }

  if (stringComplete) {
    // clear the string:
    Serial.print("ESP: ");
    Serial.println(inputString);
    inputString = "";
    stringComplete = false;
  }


  // wait 200 milliseconds before the next loop for the analog-to-digital
  // converter to settle after the last reading:
  delay(200);
}

void readAndCalcBME280Data() {
  float temp(NAN), hum(NAN), pres(NAN);

  BME280::TempUnit tempUnit(BME280::TempUnit_Celsius);
  BME280::PresUnit presUnit(BME280::PresUnit_Pa);

  bme.read(pres, bmeTemp, hum, tempUnit, presUnit);

  Luftdruck = (pres / 100 + 31.5);
}
