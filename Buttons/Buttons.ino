/*
    Copyright (2017) Ben Kelley
    
    This file is part of buttonControlMQTT.

    buttonControlMQTT is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    buttonControlMQTT is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <PubSubClient.h>
#include <Ethernet.h>
#include <EthernetClient.h>
#include <elapsedMillis.h>

// Which pins the buttons are connected to
#define RED1 2
#define RED2 5
#define BLACK1 8
#define BLACK2 10

// Topic suffixes for each button
#define RED1_TOPIC "r1"
#define RED2_TOPIC "r2"
#define BLACK1_TOPIC "b1"
#define BLACK2_TOPIC "b2"

#define LED 13

// Bit mask for storing the current state
#define BLACK1_DOWN 1
#define BLACK2_DOWN 2
#define RED1_DOWN 4
#define RED2_DOWN 8

// Time in ms for short press and long press
// Press must be at least this long to be a short press
#define SHORT_PRESS 10
// Press must be at least this long to be a long press
#define LONG_PRESS 1000

// The IP address of your MQTT server
IPAddress mqttServer(10, 1, 1, 123);
// Login details for your MQTT server
#define mqttUser "myUser"
#define mqttPassword "s3cret!"

// Use this device name when connecting to MQTT
#define MQTT_DEVICE_NAME "switchbox"

#define BUTTON_UPDATE_TOPIC_BASE "myhome/switchbox/"

// The topic that Home Assistant uses to send status updates
#define HOME_ASSISTANT_STATUS_TOPIC "hass/status"

// EtherTen doesn't have a built in MAC address, so make one up
// Consult the docs for your ethernet hardware as to the best way to handle this
byte mac[] = {
  0x00, 0xAA, 0xBB, 0xCC, 0xDE, 0x02
};

EthernetClient ethClient;
PubSubClient mqttClient(ethClient);

elapsedMillis timeElapsed;

int lastEvent = 0;
int lastLongPress = 0;

int red1mode = HIGH;
int red2mode = HIGH;
int black1mode = HIGH;
int black2mode = HIGH;

int networkConnected = 0;

EthernetClient client;

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i=0;i<length;i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

void reconnect() {
  if (!mqttClient.connected()) {
    Serial.print("Connecting to MQTT...");
    // Attempt to connect
    if (mqttClient.connect(MQTT_DEVICE_NAME, mqttUser, mqttPassword)) {
      Serial.println("Connected to MQTT");
      mqttClient.subscribe(HOME_ASSISTANT_STATUS_TOPIC);
    } else {
      Serial.print("Failed to connect to MQTT");
      delay(10000);
    }
  }
}

void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  Serial.println("Thinking...");

  delay(500);
  Serial.println("Connecting to DHCP");
  // Set up input buttons with pullup resistors. Inputs go LOW when the button is pressed
  pinMode(RED1, INPUT_PULLUP);
  pinMode(RED2, INPUT_PULLUP);
  pinMode(BLACK1, INPUT_PULLUP);
  pinMode(BLACK2, INPUT_PULLUP);

  // Connect to network
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    digitalWrite(LED, HIGH);
    networkConnected = 0;
  } else {
    networkConnected = 1;
    Serial.println("Network connected. Happy.");
  }

  mqttClient.setServer(mqttServer, 1883);
  mqttClient.setCallback(mqttCallback);
  
  pinMode(LED, OUTPUT);
  digitalWrite(LED, LOW);
}

void sendButtonMessage(const char* topic, int longPress) {
  char buf[30];

  strcpy(buf, BUTTON_UPDATE_TOPIC_BASE);
  strcat(buf, topic);

  if (longPress) {
    mqttClient.publish(buf, "long");
  } else {
    mqttClient.publish(buf, "short");  
  }
}

void processButtonPress(int value, int button, const char* topic) {
   if (value == LOW) {
    // button pressed
    // Was it alredy pressed?
    if ((lastEvent & button) == button) {
      // Yes it was alreayd pressed
      // Is it a long press?
      if (timeElapsed > LONG_PRESS) {
        if ((lastLongPress & button) == 0) {
          // new long press
          lastLongPress |= button;
          Serial.print("L");
          Serial.print(button);
          sendButtonMessage(topic, 1);
        } else {
          // Long press, and you're still pressing it
        }
      } else {
        // You're holding the button down, but not long enough for a long press yet
      }
    } else {
      // You're newly pressing a button when it wasn't pressed before
      lastEvent = lastEvent | button;
      // start the timer to know if it was a long press
      timeElapsed = 0;
    }
  } else {
    // button is not pressed
    // Were we tracking a long press of this button?
    if ((lastLongPress & button) == 0) {
      // No, but was it previously pressed?
      if ((lastEvent & button) == button) {
        // Yes it was. You released after it was previously pressed
        // But was it pressed for long enough?
        if (timeElapsed < SHORT_PRESS) {
          // You didn't press it for long enough. e.g. bounce
          // Forget you ever pressed it
          lastEvent = 0;
        } else {
          // short press of detected
          Serial.print("S");
          Serial.print(button);
          lastEvent = 0;
          sendButtonMessage(topic, 0);
        }
      }
    } else {
      // You released it after a long press
      lastLongPress = 0;
      lastEvent = 0;
    }
  }
}

void loop() {
  red1mode = digitalRead(RED1);
  red2mode = digitalRead(RED2);
  black1mode = digitalRead(BLACK1);
  black2mode = digitalRead(BLACK2);
  
  processButtonPress(red1mode, RED1_DOWN, RED1_TOPIC);
  processButtonPress(red2mode, RED2_DOWN, RED2_TOPIC);
  processButtonPress(black1mode, BLACK1_DOWN, BLACK1_TOPIC);
  processButtonPress(black2mode, BLACK2_DOWN, BLACK2_TOPIC);

  if (networkConnected && !mqttClient.connected()) {
    reconnect();
  }
  if (networkConnected) {
      mqttClient.loop();
  }
}
