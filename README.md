# IoT-Based Wireless Health Sensor Network

A real-time wireless health monitoring system using Arduino sensor nodes with MAX30102 (ECG/Heart Rate + SpO2) sensors transmitting data via nRF24L01 radio modules to an ESP8266 gateway that uploads to ThingSpeak cloud.

## 🩺 System Overview

- **Node 1 & Node 2**: Arduino UNO boards with MAX30102 sensors measuring heart rate and blood oxygen saturation (SpO2)
- **Gateway**: ESP8266 receives wireless data from both nodes via nRF24L01 and uploads to ThingSpeak cloud platform
- **Communication**: 2.4GHz nRF24L01 wireless modules (250Kbps, channel 108)
- **Cloud Platform**: ThingSpeak (Node1 → Fields 1-2, Node2 → Fields 3-4)

## 📦 Hardware Components

### Per Sensor Node (×2)
- Arduino UNO
- MAX30102 Pulse Oximeter & Heart Rate Sensor
- nRF24L01 Wireless Transceiver Module
- **10µF capacitor** (across nRF24L01 VCC-GND for stable power)
- Jumper wires

### Gateway
- ESP8266 (NodeMCU or Wemos D1 Mini)
- nRF24L01 Wireless Transceiver Module
- **10µF capacitor** (across nRF24L01 VCC-GND)
- Jumper wires

## 🔌 Wiring Diagrams

### Arduino UNO (Node 1 & Node 2) Connections

#### MAX30102 Sensor
```
MAX30102    →    Arduino UNO
─────────────────────────────
VCC         →    3.3V
GND         →    GND
SDA         →    A4 (I2C Data)
SCL         →    A5 (I2C Clock)
```

#### nRF24L01 Radio Module
```
nRF24L01    →    Arduino UNO
─────────────────────────────
VCC         →    5V ⚡ (IMPORTANT: Use 5V for Arduino)
GND         →    GND
CE          →    Digital Pin 9
CSN         →    Digital Pin 10
SCK         →    Digital Pin 13 (SPI Clock)
MOSI        →    Digital Pin 11 (SPI Master Out)
MISO        →    Digital Pin 12 (SPI Master In)
```

> **⚠️ CRITICAL NOTE**: Connect nRF24L01 VCC to **5V** on Arduino UNO. Add a 10µF capacitor between VCC and GND pins of the nRF24L01 module for voltage stability.

---

### ESP8266 Gateway Connections

#### nRF24L01 Radio Module
```
nRF24L01    →    ESP8266 (NodeMCU)
──────────────────────────────────
VCC         →    3.3V ⚡ (IMPORTANT: Use 3.3V for ESP8266)
GND         →    GND
CE          →    D2 (GPIO4)
CSN         →    D8 (GPIO15)
SCK         →    D5 (GPIO14 - SPI Clock)
MOSI        →    D7 (GPIO13 - SPI MOSI)
MISO        →    D6 (GPIO12 - SPI MISO)
```

> **⚠️ CRITICAL NOTE**: Connect nRF24L01 VCC to **3.3V** on ESP8266 (NOT 5V!). Add a 10µF capacitor between VCC and GND pins of the nRF24L01 module. ESP8266 GPIO pins are NOT 5V tolerant.

## 📚 Required Libraries

### For Arduino Nodes
Install via Arduino IDE → Library Manager:
- `SparkFun MAX3010x Sensor Library` by SparkFun
- `RF24` by TMRh20

### For ESP8266 Gateway
- `ESP8266WiFi` (included with ESP8266 board support)
- `ThingSpeak` by MathWorks
- `RF24` by TMRh20

## 🚀 Setup Instructions

### 1. Arduino Nodes Configuration
1. Open `Node1_ECG_SpO2_Transmitter.ino` in Arduino IDE
2. Upload to first Arduino UNO
3. Open `Node2_ECG_SpO2_Transmitter.ino`
4. Upload to second Arduino UNO
5. Both nodes will automatically start transmitting when a finger is detected on MAX30102

