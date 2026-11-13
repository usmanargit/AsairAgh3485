/**
 * @file AGH3485.h
 * @brief Arduino Library for ASAIR AGH3485 Modbus RS485 Temperature & Humidity Sensor
 * @author Usman AR (usmanar.tsm@gmail.com)
 * @license MIT
 */

#ifndef AGH3485_H
#define AGH3485_H

#include <Arduino.h>
#include <Stream.h>

// Default Modbus parameters for ASAIR AGH3485
#define AGH3485_DEFAULT_ADDRESS     0x01
#define AGH3485_DEFAULT_TIMEOUT     1000  // milliseconds
#define AGH3485_DEFAULT_BAUD        9600

// Modbus Function Codes
#define AGH3485_FC_READ_HOLDING     0x03
#define AGH3485_FC_READ_INPUT       0x04
#define AGH3485_FC_WRITE_SINGLE     0x06

// Internal Register Addresses
#define AGH3485_REG_HUMIDITY        0x0000
#define AGH3485_REG_TEMPERATURE     0x0001
#define AGH3485_REG_SLAVE_ADDR      0x0066
#define AGH3485_REG_BAUD_RATE       0x0067

/**
 * @brief Library Status and Error Codes
 */
enum AGH3485_Status {
    AGH3485_SUCCESS = 0,
    AGH3485_ERR_TIMEOUT,
    AGH3485_ERR_CRC,
    AGH3485_ERR_INVALID_RESPONSE,
    AGH3485_ERR_EXCEPTION,
    AGH3485_ERR_NOT_INITIALIZED,
    AGH3485_ERR_INVALID_PARAM
};

class AGH3485 {
public:
    /**
     * @brief Construct a new AGH3485 instance.
     * @param slaveAddress Modbus slave ID (default: 0x01).
     */
    explicit AGH3485(uint8_t slaveAddress = AGH3485_DEFAULT_ADDRESS);

    /**
     * @brief Initialize the sensor with a communication Stream (e.g. Serial1, SoftwareSerial).
     * @param stream Reference to Stream object (HardwareSerial or SoftwareSerial).
     * @param derePin Optional RS485 DE/RE direction pin. Set to -1 if using auto-switching transceiver.
     * @return true Initialization successful.
     */
    bool begin(Stream &stream, int8_t derePin = -1);

    /**
     * @brief Query the sensor for the latest temperature and humidity measurements.
     * @return true Measurement read successfully.
     * @return false Communication failed (check getStatus() or getStatusText()).
     */
    bool read();

    /**
     * @brief Helper to query and directly assign temperature and humidity values.
     * @param[out] temperature Target variable for temperature in °C.
     * @param[out] humidity Target variable for relative humidity in %RH.
     * @return true Measurement read successfully.
     */
    bool read(float &temperature, float &humidity);

    /**
     * @brief Get temperature in Celsius (°C) from last successful read.
     * @return float Temperature in °C.
     */
    float getTemperature() const;

    /**
     * @brief Get relative humidity in percent (%RH) from last successful read.
     * @return float Relative humidity in %RH.
     */
    float getHumidity() const;

    /**
     * @brief Get raw 16-bit signed temperature integer (value * 10).
     */
    int16_t getRawTemperature() const;

    /**
     * @brief Get raw 16-bit unsigned humidity integer (value * 10).
     */
    uint16_t getRawHumidity() const;

    /**
     * @brief Get the status code of the last operation.
     */
    AGH3485_Status getStatus() const;

    /**
     * @brief Get human-readable description of the current status code.
     */
    const char* getStatusText() const;

    /**
     * @brief Set local target slave address for Modbus requests.
     * @param address Modbus slave ID (1 to 247).
     */
    void setAddress(uint8_t address);

    /**
     * @brief Get current target slave address.
     */
    uint8_t getAddress() const;

    /**
     * @brief Set Modbus function code used to read registers (0x03 or 0x04).
     * @param functionCode 0x03 (Holding Registers) or 0x04 (Input Registers).
     */
    void setFunctionCode(uint8_t functionCode);

    /**
     * @brief Get current Modbus function code used for reading.
     */
    uint8_t getFunctionCode() const;

    /**
     * @brief Set response timeout in milliseconds.
     * @param timeoutMs Timeout duration in ms (default 1000ms).
     */
    void setTimeout(uint16_t timeoutMs);

    /**
     * @brief Get response timeout in milliseconds.
     */
    uint16_t getTimeout() const;

    /**
     * @brief Change the sensor's internal Modbus slave address via holding register 0x0066.
     * @param newAddress New slave address (1 to 247).
     * @return true Address changed successfully.
     */
    bool setDeviceAddress(uint8_t newAddress);

    /**
     * @brief Change the sensor's internal baud rate via holding register 0x0067.
     * Supported standard baud rates: 2400, 4800, 9600, 19200, 38400, 115200.
     * @param baudRate Target baud rate.
     * @return true Baud rate changed successfully.
     */
    bool setDeviceBaudRate(uint32_t baudRate);

    /**
     * @brief Calculate Modbus RTU CRC-16 checksum.
     * @param buffer Byte array.
     * @param length Number of bytes to include in calculation.
     * @return uint16_t CRC-16 checksum (Low byte first in Modbus frames).
     */
    static uint16_t calculateCRC(const uint8_t *buffer, size_t length);

private:
    Stream *_serial;
    int8_t _derePin;
    uint8_t _slaveAddress;
    uint8_t _functionCode;
    uint16_t _timeoutMs;

    float _temperature;
    float _humidity;
    int16_t _rawTemperature;
    uint16_t _rawHumidity;
    AGH3485_Status _status;

    void setTxMode();
    void setRxMode();
    void clearRxBuffer();
    bool sendCommand(const uint8_t *frame, size_t length);
    size_t receiveResponse(uint8_t *buffer, size_t maxLen, size_t expectedLen);
    bool writeSingleRegister(uint16_t regAddress, uint16_t regValue);
};

#endif // AGH3485_H

