#include <SPI.h>
#include <Ethernet.h>
#include <aWOT.h>
#include <ArduinoJson.h>

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
EthernetServer server(80);
EthernetClient client;
Application app;

// define a handler function
void indexCmd(Request &req, Response &res) {

  // Allocate a temporary JsonDocument
  // Use arduinojson.org/v6/assistant to compute the capacity.
  StaticJsonDocument<500> doc;

  // Create the "analog" array
  JsonArray analogValues = doc.createNestedArray("analog");
  for (int pin = 0; pin < 6; pin++) {
    // Read the analog input
    int value = analogRead(pin);

    // Add the value at the end of the array
    analogValues.add(value);
  }

  // Create the "digital" array
  JsonArray digitalValues = doc.createNestedArray("digital");
  for (int pin = 0; pin < 14; pin++) {
    // Read the digital input
    int value = digitalRead(pin);

    // Add the value at the end of the array
    digitalValues.add(value);
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

  if (Ethernet.begin(mac)) {
    Serial.println(Ethernet.localIP());
  } else{
    Serial.println("Ethernet failed");
  }

  // mount the handler to the default router
  app.get("/", &indexCmd);
}

void loop(){
  client = server.available();
  if (client.connected()) {
    app.process(&client);
    client.stop();
  }
}
