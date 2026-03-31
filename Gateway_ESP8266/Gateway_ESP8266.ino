#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ThingSpeak.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

// WiFi
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

// ThingSpeak
WiFiClient client;
unsigned long channelID = 2659427;
const char* writeAPIKey = "YOUR_API_KEY";

// nRF24
#define CE_PIN D2
#define CSN_PIN D8
RF24 radio(CE_PIN, CSN_PIN);
const byte address[6] = "00001";

// SAME struct as sender
struct SensorPacket {
  byte nodeID;
  int ecgValue;
  unsigned long timestamp;
};

// Store values from 3 nodes
int ecg1 = 0;
int ecg2 = 0;
int ecg3 = 0;

unsigned long lastUpdate = 0;
const unsigned long updateDelay = 20000; // 20 sec

void setup() {
  Serial.begin(115200);
  Serial.println("ESP8266 Gateway Starting...");

  // nRF24 setup
  if (!radio.begin()) {
    Serial.println("nRF24 failed");
    while (1);
  }

  radio.setDataRate(RF24_250KBPS);
  radio.setPALevel(RF24_PA_LOW);
  radio.setChannel(108);
  radio.openReadingPipe(0, address);
  radio.startListening();

  // WiFi connect
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi connected!");
  ThingSpeak.begin(client);
}

void loop() {
  // Reconnect WiFi if needed
  if (WiFi.status() != WL_CONNECTED) {
    WiFi.begin(ssid, password);
    delay(5000);
    return;
  }

  // Receive data
  if (radio.available()) {
    SensorPacket pkt;
    radio.read(&pkt, sizeof(pkt));

    Serial.print("Node ");
    Serial.print(pkt.nodeID);
    Serial.print(" ECG: ");
    Serial.println(pkt.ecgValue);

    if (pkt.nodeID == 1) ecg1 = pkt.ecgValue;
    else if (pkt.nodeID == 2) ecg2 = pkt.ecgValue;
    else if (pkt.nodeID == 3) ecg3 = pkt.ecgValue;
  }

  // Upload every 20 seconds
  unsigned long now = millis();
  if (now - lastUpdate > updateDelay) {

    ThingSpeak.setField(1, ecg1);
    ThingSpeak.setField(2, ecg2);
    ThingSpeak.setField(3, ecg3);

    int response = ThingSpeak.writeFields(channelID, writeAPIKey);

    if (response == 200) {
      Serial.println("Upload success");
    } else {
      Serial.print("Upload failed: ");
      Serial.println(response);
    }

    lastUpdate = now;
  }

  delay(100);
}