### 2. ESP8266 Gateway Configuration
1. Open `Gateway_ESP8266.ino`
2. **Edit WiFi credentials**:
   ```cpp
   const char* ssid = "YOUR_WIFI_SSID";
   const char* password = "YOUR_WIFI_PASS";
   ```
3. **Edit ThingSpeak credentials**:
   ```cpp
   unsigned long channelID = YOUR_CHANNEL_ID;
   const char* writeAPIKey = "YOUR_WRITE_API_KEY";
   ```
4. Select **Board: NodeMCU 1.0 (ESP-12E Module)** or your ESP8266 variant
5. Upload to ESP8266

### 3. ThingSpeak Channel Setup
1. Create a ThingSpeak account at [thingspeak.com](https://thingspeak.com)
2. Create a new channel with 4 fields:
   - **Field 1**: Node 1 Heart Rate (BPM)
   - **Field 2**: Node 1 SpO2 (%)
   - **Field 3**: Node 2 Heart Rate (BPM)
   - **Field 4**: Node 2 SpO2 (%)
3. Copy Channel ID and Write API Key to gateway code

## 📊 Data Flow

```
[Arduino Node 1] ──(nRF24L01)──┐
   MAX30102                      │
                                 ├──> [ESP8266 Gateway] ──(WiFi)──> [ThingSpeak Cloud]
[Arduino Node 2] ──(nRF24L01)──┘
   MAX30102
```

## 🔍 Serial Monitor Output

### Node Output (115200 baud)
```
Node1 HR:75 SpO2:98%
Node1 HR:76 SpO2:97%
Finger not detected
```

### Gateway Output (115200 baud)
```
WiFi connected!
Gateway Ready - Fields: 1=Node1HR,2=Node1SpO2,3=Node2HR,4=Node2SpO2
Node 1 HR:75 BPM, SpO2:98%
Node1 data uploaded
Node 2 HR:82 BPM, SpO2:96%
Node2 data uploaded
```

## ⚙️ System Parameters

- **RF Channel**: 108 (2.4GHz)
- **Data Rate**: 250 Kbps
- **Power Level**: Low (RF24_PA_LOW)
- **Transmission Interval**: 1 second per node
- **Cloud Upload Interval**: 20 seconds
- **Address**: "00001"

## 🛠️ Troubleshooting

| Issue | Solution |
|-------|----------|
| "NRF not found" error | Check nRF24L01 wiring, add 10µF capacitor, ensure correct voltage (5V for Arduino, 3.3V for ESP8266) |
| "MAX30102 not found" | Verify I2C connections (SDA/SCL), ensure 3.3V power supply |
| "WiFi lost" messages | Check WiFi credentials, move gateway closer to router |
| "Finger not detected" | Ensure proper finger placement on MAX30102 sensor, check sensor power |
| Upload fails to ThingSpeak | Verify API key and Channel ID, check 15-second rate limit |
| No data received at gateway | Ensure both nodes and gateway use same address ("00001"), check radio channel (108) |

## 📝 Key Features

- ✅ Real-time heart rate monitoring (BPM)
- ✅ Blood oxygen saturation (SpO2%) measurement
- ✅ Wireless transmission up to 100m (line of sight)
- ✅ Dual-node support for multi-patient monitoring
- ✅ Cloud data storage and visualization
- ✅ Auto-reconnect WiFi on connection loss
- ✅ Timestamped data packets

## 🔐 Security Notes

- Store WiFi and API credentials securely
- Use ThingSpeak private channels for sensitive health data
- Consider implementing packet encryption for production deployments

## 📄 License

MIT License - Free for educational and commercial use

## 👨‍💻 Author

**Phone Pyae Sone Maung**  
Electronic and Communication Engineering Student  
[GitHub Profile](https://github.com/phhonepyaesonemaung)

## 🤝 Contributing

Pull requests welcome! For major changes, please open an issue first.

---

**⭐ If this project helped you, please star the repository!**
