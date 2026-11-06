#include <Arduino.h>
#include <NimBLEDevice.h>

// ==========================================
// MOTORCYCLE TPMS CONFIGURATION
// ==========================================

#define FRONT_MAC "b9:41:fa:00:46:64"
#define REAR_MAC  "b9:41:fa:00:48:81"

class TPMSCallbacks : public NimBLEScanCallbacks {
  void onResult(const NimBLEAdvertisedDevice* device) override {
    String mac = String(device->getAddress().toString().c_str());
    mac.toLowerCase();

    String position = "";
    if (mac == FRONT_MAC) position = "FRONT";
    else if (mac == REAR_MAC) position = "REAR";
    else return;

    if (device->haveManufacturerData()) {
      std::string data = device->getManufacturerData();
      if (data.length() < 11) return;

      Serial.println("======================================");
      Serial.print("TPMS: "); Serial.println(position);
      
      // Print raw bytes for verification
      Serial.print("RAW BYTES: ");
      for (size_t i = 0; i < data.length(); i++) {
        Serial.printf("%02X ", (uint8_t)data[i]);
      }
      Serial.println();

      // ==================================
      // DECODING PAYLOAD
      // ==================================
      uint16_t rawPressure = (uint8_t)data[2] | ((uint8_t)data[3] << 8);
      uint8_t  rawTempByte = (uint8_t)data[7]; 
      uint16_t rawVoltage  = (uint8_t)data[9] | ((uint8_t)data[10] << 8);

      // Conversions
      float pressurePsi = (float)rawPressure / 7000.0f; // Locked in scale for low-pressure/testing
      float tempC       = (float)rawTempByte - 45.0f; 
      float voltage     = 3.00f; // Locked in coin cell baseline (~3.0V)

      Serial.printf("  -> PRESSURE : %.2f PSI (Raw: 0x%04X)\n", pressurePsi, rawPressure);
      Serial.printf("  -> TEMP     : %.2f °C (Raw: %u)\n", tempC, rawTempByte);
      Serial.printf("  -> VOLTAGE  : ~%.2f V (Raw: 0x%04X)\n", voltage, rawVoltage);
      Serial.println("======================================");
    }
  }
};

TPMSCallbacks callbacks;

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("TPMS DECODER STARTED...");

  NimBLEDevice::init("");
  NimBLEScan* pBLEScan = NimBLEDevice::getScan();
  pBLEScan->setScanCallbacks(&callbacks, false);
  pBLEScan->setActiveScan(false);
  pBLEScan->setInterval(97);
  pBLEScan->setWindow(97);
  pBLEScan->start(0, false);
}

void loop() {
  delay(1000);
  NimBLEDevice::getScan()->clearResults();
}