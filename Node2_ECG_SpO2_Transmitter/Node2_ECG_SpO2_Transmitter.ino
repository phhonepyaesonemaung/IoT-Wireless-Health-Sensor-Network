#include <Wire.h>
#include "MAX30105.h"      // SparkFun MAX3010x library
#include "heartRate.h"     // From SparkFun library
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

MAX30105 particleSensor;
const byte RATE_SIZE = 4;  // Increase for smoother HR

RF24 radio(9, 10);  // CE, CSN
const byte address[6] = "00001";

struct SensorPacket {
  byte nodeID = 2;         // Node 2
  int heartRate;           // BPM
  int spo2;                // % oxygen
  unsigned long timestamp;
};

sensors_event_t irEvent, redEvent;
int rates[RATE_SIZE]; 
byte rateSpot = 0;
long lastBeatTime = 0;
bool ratesTimerReady = false;
long beatsPerMinute;
int beatAvg = 0;

void setup() {
  Serial.begin(115200);
  Serial.println("Node 2: MAX30102 + nRF24L01");

  // MAX30102 init
  if (!particleSensor.begin()) {
    Serial.println("MAX30102 not found");
    while (1);
  }
  particleSensor.setup();  // default settings
  particleSensor.setPulseAmplitudeRed(0x0A);  // lower LED power

  // nRF24 init
  if (!radio.begin()) {
    Serial.println("nRF24 not found");
    while (1);
  }
  radio.setDataRate(RF24_250KBPS);
  radio.setPALevel(RF24_PA_LOW);
  radio.setChannel(108);
  radio.setRetries(5, 15);
  radio.openWritingPipe(address);
  radio.stopListening();

  Serial.println("Node 2 Ready");
}

void loop() {
  long irValue = particleSensor.getIR();
  if (irValue < 50000) {
    Serial.println("Finger not detected");
    delay(100);
    return;
  }

  // Heart rate calculation
  if (checkForBeat(irValue) == true) {
    long delta = millis() - lastBeatTime;
    lastBeatTime = millis();
    beatsPerMinute = 60 / (delta / 1000.0);
    
    if (beatsPerMinute < 255 && beatsPerMinute > 20) {
      rates[rateSpot++] = (byte)beatsPerMinute;
      rateSpot %= RATE_SIZE;
      
      beatAvg = 0;
      for (byte i = 0; i < RATE_SIZE; i++)
        beatAvg += rates[i];
      beatAvg /= RATE_SIZE;
    }
  }

  // SpO2 calculation (simplified ratio)
  particleSensor.setLEDMode(2);
  long redValue = particleSensor.getRed();
  int spo2 = map(redValue / (irValue / 100), 0, 255, 85, 100);  // calibrated range
  if (spo2 > 100) spo2 = 100;
  if (spo2 < 80) spo2 = 80;

  // Pack and transmit
  SensorPacket pkt;
  pkt.heartRate = beatAvg;
  pkt.spo2 = spo2;
  pkt.timestamp = millis();

  bool ok = radio.write(&pkt, sizeof(pkt));
  if (ok) {
    Serial.print("Node2 HR:");
    Serial.print(pkt.heartRate);
    Serial.print(" SpO2:");
    Serial.print(pkt.spo2);
    Serial.println("%");
  } else {
    Serial.println("TX failed");
  }

  delay(1000);
}
