
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

#define trigPin D1
#define echoPin D2
int reads = 0;
const char *host = "YOUR HOST";
const char* wifi_name = "YOUR WIFI";
const char* wifi_password = "YOUR WIFI'S PASSWORD";
int wifiStatus;
String textHTML;
long duration;
float distance, aux;

ESP8266WebServer server(80);

void handleRoot() {
  textHTML = "<meta http-equiv= refresh content= 3>";
  textHTML += "distance: ";
  textHTML += distance;
  textHTML += "cm";
  textHTML += "<br> duration: ";
  textHTML += duration;
  server.send(200, "text/html", textHTML);
}

void handleNotFound() {
  String message = "File not found \n\n";
  message += "URI: ";
  message += server.uri();
  message += "\n Methods: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\n Args: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i > server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}

void setup(void) {
  Serial.begin(115200);
  delay(200);
  Serial.print("Starting wifi connection ");
  Serial.println(wifi_name);
  WiFi.begin(wifi_name, wifi_password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  digitalWrite(trigPin, LOW);
  wifiStatus = WiFi.status();
  if ( wifiStatus == WL_CONNECTED) {
    Serial.println("NodeMCU connected on lan! ");
    Serial.print("IP to lan monitoring: ");
    Serial.println(WiFi.localIP());
    Serial.println("");
  }
  else {
    Serial.println("NodeMCU not connected");
  }
  delay(1000);
  server.on("/", handleRoot);
  server.on("/inline", []() {
    server.send(200, "text/plain", "2 opção de envio");
  });
  server.onNotFound(handleNotFound);
  server.begin();
  Serial.println("HTTP server iniciado");
}
void loop(void) {
  server.handleClient();
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH , 1000000);
  aux = duration;
  distance = (aux * 340 / 10000) / 2; // distance em centimetros
  Serial.print("distance: ");
  Serial.println(distance);
  Serial.print("duration: ");
  Serial.println(duration);
  reads = reads + 1;
  if (reads > 300) {
    WiFiClient client;
    const int httpPort = 80; // Change according to your host port
    if (!client.connect(host, httpPort)) {
      Serial.println("connection failure");
    }
    String data = String(distance,0);
    client.println(data);
    Serial.println("connection OK!");
    Serial.println(data);
    unsigned long timeout =  millis();
    while (client.available() == 0) {
      if (millis() - timeout > 5000) {
        Serial.println(">>> Client Timeout !");
        client.stop();
        return;
      }
    }
    reads = 0;
  }
  delay(1000);
}
