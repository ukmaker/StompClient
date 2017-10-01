/**
 * SpringStompDHTSender.ino
 * 
 * This example listens to two STOMP server topics, "/commands/blink" and "/commands/sample".
 * 
 * In response it either blinks the on-board LED, or sends temperature and humidity samples from an attached DHT22 device.
 * 
 * Works best when used with the Spring Websockets+Stomp example code: 
 * 
 * Assuming a NodeMCU device, the DHT22 should be connected:
 * 
 * NodeMcu   | DHT22
 *   3V      |   Pin 1
 *   D3      |   Pin2
 *   GND     |   Pin4
 *   
 * A 10K resistor should be connected between the DHT22 pins 1 and 2.
 * DHT22 pin 3 is left unconnected.
 * 
 * 
 * Author: Duncan McIntyre <duncan@calligram.co.uk>
 * 
 */
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WebSocketsClient.h>
#include "StompClient.h"
#include "DHT.h"

#define LED 2
#define DHTPIN 0
#define DHTTYPE DHT22

/**
* WiFi settings
**/
const char* wlan_ssid             = "--- Your wifi SSID ---";
const char* wlan_password         = "--- Your wifi password";

/**
* Stomp server settings
**/
bool useWSS                       = true;
const char* ws_host               = "--- Your Stomp server hostname ---";
const int ws_port                 = 4443;
const char* ws_baseurl            = "/esp-websocket/"; // don't forget leading and trailing "/" !!!

bool sample = false;
int blink = 0;
bool blinkOn = false;
long lastBlinked;

bool b1 = false;
bool b1_l = false;
bool b2 = false;
bool b2_l = false;
long b1_t = 0;
long b2_t = 0;

// VARIABLES

WebSocketsClient webSocket;

Stomp::StompClient stomper(webSocket, ws_host, ws_port, ws_baseurl, true);

DHT dht(DHTPIN, DHTTYPE);

void setup() {

  // We'll flash this led whenever a message is received
  pinMode(LED, OUTPUT);


  dht.begin();

  // setup serial
  Serial.begin(115200);
  // flush it - ESP Serial seems to start with rubbish
  Serial.println();

  // connect to WiFi
  Serial.println("Logging into WLAN: " + String(wlan_ssid));
  Serial.print(" ...");
  WiFi.begin(wlan_ssid, wlan_password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" success.");
  Serial.print("IP: "); Serial.println(WiFi.localIP());

  // Start the StompClient
  stomper.onConnect(subscribe);
  stomper.onError(error);

  if (useWSS) {
    stomper.beginSSL();
  } else {
    stomper.begin();
  }
}

// Once the Stomp connection has been made, subscribe to a topic
void subscribe(Stomp::StompCommand cmd) {
  Serial.println("Connected to STOMP broker");
  stomper.subscribe("/commands/blink", Stomp::CLIENT, handleBlinkMessage);
  stomper.subscribe("/commands/sample", Stomp::CLIENT, handleSampleMessage);
}

void error(const Stomp::StompCommand cmd) {
  Serial.println("ERROR: " + cmd.body);
}

Stomp::Stomp_Ack_t handleBlinkMessage(const Stomp::StompCommand cmd) {
  Serial.println("Got a message!");
  Serial.println(cmd.body);

  startBlink();
  return Stomp::CONTINUE;
}

Stomp::Stomp_Ack_t handleSampleMessage(const Stomp::StompCommand cmd) {
  Serial.println("Got a message!");
  Serial.println(cmd.body);

  sample = true;
  return Stomp::CONTINUE;
}

void startBlink() {
  lastBlinked = millis();
  blink = 1;
  blinkOn = true;
  digitalWrite(LED, LOW);
}

void blinker() {
  if (blink == 0 || blink > 5) return;
  if (millis() > (lastBlinked + 200)) {
    lastBlinked = millis();
    blink++;
    blinkOn = !blinkOn;
    if (blinkOn) {
      digitalWrite(LED, LOW);
    } else {
      digitalWrite(LED, HIGH);
    }
  }
}

void takeSample() {
  if (sample) {
    stomper.sendMessage("/esp/sensors", "{\\\"name\\\":\\\"temperature\\\",\\\"value\\\":" + String(dht.readTemperature()) + "}");
    stomper.sendMessage("/esp/sensors", "{\\\"name\\\":\\\"humidity\\\",   \\\"value\\\":" + String(dht.readHumidity()) + "}");
    sample = false;
  }
}


void loop() {
  webSocket.loop();
  blinker();
  takeSample();
}
