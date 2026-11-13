/**
 * @file AGH3485.cpp
 * @brief Implementation of Arduino Library for ASAIR AGH3485 Modbus RS485 Sensor
 * @author Usman AR (usmanar.tsm@gmail.com)
 * @license MIT
 */

#include "AGH3485.h"

AGH3485::AGH3485(uint8_t slaveAddress)
    : _serial(nullptr),
      _derePin(-1),
      _slaveAddress(slaveAddress),
      _functionCode(AGH3485_FC_READ_HOLDING),
      _timeoutMs(AGH3485_DEFAULT_TIMEOUT),
      _temperature(0.0f),
      _humidity(0.0f),
      _rawTemperature(0),
      _rawHumidity(0),
      _status(AGH3485_SUCCESS) {}

bool AGH3485::begin(Stream &stream, int8_t derePin) {
    _serial = &stream;
    _derePin = derePin;

    if (_derePin >= 0) {
        pinMode(_derePin, OUTPUT);
        setRxMode();
    }

    _status = AGH3485_SUCCESS;
    return true;
}

bool AGH3485::read() {
    if (!_serial) {
        _status = AGH3485_ERR_NOT_INITIALIZED;
        return false;
    }

    // Modbus RTU Read Request:
    // [0]: Slave Address
    // [1]: Function Code (0x03 or 0x04)
    // [2]: Start Register Address High (0x00)
    // [3]: Start Register Address Low  (0x00)
    // [4]: Register Count High (0x00)
    // [5]: Register Count Low  (0x02 - Humidity & Temperature)
    // [6]: CRC Low
    // [7]: CRC High
    uint8_t request[8];
    request[0] = _slaveAddress;
    request[1] = _functionCode;
    request[2] = (AGH3485_REG_HUMIDITY >> 8) & 0xFF;
    request[3] = AGH3485_REG_HUMIDITY & 0xFF;
    request[4] = 0x00;
    request[5] = 0x02;

    uint16_t crc = calculateCRC(request, 6);
    request[6] = (uint8_t)(crc & 0xFF);
    request[7] = (uint8_t)((crc >> 8) & 0xFF);

    if (!sendCommand(request, sizeof(request))) {
        _status = AGH3485_ERR_NOT_INITIALIZED;
        return false;
    }

    // Expected Response Frame (9 bytes):
    // [0]: Slave Address
    // [1]: Function Code (0x03 or 0x04)
    // [2]: Byte Count (0x04)
    // [3..4]: Humidity High, Low
    // [5..6]: Temperature High, Low
    // [7..8]: CRC Low, High
    uint8_t response[16];
    size_t received = receiveResponse(response, sizeof(response), 9);

    if (received < 5) {
        _status = AGH3485_ERR_TIMEOUT;
        return false;
    }

    // Check device address
    if (response[0] != _slaveAddress) {
        _status = AGH3485_ERR_INVALID_RESPONSE;
        return false;
    }

    // Check for Modbus Exception response (Function Code | 0x80)
    if (response[1] & 0x80) {
        _status = AGH3485_ERR_EXCEPTION;
        return false;
    }

    if (response[1] != _functionCode || response[2] != 4 || received < 9) {
        _status = AGH3485_ERR_INVALID_RESPONSE;
        return false;
    }

    // Validate CRC
    uint16_t receivedCRC = (uint16_t)response[7] | ((uint16_t)response[8] << 8);
    uint16_t calculatedCRC = calculateCRC(response, 7);
    if (receivedCRC != calculatedCRC) {
        _status = AGH3485_ERR_CRC;
        return false;
    }

    // Parse registers
    _rawHumidity = (uint16_t)((response[3] << 8) | response[4]);
    _rawTemperature = (int16_t)((response[5] << 8) | response[6]);

    _humidity = (float)_rawHumidity / 10.0f;
    _temperature = (float)_rawTemperature / 10.0f;

    _status = AGH3485_SUCCESS;
    return true;
}

bool AGH3485::read(float &temperature, float &humidity) {
    if (read()) {
        temperature = _temperature;
        humidity = _humidity;
        return true;
    }
    return false;
}

float AGH3485::getTemperature() const {
    return _temperature;
}

float AGH3485::getHumidity() const {
    return _humidity;
}

int16_t AGH3485::getRawTemperature() const {
    return _rawTemperature;
}

uint16_t AGH3485::getRawHumidity() const {
    return _rawHumidity;
}

AGH3485_Status AGH3485::getStatus() const {
    return _status;
}

const char* AGH3485::getStatusText() const {
    switch (_status) {
        case AGH3485_SUCCESS:
            return "OK";
        case AGH3485_ERR_TIMEOUT:
            return "Response Timeout";
        case AGH3485_ERR_CRC:
            return "CRC Checksum Error";
        case AGH3485_ERR_INVALID_RESPONSE:
            return "Invalid Response Frame";
        case AGH3485_ERR_EXCEPTION:
            return "Modbus Exception Received";
        case AGH3485_ERR_NOT_INITIALIZED:
            return "Stream Not Initialized";
        case AGH3485_ERR_INVALID_PARAM:
            return "Invalid Parameter";
        default:
            return "Unknown Error";
    }
}

void AGH3485::setAddress(uint8_t address) {
    _slaveAddress = address;
}

uint8_t AGH3485::getAddress() const {
    return _slaveAddress;
}

