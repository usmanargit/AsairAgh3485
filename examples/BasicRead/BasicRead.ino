/**
 * @file BasicRead.ino
 * @brief Basic Example for ASAIR AGH3485 Modbus RS485 Sensor
 * 
 * Hardware Connections:
 * Sensor Pinout (typical AGH3485):
 *  - Pin 1 (Yellow): RS485 A+
 *  - Pin 2 (Red):    Power VCC (DC 9V - 36V, typically 12V/24V)
 *  - Pin 3 (Black):  GND (Common ground with Arduino / RS485 transceiver)
 *  - Pin 4 (White):  RS485 B-
 * 
 * RS485 Transceiver Module (e.g., MAX485) to Arduino Uno/Nano:
 *  - VCC  -> 5V
 *  - GND  -> GND
 *  - RO   -> Pin 2 (Arduino RX - SoftwareSerial)
 *  - DI   -> Pin 3 (Arduino TX - SoftwareSerial)
 *  - DE   -> Pin 4 (Direction Control - Driver Enable)
 *  - RE   -> Pin 4 (Tied to DE)
 *  - A    -> Sensor Pin 1 (Yellow)
 *  - B    -> Sensor Pin 4 (White)
 * 
 * For ESP32 / Mega / Leonardo / SAMD, HardwareSerial (e.g. Serial1 / Serial2) can be used directly!
 */

#include <Arduino.h>
#include <AGH3485.h>

#if defined(ARDUINO_ARCH_AVR)
  #include <SoftwareSerial.h>
  // Arduino Uno / Nano SoftwareSerial pins
  const uint8_t RX_PIN = 2;  // Connects to MAX485 RO
  const uint8_t TX_PIN = 3;  // Connects to MAX485 DI
  SoftwareSerial rs485Serial(RX_PIN, TX_PIN);
  #define SENSOR_SERIAL rs485Serial
#elif defined(ESP32)
  // ESP32 HardwareSerial
  HardwareSerial rs485Serial(2);
  const int8_t RX_PIN = 16;
  const int8_t TX_PIN = 17;
  #define SENSOR_SERIAL rs485Serial
#else
  // For other boards with Serial1
  #define SENSOR_SERIAL Serial1
#endif

// RS485 Direction pin (DE & RE connected together)
// Set to -1 if using an auto-switching RS485 transceiver module
const int8_t DERE_PIN = 4;

// Default Modbus address for AGH3485 is 0x01
AGH3485 sensor(AGH3485_DEFAULT_ADDRESS);

void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 3000) {
    // Wait for serial monitor on native USB boards
  }

  Serial.println(F("--- ASAIR AGH3485 Sensor Basic Read Demo ---"));

  // Start RS485 communication (default factory baud rate is 9600)
#if defined(ARDUINO_ARCH_AVR)
  rs485Serial.begin(9600);
#elif defined(ESP32)
  rs485Serial.begin(9600, SERIAL_8N1, RX_PIN, TX_PIN);
#else
  SENSOR_SERIAL.begin(9600);
#endif

  // Initialize AGH3485 library with the stream and DE/RE control pin
  if (!sensor.begin(SENSOR_SERIAL, DERE_PIN)) {
    Serial.println(F("[ERROR] Failed to initialize AGH3485 sensor!"));
  } else {
    Serial.println(F("[OK] AGH3485 sensor initialized successfully."));
  }
}

void loop() {
  Serial.println(F("Querying sensor..."));

  // Read data from sensor
  if (sensor.read()) {
    float temperature = sensor.getTemperature();
    float humidity = sensor.getHumidity();

    Serial.print(F("Temperature: "));
    Serial.print(temperature, 1);
    Serial.print(F(" °C | Humidity: "));
    Serial.print(humidity, 1);
    Serial.println(F(" %RH"));
  } else {
    // Print failure diagnostic
    Serial.print(F("[FAILED] Reason: "));
    Serial.println(sensor.getStatusText());
  }

  Serial.println(F("-----------------------------------------"));
  delay(2000);  // Read every 2 seconds
}

