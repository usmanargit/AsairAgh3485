# AsairAgh3485 Arduino Library

Library Arduino resmi dan komprehensif untuk sensor transmiter suhu dan kelembaban industri **ASAIR AGH3485** (dan kompatibel dengan seri AGH lainnya seperti AGH1485 / AGH4485) menggunakan protokol komunikasi **RS485 Modbus-RTU**.

Library ini dirancang sesuai standar **Arduino 1.5 Library Specification**, siap untuk dipublikasikan ke Arduino Library Manager, berbobot sangat ringan, dan tidak membutuhkan dependensi library Modbus eksternal (menggunakan kalkulasi CRC-16 Modbus internal).

---

## Fitur Utama

- **Zero External Dependencies**: Mengimplementasikan parser Modbus-RTU dan CRC-16 bawaan yang sangat efisien dan hemat memori.
- **Multi-Sensor Bus Support**: Mendukung banyak sensor dalam 1 bus RS485 (multi-drop topology) cukup dengan membedakan slave address.
- **Universal Stream Support**: Kompatibel dengan semua turunan `Stream` Arduino (`HardwareSerial`, `SoftwareSerial`, `AltSoftSerial`, dll.).
- **Automatic RS485 DE/RE Pin Control**: Mengontrol pin pengarah komunikasi Half-Duplex (DE & RE) secara otomatis dengan delay propagasi yang tepat. Mendukung modul auto-transceiver jika pin disetel ke `-1`.
- **Penanganan Suhu Negatif**: Mendukung pembacaan akurat di bawah 0°C (16-bit two's complement).
- **Pengaturan Device Dinamis**: Menyediakan fungsi bantuan untuk mengubah Slave Address dan Baud Rate sensor via Modbus holding register.
- **Diagnostik Error Lengkap**: Fungsi `getStatus()` dan `getStatusText()` untuk mempermudah pelacakan error (Timeout, CRC Mismatch, Modbus Exception, dll.).

---

## Pinout Sensor ASAIR AGH3485

| No. Pin | Warna Kabel | Nama Sinyal | Keterangan |
| :--- | :--- | :--- | :--- |
| **Pin 1** | Kuning / Yellow | **RS485 A+** | Jalur data non-inverting RS485 |
| **Pin 2** | Merah / Red | **VCC** | Tegangan input DC 9V s/d 36V (disarankan 12V / 24V) |
| **Pin 3** | Hitam / Black | **GND** | Ground (hubungkan bersama GND mikrokontroler/modul RS485) |
| **Pin 4** | Putih / White | **RS485 B-** | Jalur data inverting RS485 |

> [!WARNING]
> Sensor AGH3485 memerlukan suplai daya minimal **DC 9V** (maksimal 36V). Jangan menggunakan pin 5V Arduino untuk menyuplai sensor. Gunakan catu daya eksternal 12V / 24V dan sambungkan ground (GND) catu daya dengan GND Arduino.

---

## Skema Rangkaian (Wiring Diagram)

### 1. Arduino Uno / Nano (Menggunakan Modul MAX485 + SoftwareSerial)

```text
  [Sensor AGH3485]                  [Modul MAX485]                  [Arduino Uno/Nano]
  Pin 1 (Kuning / A+) ------------> Pin A
  Pin 4 (Putih / B-)  ------------> Pin B
  Pin 2 (Merah / VCC) ------------> (+) Catu Daya Eksternal (12V)
  Pin 3 (Hitam / GND) ------------> (-) GND Catu Daya ------------> Pin GND
                                    Pin VCC -----------------------> Pin 5V
                                    Pin GND -----------------------> Pin GND
                                    Pin RO  -----------------------> Pin D2 (SoftwareSerial RX)
                                    Pin DI  -----------------------> Pin D3 (SoftwareSerial TX)
                                    Pin DE  ----+------------------> Pin D4 (Direction Pin)
                                    Pin RE  ----+
```

### 2. Banyak Sensor (Multi-Drop RS485 Bus)

```text
                     Bus RS485 (Kabel twisted-pair)
  MAX485 Pin A ======+===================+===================+====== (Terminating Resistor 120Ω opsional)
                     |                   |                   |
               Sensor 1 (ID: 1)    Sensor 2 (ID: 2)    Sensor 3 (ID: 3)
                     |                   |                   |
  MAX485 Pin B ======+===================+===================+======
```

---

## Contoh Kode Program

Library menyediakan contoh sketsa di folder `examples/`:
1. **`BasicRead`**: Pembacaan 1 sensor tunggal.
2. **`MultipleSensors`**: Pembacaan beberapa sensor pada satu bus RS485.
3. **`ChangeAddress`**: Mengubah alamat ID Modbus sensor ke alamat baru.

### Contoh Multiple Sensors

```cpp
#include <Arduino.h>
#include <SoftwareSerial.h>
#include <AGH3485.h>

SoftwareSerial rs485(2, 3); // RX, TX
const int8_t DERE_PIN = 4;

// Buat array sensor dengan ID Modbus yang berbeda
AGH3485 sensors[] = {
  AGH3485(0x01), // Sensor 1
  AGH3485(0x02), // Sensor 2
  AGH3485(0x03)  // Sensor 3
};

const uint8_t TOTAL_SENSORS = sizeof(sensors) / sizeof(sensors[0]);

void setup() {
  Serial.begin(115200);
  rs485.begin(9600);

  for (uint8_t i = 0; i < TOTAL_SENSORS; i++) {
    sensors[i].begin(rs485, DERE_PIN);
    sensors[i].setTimeout(500);
  }
}

void loop() {
  for (uint8_t i = 0; i < TOTAL_SENSORS; i++) {
    Serial.print("Sensor ID ");
    Serial.print(sensors[i].getAddress());
    Serial.print(": ");

    if (sensors[i].read()) {
      Serial.print(sensors[i].getTemperature(), 1);
      Serial.print(" °C, ");
      Serial.print(sensors[i].getHumidity(), 1);
      Serial.println(" %RH");
    } else {
      Serial.println(sensors[i].getStatusText());
    }
    delay(50); // jeda antar sensor
  }
  delay(3000);
}
```

---

## Panduan Agar Terindeks di Arduino IDE Library Manager

Agar library ini dapat dicari dan diinstal langsung melalui menu **Tools > Manage Libraries...** di Arduino IDE, ikuti 5 langkah berikut:

### Langkah 1: Buat Repositori GitHub Publik
1. Buka [GitHub](https://github.com/) dan buat repositori baru dengan status **Public**.
2. Beri nama repositori sesuai nama library, misalnya: `AsairAgh3485`.
3. Push seluruh isi folder `AsairAgh3485` ke branch utama (biasanya `main`):
   ```bash
   git init
   git add .
   git commit -m "feat: Initial release of AsairAgh3485 library v1.0.0"
   git branch -M main
   git remote add origin https://github.com/usmanargit/AsairAgh3485.git
   git push -u origin main
   ```

### Langkah 2: Buat Git Release / Tag
Arduino Library Manager memindai versi berdasarkan **Git Tag**:
1. Di halaman GitHub repository Anda, buka tab **Releases** (atau jalankan via git CLI).
2. Klik **Create a new release** (atau `Draft a new release`).
3. Pada kolom tag, masukkan versi yang **sama persis** dengan yang ada di `library.properties`, yaitu `1.0.0` (atau `v1.0.0`).
4. Beri judul release `1.0.0` dan klik **Publish release**.

### Langkah 3: Daftarkan ke Arduino Library Registry
Arduino menggunakan issue tracker resmi untuk mendaftarkan library baru:
1. Buka tautan pendaftaran resmi berikut:  
   👉 [Arduino Library Registry - Add Library Issue](https://github.com/arduino/library-registry/issues/new?template=new_library.yml)
2. Isi formulir yang disediakan:
   - **Repository URL**: Masukkan link repository GitHub Anda, contoh:  
     `https://github.com/usmanargit/AsairAgh3485.git`
3. Klik tombol **Submit new issue**.

### Langkah 4: Pemindaian Otomatis oleh Bot Arduino
1. Bot Arduino (`arduino-libraries`) akan memvalidasi metadata `library.properties` Anda secara otomatis.
2. Jika semua aturan terpenuhi, bot akan memberikan komentar `Approved` dan menutup issue.
3. Dalam kurun waktu beberapa jam, library Anda akan masuk ke database indeks dan muncul di Library Manager Arduino IDE!

### Langkah 5: Rilis Pembaruan di Masa Depan
Jika Anda memperbarui kode di masa depan:
1. Naikkan nilai `version` di `library.properties` (misalnya `1.0.1`).
2. Push commit dan buat Git Tag/Release baru `1.0.1`.
3. Anda **tidak perlu** membuka issue lagi! Crawler Arduino akan mendeteksi rilis baru secara otomatis.

---

## Lisensi

Dilisensikan di bawah lisensi open-source [MIT License](LICENSE).  
Copyright (c) 2026 Usman AR.