void AGH3485::setFunctionCode(uint8_t functionCode) {
    _functionCode = functionCode;
}

uint8_t AGH3485::getFunctionCode() const {
    return _functionCode;
}

void AGH3485::setTimeout(uint16_t timeoutMs) {
    _timeoutMs = timeoutMs;
}

uint16_t AGH3485::getTimeout() const {
    return _timeoutMs;
}

bool AGH3485::setDeviceAddress(uint8_t newAddress) {
    if (newAddress < 1 || newAddress > 247) {
        _status = AGH3485_ERR_INVALID_PARAM;
        return false;
    }

    if (writeSingleRegister(AGH3485_REG_SLAVE_ADDR, newAddress)) {
        _slaveAddress = newAddress;
        return true;
    }
    return false;
}

bool AGH3485::setDeviceBaudRate(uint32_t baudRate) {
    uint16_t code = 0;
    switch (baudRate) {
        case 2400:   code = 1; break;
        case 4800:   code = 2; break;
        case 9600:   code = 3; break;
        case 19200:  code = 4; break;
        case 38400:  code = 5; break;
        case 115200: code = 6; break;
        default:
            _status = AGH3485_ERR_INVALID_PARAM;
            return false;
    }

    return writeSingleRegister(AGH3485_REG_BAUD_RATE, code);
}

bool AGH3485::writeSingleRegister(uint16_t regAddress, uint16_t regValue) {
    if (!_serial) {
        _status = AGH3485_ERR_NOT_INITIALIZED;
        return false;
    }

    // Modbus FC 0x06 (Write Single Register)
    uint8_t request[8];
    request[0] = _slaveAddress;
    request[1] = AGH3485_FC_WRITE_SINGLE;
    request[2] = (uint8_t)((regAddress >> 8) & 0xFF);
    request[3] = (uint8_t)(regAddress & 0xFF);
    request[4] = (uint8_t)((regValue >> 8) & 0xFF);
    request[5] = (uint8_t)(regValue & 0xFF);

    uint16_t crc = calculateCRC(request, 6);
    request[6] = (uint8_t)(crc & 0xFF);
    request[7] = (uint8_t)((crc >> 8) & 0xFF);

    if (!sendCommand(request, sizeof(request))) {
        _status = AGH3485_ERR_NOT_INITIALIZED;
        return false;
    }

    // Successful FC 0x06 echoes the 8-byte request frame
    uint8_t response[12];
    size_t received = receiveResponse(response, sizeof(response), 8);

    if (received < 5) {
        _status = AGH3485_ERR_TIMEOUT;
        return false;
    }

    if (response[0] != _slaveAddress) {
        _status = AGH3485_ERR_INVALID_RESPONSE;
        return false;
    }

    if (response[1] & 0x80) {
        _status = AGH3485_ERR_EXCEPTION;
        return false;
    }

    if (received < 8 || response[1] != AGH3485_FC_WRITE_SINGLE) {
        _status = AGH3485_ERR_INVALID_RESPONSE;
        return false;
    }

    uint16_t receivedCRC = (uint16_t)response[6] | ((uint16_t)response[7] << 8);
    uint16_t calculatedCRC = calculateCRC(response, 6);
    if (receivedCRC != calculatedCRC) {
        _status = AGH3485_ERR_CRC;
        return false;
    }

    _status = AGH3485_SUCCESS;
    return true;
}

void AGH3485::setTxMode() {
    if (_derePin >= 0) {
        digitalWrite(_derePin, HIGH);
        delayMicroseconds(50);
    }
}

void AGH3485::setRxMode() {
    if (_derePin >= 0) {
        digitalWrite(_derePin, LOW);
    }
}

void AGH3485::clearRxBuffer() {
    if (!_serial) return;
    while (_serial->available() > 0) {
        _serial->read();
    }
}

bool AGH3485::sendCommand(const uint8_t *frame, size_t length) {
    if (!_serial) return false;

    clearRxBuffer();
    setTxMode();
    _serial->write(frame, length);
    _serial->flush();  // Wait for all bytes to transmit
    delayMicroseconds(100);
    setRxMode();

    return true;
}

size_t AGH3485::receiveResponse(uint8_t *buffer, size_t maxLen, size_t expectedLen) {
    if (!_serial) return 0;

    size_t count = 0;
    uint32_t startMs = millis();

    while (millis() - startMs < _timeoutMs) {
        while (_serial->available() > 0 && count < maxLen) {
            buffer[count++] = (uint8_t)_serial->read();
            startMs = millis();  // Reset timeout on incoming byte

            if (expectedLen > 0 && count >= expectedLen) {
                return count;
            }
        }

        // Inter-frame silence check
        if (count > 0 && (_serial->available() == 0)) {
            delay(10);
            if (_serial->available() == 0) {
                break;
            }
        }
    }

    return count;
}

uint16_t AGH3485::calculateCRC(const uint8_t *buffer, size_t length) {
    uint16_t crc = 0xFFFF;
    for (size_t pos = 0; pos < length; pos++) {
        crc ^= (uint16_t)buffer[pos];
        for (uint8_t i = 8; i != 0; i--) {
            if ((crc & 0x0001) != 0) {
                crc >>= 1;
                crc ^= 0xA001;
            } else {
                crc >>= 1;
            }
        }
    }
    return crc;
}

