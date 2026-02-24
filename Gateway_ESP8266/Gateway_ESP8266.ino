#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ThingSpeak.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

const char* ssid = "YOUR_WIFI_SSID";      // ← your WiFi
const char* password = "YOUR_WIFI_PASS";

WiFiClient client;
unsigned long channelID = 2659427;        // ← your ThingSpeak channel
const char* writeAPIKey = "CSMG85BA8377Z1MW";  // ← your API key

#define CE_PIN D2
#define CSN_PIN D8
RF24 radio(CE_PIN, CSN_PIN);
const byte address[6] = "00001";

struct SensorPacket {
  byte nodeID;
  int heartRate;
  int spo2;
  unsigned long timestamp;
};

unsigned long lastUpdate = 0;
const unsigned long updateDelay = 20000;  // 20s between uploads

void setup() {
  Serial.begin(115200);
  Serial.println("ECG/O2 Gateway Starting...");

  // nRF24
  if (!radio.begin()) {
    Serial.println("nRF24 failed");
    while (1);
  }
  radio.setDataRate(RF24_250KBPS);
  radio.setPALevel(RF24_PA_LOW);
  radio.setChannel(108);
  radio.openReadingPipe(0, address);
  radio.startListening();

  // WiFi + ThingSpeak
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected!");
  ThingSpeak.begin(client);

  Serial.println("Gateway Ready - Fields: 1=Node1HR,2=Node1SpO2,3=Node2HR,4=Node2SpO2");
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi lost - reconnecting...");
    WiFi.begin(ssid, password);
    delay(5000);
    return;
  }

  if (radio.available()) {
    SensorPacket pkt;
    radio.read(&pkt, sizeof(pkt));

    Serial.print("Node ");
    Serial.print(pkt.nodeID);
    Serial.print(" HR:");
    Serial.print(pkt.heartRate);
    Serial.print(" BPM, SpO2:");
    Serial.print(pkt.spo2);
    Serial.println("%");

    // Buffer data and upload periodically
    unsigned long now = millis();
    if (now - lastUpdate > updateDelay) {
      if (pkt.nodeID == 1) {
        ThingSpeak.setField(1, pkt.heartRate);
        ThingSpeak.setField(2, pkt.spo2);
        ThingSpeak.writeFields(channelID, writeAPIKey);
        Serial.println("Node1 data uploaded");
      } else if (pkt.nodeID == 2) {
        ThingSpeak.setField(3, pkt.heartRate);
        ThingSpeak.setField(4, pkt.spo2);
        ThingSpeak.writeFields(channelID, writeAPIKey);
        Serial.println("Node2 data uploaded");
      }
      lastUpdate = now;
    }
  }
  delay(100);
}
