#pragma once

#include <Arduino.h>

class WiFlyRn131 {
public:
    WiFlyRn131(
        uint8_t csPin,
        uint8_t mosiPin,
        uint8_t misoPin,
        uint8_t sckPin
    );

    // Initialise software SPI and the SC16IS750 UART bridge.
    bool begin(
        unsigned long baudRate = 9600
    );

    // Join a Wi-Fi network.
    bool join(const char* ssid, const char* password, uint32_t timeoutMs = 20000);

    // Send an RN-131 command.
    //
    // response may be nullptr when the response is not required.
    bool command(const char* cmd, String* response = nullptr, uint32_t timeoutMs = 1000);

    // Perform a plain HTTP GET.
    //
    // The response is streamed directly to output rather than
    // being stored in RAM.
    //
    // Example:
    //
    //     wifi.httpGet(
    //         "example.com",
    //         80,
    //         "/",
    //         Serial
    //     );
    //
    bool httpGet(const char* host, uint16_t port, const char* path, Stream& output, uint32_t timeoutMs = 30000);

    bool httpPost(const char* host, uint16_t port, const char* path, const char* contentType, const char* apiKey,
        const char* body, Stream& output, uint32_t timeoutMs = 30000);

    // Low-level UART access.
    int available();

    int read();

    void write(uint8_t value);

    void write(const char* text);

private:
    uint8_t csPin_;
    uint8_t mosiPin_;
    uint8_t misoPin_;
    uint8_t sckPin_;

    unsigned long uartBaudRate_ = 9600;

    // ========================================================
    // SC16IS750 registers
    // ========================================================
    static constexpr uint8_t REG_RHR   = 0x00;
    static constexpr uint8_t REG_THR   = 0x00;

    static constexpr uint8_t REG_IER   = 0x01;

    static constexpr uint8_t REG_FCR   = 0x02;
    static constexpr uint8_t REG_IIR   = 0x02;

    static constexpr uint8_t REG_LCR   = 0x03;
    static constexpr uint8_t REG_MCR   = 0x04;
    static constexpr uint8_t REG_LSR   = 0x05;
    static constexpr uint8_t REG_MSR   = 0x06;
    static constexpr uint8_t REG_SPR   = 0x07;

    static constexpr uint8_t REG_TXLVL = 0x08;
    static constexpr uint8_t REG_RXLVL = 0x09;

    // Divisor latch registers.
    static constexpr uint8_t REG_DLL   = 0x00;
    static constexpr uint8_t REG_DLM   = 0x01;

    // Enhanced Feature Register.
    static constexpr uint8_t REG_EFR   = 0x02;

    static constexpr uint8_t REG_TCR = 0x06;
    static constexpr uint8_t REG_TLR = 0x07;

    // ========================================================
    // SC16IS750 bits
    // ========================================================
    // Transmitter completely empty:
    // holding register + shift register.
    static constexpr uint8_t LSR_TEMT = 1 << 6;

    static constexpr uint8_t EFR_ENABLE_CTS = 1 << 7;

    static constexpr uint8_t EFR_ENABLE_RTS = 1 << 6;

    static constexpr uint8_t EFR_ENABLE_ENHANCED_FUNCTIONS = 1 << 4;

    // Later SparkFun WiFly Shield crystal.
    static constexpr unsigned long XTAL_FREQUENCY = 14745600UL;

    static constexpr uint8_t LSR_OVERRUN_ERROR = 1 << 1;

    // ========================================================
    // Software SPI
    // ========================================================
    uint8_t spiTransfer(uint8_t value);
    void writeRegister(uint8_t reg, uint8_t value);
    uint8_t readRegister(uint8_t reg);

    // ========================================================
    // SC16IS750
    // ========================================================
    bool initialiseUart(unsigned long baudRate);
    void waitForTransmitComplete();

    // ========================================================
    // RN-131 helpers
    // ========================================================
    void println(const char* text);
    void flush();
    bool isCommandMode();
    bool enterCommandMode();
    bool waitFor(const char* expected, uint32_t timeoutMs);
    String readResponse(uint32_t timeoutMs, uint32_t quietTimeMs = 200);
};