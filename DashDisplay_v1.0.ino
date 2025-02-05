//////////////////////////////////////////////
// FILE: DashDisplay_v1.0.ino
//
// AUTHOR: rfesler@gmail.com, 2025
//
// PROJECT: MQTT Display for Vechile.
//
// DESCRIPTION:  ESP01S connecting to a MQTT Server via WiFi.
// Data being Published is incoming Solar, Shunt Load, and Altitude.
// Vechicle is utilized as a full-time living space. RPI 5
// running Rasparian, Mosquitto, and Node-Red. Display (SDD1306)
// updated via I2C.
// 
// TO-DO:
// * Publish to MQTT when device is online
// * Some type of sleep or low-power mode
// * Dynamic publication to device for user selected data
// * or, rotory encoder to select data?
// * 
// REVISIONS:
// v
// v1.00 - 02FEB2025, In-Service
// v0.40 - 04OCT2024, Scrolling, addition subscriptions
// v0.30 - 10NOV2024, WiFi/MQTT stablilty
// v0.20 - 23SEP2024, Recieving MQTT data
// v0.10 - 22SEP2024, Concept
/////////////////////////////////////////////////

#include <Wire.h>
#include <U8g2lib.h>
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// WiFi and MQTT Configuration
const char* ssid = "ChefRichInaVan";
const char* password = "--------";
const char* mqtt_server = "192.168.1.2";
const int mqtt_port = 1883;

#define SCL_OLED 0
#define SDA_OLED 2

// Initialize the OLED display with U8G2 for software I2C
U8G2_SSD1306_128X64_NONAME_1_SW_I2C OLED(U8G2_R0, SCL_OLED, SDA_OLED, /*reset=*/U8X8_PIN_NONE);

char altitudeStr[8] = "0";  // Allocate extra space for the "'" character
char solarStr[6] = "0";
char shuntStr[8] = "0";  // Allocate space for optional "+" sign

int currentTopicIndex = 0;
unsigned long lastDisplayUpdate = 0;
const unsigned long displayInterval = 5000;

WiFiClient espClient;
PubSubClient client(espClient);

// Function to connect to WiFi
void setup_wifi() {
  delay(10);
  Serial.begin(115200);
  Serial.print("Connecting to ");
  Serial.println(ssid);

  OLED.begin();
  OLED.setFont(u8g2_font_t0_12b_mf);  // Use the same font as in data display
  OLED.clearBuffer();
  OLED.drawStr(0, 8, "Connecting to WiFi...");  // Move up by setting y = 8 for better fit
  OLED.sendBuffer();

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }

  Serial.println("WiFi connected");
  OLED.clearBuffer();
  OLED.drawStr(0, 8, "WiFi Connected");  // Keep y = 8 for consistent fit
  OLED.sendBuffer();
  delay(1000);
}

// Function to handle incoming MQTT messages
void callback(char* topic, byte* payload, unsigned int length) {
  String message = "";
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }

  if (strcmp(topic, "DustyMQTT/Altitude") == 0) {
    snprintf(altitudeStr, sizeof(altitudeStr), "%d", (int)message.toFloat());  // Append "'" to altitude
  } else if (strcmp(topic, "DustyMQTT/Solar") == 0) {
    snprintf(solarStr, sizeof(solarStr), "%d", (int)message.toFloat());
  } else if (strcmp(topic, "DustyMQTT/Shunt") == 0) {
    float shuntValue = message.toFloat();
    if (shuntValue >= 0) {
      snprintf(shuntStr, sizeof(shuntStr), "+%d", (int)shuntValue);  // Add "+" for positive values
    } else {
      snprintf(shuntStr, sizeof(shuntStr), "%d", (int)shuntValue);   // Display normally for negatives
    }
  }
}

// Function to connect to MQTT broker
void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    OLED.clearBuffer();
    OLED.setFont(u8g2_font_t0_12b_mf);  // Same font as in data display
    OLED.drawStr(0, 8, "Connecting to MQTT...");  // Move y up to 8
    OLED.sendBuffer();

    if (client.connect("ESP8266Client")) {
      Serial.println("connected");
      OLED.clearBuffer();
      OLED.drawStr(0, 8, "MQTT Connected");  // Keep y = 8 for better fit
      OLED.sendBuffer();
      delay(1000);

      client.subscribe("DustyMQTT/Altitude");
      client.subscribe("DustyMQTT/Solar");
      client.subscribe("DustyMQTT/Shunt");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      OLED.clearBuffer();
      OLED.drawStr(0, 8, "MQTT Failed, retrying...");  // Keep y = 8
      OLED.sendBuffer();
      delay(5000);
    }
  }
}

// Slide-in effect for displaying data
void slideInDisplay(const char* label, const char* data) {
  int start_x = 128;
  int data_width = OLED.getStrWidth(data);  // Get the width of the entire data string
  int end_x = (128 - data_width) / 2;       // Calculate the center position for the data

  OLED.firstPage();
  do {
    // Font and position for the label
    OLED.setFont(u8g2_font_t0_12b_mf);
    int16_t label_width = OLED.getStrWidth(label);
    int16_t x_label = (128 - label_width) / 2;
    OLED.drawStr(x_label, 12, label);

    // Font for the value
    OLED.setFont(u8g2_font_fur30_tf);

    // Draw the entire value (including + or -) and center it
    OLED.drawStr(end_x, 50, data);

  } while (OLED.nextPage());
}

// Function to display the current topic data with animation
void displayCurrentTopic() {
  if (millis() - lastDisplayUpdate > displayInterval) {
    Serial.print("Displaying topic index: ");
    Serial.println(currentTopicIndex);

    switch (currentTopicIndex) {
      case 0:
        slideInDisplay("Altitude (ft)", altitudeStr);
        break;
      case 1:
        slideInDisplay("Solar (W)", solarStr);
        break;
      case 2:
        slideInDisplay("Shunt (W)", shuntStr);
        break;
    }

    currentTopicIndex = (currentTopicIndex + 1) % 3;
    lastDisplayUpdate = millis();
  }
}

void setup() {
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
  client.subscribe("DustyMQTT/Altitude");
  client.subscribe("DustyMQTT/Solar");
  client.subscribe("DustyMQTT/Shunt");
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  displayCurrentTopic();
}
