#include <ArduinoJson.h>
#include <WiFi.h>

#define PRESS LOW

WiFiClient  client;

const char* ssid = "OPPO F9";
const char* password = "01234567";

//GPIO Mapping
const byte ledPin = 2;
const byte ledPin22 = 22;
const byte sw1Pin23 = 23;

//HTTP Mapping
const char* host = "192.168.43.218";
int httpPort = 8080;
String url = "/app1.php?value=";

//Global variable
unsigned long ledTick;
char bufArr[8];

//Prototype Function
void httpRequest(void);
void httpSend(const char* d);
void setup_wifi(void);

void setup() {
  // Initialize Serial port
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);
  pinMode(ledPin22, OUTPUT);
  pinMode(sw1Pin23, INPUT_PULLUP);
  while (!Serial) continue;
  setup_wifi();
}

void loop() {
  if (PRESS == digitalRead(sw1Pin23)) {
    delay(500);
    sprintf(bufArr, "1");
    httpSend(bufArr);
  }
  //LED Flashing Indicate programming still running
  //-----------------------------------------------------
  if (millis() > ledTick) {
    ledTick = millis() + 300;
    digitalWrite(ledPin, digitalRead(ledPin) ^ 1);
  }
  //-----------------------------------------------------
}

//User Function - setup_wifi()
//------------------------------------------------
void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}
//End Function
//------------------------------------------------

void httpSend(const char* d) {
  // Connect to HTTP server
  client.setTimeout(10000);
  if (!client.connect(host, httpPort)) {
    Serial.println(F("Connection failed"));
    return;
  }
  // Send HTTP request
  /*
    HTTP Request Format:
    GET<space>URL<space>HTTP/1.1<enter>
    Host: <HOSTNAME or IP><enter>
    Connection: close<enter><enter>
  */
  String header = "";
  header = "GET ";
  header += url;
  header += d;
  header += " HTTP/1.1\r\n";
  header += "Host: ";
  header += host;
  header += "\r\nConnection: close\r\n\r\n";
  Serial.print(header);
  client.print(header);

  if (client.println() == 0) {
    Serial.println(F("Failed to send request"));
    return;
  }

  // Check HTTP status
  char status[32] = {0};
  client.readBytesUntil('\r', status, sizeof(status));
  // It should be "HTTP/1.0 200 OK" or "HTTP/1.1 200 OK"
  if (strcmp(status + 9, "200 OK") != 0) {
    Serial.print(F("Unexpected response: "));
    Serial.println(status);
    client.stop();
    return;
  }

  // Skip HTTP headers
  char endOfHeaders[] = "\r\n\r\n";
  if (!client.find(endOfHeaders)) {
    Serial.println(F("Invalid response"));
    client.stop();
    return;
  }

  // Allocate the JSON document
  // Use https://arduinojson.org/v6/assistant to compute the capacity.
  const size_t capacity = JSON_OBJECT_SIZE(3) + JSON_ARRAY_SIZE(2) + 60;
  DynamicJsonDocument doc(capacity);

  // Parse JSON object
  DeserializationError error = deserializeJson(doc, client);
  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
    client.stop();
    return;
  }

  String value = doc["result"];
  Serial.println(value);
  
  // Disconnect
  client.stop();

  //Application according to JSON Data
  if (value == "1") {
    digitalWrite(ledPin22, HIGH);
    Serial.println("LED on");
  }
  else if (value == "0") {
    digitalWrite(ledPin22, LOW);
    Serial.println("LED off");
  }
}
