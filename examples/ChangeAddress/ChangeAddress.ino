/**
 * @file ChangeAddress.ino
 * @brief Utility sketch to change the Modbus Slave Address of an ASAIR AGH3485 sensor.
 * 
 * IMPORTANT:
 * Connect ONLY ONE sensor to the RS485 bus when running this sketch.
 * Once changed, the new address is saved in the sensor's non-volatile memory.
 */

#include <Arduino.h>
#include <AGH3485.h>

#if defined(ARDUINO_ARCH_AVR)
  #include <SoftwareSerial.h>
  SoftwareSerial rs485Serial(2, 3); // RX=2, TX=3
  #define SENSOR_SERIAL rs485Serial
#elif defined(ESP32)
  HardwareSerial rs485Serial(2);
  #define SENSOR_SERIAL rs485Serial
#else
  #define SENSOR_SERIAL Serial1
#endif

const int8_t DERE_PIN = 4;

// Alamat lama (default pabrik: 0x01)
const uint8_t CURRENT_ADDRESS = 0x01;

// Alamat baru yang diinginkan (rentang: 1 s/d 247)
const uint8_t NEW_ADDRESS = 0x02;

AGH3485 sensor(CURRENT_ADDRESS);

void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 3000) {}

  Serial.println(F("--- Konfigurasi Alamat Modbus ASAIR AGH3485 ---"));
  Serial.println(F("PERINGATAN: Pastikan hanya ada SATU sensor terpasang pada bus!"));

#if defined(ARDUINO_ARCH_AVR)
  rs485Serial.begin(9600);
#elif defined(ESP32)
  rs485Serial.begin(9600, SERIAL_8N1, 16, 17);
#else
  SENSOR_SERIAL.begin(9600);
#endif

  sensor.begin(SENSOR_SERIAL, DERE_PIN);

  // Verifikasi apakah sensor merespon di alamat lama
  Serial.print(F("Mencoba membaca sensor di alamat lama (0x"));
  Serial.print(CURRENT_ADDRESS, HEX);
  Serial.print(F(")... "));

  if (sensor.read()) {
    Serial.println(F("Ditemukan!"));
    Serial.print(F("Suhu saat ini: "));
    Serial.print(sensor.getTemperature(), 1);
    Serial.println(F(" °C"));

    Serial.print(F("Mengubah alamat menjadi 0x"));
    Serial.print(NEW_ADDRESS, HEX);
    Serial.print(F("... "));

    if (sensor.setDeviceAddress(NEW_ADDRESS)) {
      Serial.println(F("BERHASIL!"));
      Serial.println(F("Menguji pembacaan dengan alamat baru..."));
      delay(500);

      // Verifikasi pembacaan dengan alamat baru
      if (sensor.read()) {
        Serial.print(F("Sukses! Sensor kini merespon di alamat: 0x"));
        Serial.println(sensor.getAddress(), HEX);
      } else {
        Serial.println(F("Catatan: Silakan reset daya sensor jika belum merespon alamat baru."));
      }
    } else {
      Serial.print(F("Gagal mengubah alamat: "));
      Serial.println(sensor.getStatusText());
    }
  } else {
    Serial.println(F("Gagal! Sensor tidak merespon di alamat tersebut."));
    Serial.println(sensor.getStatusText());
  }
}

void loop() {
  // Tidak ada proses berulang
}

