#include <SPI.h>
#include <Ethernet.h>
#include <aWOT.h>
#include <ArduinoJson.h>
#include <SimpleDHT.h>


#define rainSensorPower 7
#define rainSensorPin 8
#define redLED 3
#define yellowLED 4 
#define greenLED 5
#define tempSensor 2

SimpleDHT11 dht11(tempSensor);

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
EthernetServer server(80);
EthernetClient client;
Application app;

int * readTempHumid() {

  static int  r[10];
  byte temperature = 0;
  byte humidity = 0;
  int err = SimpleDHTErrSuccess;
  if ((err = dht11.read(&temperature, &humidity, NULL)) != SimpleDHTErrSuccess) {
    Serial.print("Read DHT11 failed, err="); Serial.print(SimpleDHTErrCode(err));
    Serial.print(","); Serial.println(SimpleDHTErrDuration(err)); delay(1000);
    return;
  }
  
  Serial.print("Sample OK: ");
  Serial.print((int)temperature); Serial.print(" *C, "); 
  Serial.print((int)humidity); Serial.println(" H");
  
  // DHT11 sampling rate is 1HZ.
  delay(1500);
  r[0]=(int)temperature;
  r[1]=(int)humidity;

  return r;
}

int readRainSensor() {
  digitalWrite(rainSensorPower, HIGH);  // Turn the rain sensor ON
  delay(10);              // Allow power to settle
  int val = digitalRead(rainSensorPin); // Read the rain sensor output
  digitalWrite(rainSensorPower, LOW);   // Turn the rain sensor OFF
  return val;             // Return the value
}

// define a handler function
void indexCmd(Request &req, Response &res) {

  int *p;

  p = readTempHumid();

  int val = readRainSensor();

  // Allocate a temporary JsonDocument
  // Use arduinojson.org/v6/assistant to compute the capacity.
  StaticJsonDocument<500> doc;

  JsonArray temperature = doc.createNestedArray("temperature");
  temperature.add(p[0]);
  JsonArray humidity = doc.createNestedArray("humidity");
  humidity.add(p[1]);
  JsonArray rain = doc.createNestedArray("rain");

  if (val) {
    rain.add("It's not raining");
  } else {
     rain.add("It's raining");
  }

  Serial.print(F("Sending: "));
  serializeJson(doc, Serial);
  Serial.println();

  // Write response headers
  client.println(F("HTTP/1.0 200 OK"));
  client.println("Access-Control-Allow-Origin: *");
    client.println(
        "Access-Control-Allow-Methods: GET, POST, PUT, DELETE, OPTIONS");
    client.println("Access-Control-Allow-Headers: "
                   "Origin, X-Requested-With, Content-Type, Accept");
  client.println(F("Content-Type: application/json"));
  client.println(F("Connection: close"));
  client.print(F("Content-Length: "));
  client.println(measureJsonPretty(doc));
  client.println();

 // Write JSON document
  serializeJsonPretty(doc, client);
}

void setup() {

  Serial.begin(115200);
  pinMode(rainSensorPower, OUTPUT);
  pinMode(redLED, OUTPUT);
  pinMode(yellowLED, OUTPUT);
  pinMode(greenLED, OUTPUT);
  digitalWrite(rainSensorPower, LOW);

  if (Ethernet.begin(mac)) {
    Serial.println(Ethernet.localIP());
  } else{
    Serial.println("Ethernet failed");
  }

  // mount the handler to the default router
  app.get("/", &indexCmd);
}

void loop(){
  int *p;

  p = readTempHumid();
  if(p[0]>35 || p[1] >70)
  {
     digitalWrite(yellowLED, HIGH);
     digitalWrite(greenLED, LOW);
  }
  else if(p[0]<35 || p[1]<70)
  {
    digitalWrite(yellowLED, LOW);
    digitalWrite(greenLED, HIGH);
  }

  int val = readRainSensor();

  if(val)
  {
    digitalWrite(redLED, LOW);
  } else
  {
    digitalWrite(redLED, HIGH);
  }
  
  client = server.available();
  if (client.connected()) {
    app.process(&client);
    client.stop();
  }
}
