#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

RF24 radio(9, 10);  // CE, CSN
const byte address[6] = "00001";

// ECG pins
const int ecgPin = A0;
const int loPlus = 2;
const int loMinus = 3;

// 🔥 CHANGE THIS FOR EACH NODE (1, 2, 3)
byte nodeID = 2;

struct SensorPacket {
  byte nodeID;
  int ecgValue;
  unsigned long timestamp;
};

void setup() {
  Serial.begin(115200);
  Serial.print("Node ");
  Serial.print(nodeID);
  Serial.println(" ECG Sender");

  pinMode(loPlus, INPUT);
  pinMode(loMinus, INPUT);

  if (!radio.begin()) {
    Serial.println("nRF24 not found");
    while (1);
  }

  radio.setDataRate(RF24_250KBPS);
  radio.setPALevel(RF24_PA_LOW);
  radio.setChannel(108);
  radio.openWritingPipe(address);
  radio.stopListening();
}

void loop() {
  // Check electrode connection
  if (digitalRead(loPlus) == 1 || digitalRead(loMinus) == 1) {
    Serial.println("Electrodes not connected");
    delay(500);
    return;
  }

  int ecgValue = analogRead(ecgPin);

  // Create packet
  SensorPacket pkt;
  pkt.nodeID = nodeID;
  pkt.ecgValue = ecgValue;
  pkt.timestamp = millis();

  // Send
  radio.write(&pkt, sizeof(pkt));

  Serial.print("ECG: ");
  Serial.println(ecgValue);

  // 🔥 Different delay for each node (avoid collision)
  if (nodeID == 1) delay(50);
  else if (nodeID == 2) delay(70);
  else if (nodeID == 3) delay(90);
}