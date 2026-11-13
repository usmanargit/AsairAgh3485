/**
 * @file MultipleSensors.ino
 * @brief Example for reading multiple ASAIR AGH3485 sensors on a single RS485 Modbus bus.
 * 
 * In Modbus-RTU / RS485 multi-drop networks, all sensors share the same A(+) and B(-)
 * physical lines. Each sensor MUST have a unique Modbus Slave Address (e.g., 1, 2, 3...).
 * 
 * Hardware Connections:
 *  - Sensor 1: A+ -> MAX485 A, B- -> MAX485 B, Power -> 12V/24V, GND -> Common GND
 *  - Sensor 2: A+ -> MAX485 A, B- -> MAX485 B, Power -> 12V/24V, GND -> Common GND
 *  - Sensor 3: A+ -> MAX485 A, B- -> MAX485 B, Power -> 12V/24V, GND -> Common GND
 * 
 * MAX485 to Arduino Uno / Nano:
 *  - RO   -> Pin 2 (SoftwareSerial RX)
 *  - DI   -> Pin 3 (SoftwareSerial TX)
 *  - DE   -> Pin 4
 *  - RE   -> Pin 4 (connected to DE)
 *  - VCC  -> 5V
 *  - GND  -> GND
 */

#include <Arduino.h>
#include <AGH3485.h>

#if defined(ARDUINO_ARCH_AVR)
  #include <SoftwareSerial.h>
  const uint8_t RX_PIN = 2;
  const uint8_t TX_PIN = 3;
  SoftwareSerial rs485Serial(RX_PIN, TX_PIN);
  #define SENSOR_SERIAL rs485Serial
#elif defined(ESP32)
  HardwareSerial rs485Serial(2);
  const int8_t RX_PIN = 16;
  const int8_t TX_PIN = 17;
  #define SENSOR_SERIAL rs485Serial
#else
  #define SENSOR_SERIAL Serial1
#endif

const int8_t DERE_PIN = 4;

// Array of sensors with distinct Modbus slave IDs
AGH3485 sensors[] = {
  AGH3485(0x01),  // Sensor 1 (Alamat default 1)
  AGH3485(0x02),  // Sensor 2 (Alamat 2)
  AGH3485(0x03)   // Sensor 3 (Alamat 3)
};

const uint8_t TOTAL_SENSORS = sizeof(sensors) / sizeof(sensors[0]);

void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 3000) {}

  Serial.println(F("--- ASAIR AGH3485 Multiple Sensors Demo ---"));

  // Inisialisasi komunikasi RS485 (default 9600 baud)
#if defined(ARDUINO_ARCH_AVR)
  rs485Serial.begin(9600);
#elif defined(ESP32)
  rs485Serial.begin(9600, SERIAL_8N1, RX_PIN, TX_PIN);
#else
  SENSOR_SERIAL.begin(9600);
#endif

  // Inisialisasi setiap sensor ke serial stream dan pin DE/RE yang sama
  for (uint8_t i = 0; i < TOTAL_SENSORS; i++) {
    sensors[i].begin(SENSOR_SERIAL, DERE_PIN);
    // Timeout dapat disesuaikan (misal 500ms agar polling antar sensor lebih cepat)
    sensors[i].setTimeout(500);
  }

  Serial.print(F("Jumlah sensor terdaftar pada bus: "));
  Serial.println(TOTAL_SENSORS);
  Serial.println(F("==========================================="));
}

void loop() {
  for (uint8_t i = 0; i < TOTAL_SENSORS; i++) {
    uint8_t addr = sensors[i].getAddress();
    Serial.print(F("[Sensor ID "));
    Serial.print(addr);
    Serial.print(F("] -> "));

    // Baca data dari sensor
    if (sensors[i].read()) {
      float temp = sensors[i].getTemperature();
      float hum = sensors[i].getHumidity();

      Serial.print(F("Suhu: "));
      Serial.print(temp, 1);
      Serial.print(F(" °C | Kelembaban: "));
      Serial.print(hum, 1);
      Serial.println(F(" %RH"));
    } else {
      Serial.print(F("GAGAL! Alasan: "));
      Serial.println(sensors[i].getStatusText());
    }

    // Jeda antar-query ke bus RS485 (Modbus inter-frame silence)
    delay(100);
  }

  Serial.println(F("-------------------------------------------"));
  delay(3000); // Polling ulang setiap 3 detik
}

