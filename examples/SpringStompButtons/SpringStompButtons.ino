/**
 * SpringStompButtons.ino
 * 
 * This example monitors two buttons attached to an ESP8266 device and sends a message to a STOMP server
 * when they are pressed.
 * 
 * Works best when used with the Spring Websockets+Stomp example code: 
 * 
 * 
 * Connect a normally-open button between ESP D1 and ground, and between ESP D2 and ground.
 * 
 * Author: Duncan McIntyre <duncan@calligram.co.uk>
 * 
 */

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WebSocketsClient.h>
#include "StompClient.h"


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


// Confusingly, on a NodeMCU board these correspond to D1 and D2
#define BUTTON1 4
#define BUTTON2 5

// VARIABLES

WebSocketsClient webSocket;

Stomp::StompClient stomper(webSocket, ws_host, ws_port, ws_baseurl, true);

bool b1 = false;
bool b1_l = false;
bool b2 = false;
bool b2_l = false;
long b1_t = 0;
long b2_t = 0;


void setup() {

  // Enable pullups on the GPIO pins the buttons are attached to
  pinMode(BUTTON1, INPUT_PULLUP);
  pinMode(BUTTON2, INPUT_PULLUP);

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

  stomper.onConnect(subscribe);
  stomper.onError(error);

  // Start the StompClient
  if (useWSS) {
    stomper.beginSSL();
  } else {
    stomper.begin();
  }
}

// Once the Stomp connection has been made, subscribe to a topic
void subscribe(Stomp::StompCommand cmd) {
  Serial.println("Connected to STOMP broker");
}

void error(const Stomp::StompCommand cmd) {
  Serial.println("ERROR: " + cmd.body);
}

void clickB1() {
  stomper.sendMessage("/esp/buttons", "{\\\"button\\\": 0}");
}

void clickB2() {
  stomper.sendMessage("/esp/buttons", "{\\\"button\\\": 1}");
}

/**
 * Read and debounce the buttons
 */
void buttons() {

  bool b = digitalRead(BUTTON1);
  if (b != b1_l) {
    b1_t = millis();
    b1_l = b;
  } else if ((millis() - b1_t) > 200) {
    if (b1_l && !b1) clickB1();
    b1 = b1_l;
  }

  b = digitalRead(BUTTON2);
  if (b != b2_l) {
    b2_t = millis();
    b2_l = b;
  } else if ((millis() - b2_t) > 200) {
    if (b2_l && !b2) clickB2();
    b2 = b2_l;
  }

}

void loop() {
  webSocket.loop();
  buttons();
}
