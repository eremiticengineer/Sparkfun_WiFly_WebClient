#include "WiFlyRn131.h"

// ============================================================
// Constructor
// ============================================================

WiFlyRn131::WiFlyRn131(uint8_t csPin, uint8_t mosiPin, uint8_t misoPin, uint8_t sckPin)
    : csPin_(csPin),
      mosiPin_(mosiPin),
      misoPin_(misoPin),
      sckPin_(sckPin)
{
}

// ============================================================
// begin
// ============================================================

bool WiFlyRn131::begin(unsigned long baudRate)
{
    uartBaudRate_ = baudRate;
    pinMode(csPin_, OUTPUT);
    pinMode(mosiPin_, OUTPUT);
    pinMode(misoPin_, INPUT);
    pinMode(sckPin_, OUTPUT);
    digitalWrite(csPin_, HIGH);
    digitalWrite(mosiPin_, LOW);
    digitalWrite(sckPin_, LOW);
    delay(100);
    return initialiseUart(baudRate);
}

// ============================================================
// Software SPI
//
// SPI mode 0
// MSB first
// ============================================================

uint8_t WiFlyRn131::spiTransfer(uint8_t value)
{
    uint8_t result = 0;
    for (int8_t bit = 7; bit >= 0; --bit) {
        digitalWrite(mosiPin_, (value & (1 << bit)) ? HIGH : LOW);

        // Mode 0:
        // clock idle LOW,
        // sample on rising edge.
        digitalWrite(sckPin_, HIGH);
        result <<= 1;
        if (digitalRead(misoPin_)) {
            result |= 1;
        }

        digitalWrite(sckPin_, LOW);
    }

    return result;
}

// ============================================================
// SC16IS750 register access
// ============================================================

void WiFlyRn131::writeRegister(uint8_t reg, uint8_t value)
{
    const uint8_t address = reg << 3;
    digitalWrite(csPin_, LOW);
    spiTransfer(address);
    spiTransfer(value);
    digitalWrite(csPin_, HIGH);
}

uint8_t WiFlyRn131::readRegister(uint8_t reg)
{
    const uint8_t address = 0x80 | (reg << 3);
    digitalWrite(csPin_, LOW);
    spiTransfer(address);
    const uint8_t value = spiTransfer(0xFF);
    digitalWrite(csPin_, HIGH);
    return value;
}

// ============================================================
// SC16IS750 initialisation
// ============================================================

bool WiFlyRn131::initialiseUart(unsigned long baudRate)
{
    // --------------------------------------------------------
    // UART divisor
    //
    //             XTAL
    // divisor = ---------
    //           baud * 16
    // --------------------------------------------------------

    const unsigned long divisor =
        XTAL_FREQUENCY / (baudRate * 16UL);

    // --------------------------------------------------------
    // Set baud-rate divisor
    // --------------------------------------------------------

    writeRegister(REG_LCR, 0x80);

    writeRegister(
        REG_DLL,
        lowByte(divisor)
    );

    writeRegister(
        REG_DLM,
        highByte(divisor)
    );

    // --------------------------------------------------------
    // Access Enhanced Feature Register
    // --------------------------------------------------------

    writeRegister(REG_LCR, 0xBF);

    // Enable enhanced functions first.
    //
    // Do not enable RTS/CTS yet because the TCR thresholds
    // should be configured before automatic flow control is
    // enabled.
    writeRegister(
        REG_EFR,
        EFR_ENABLE_ENHANCED_FUNCTIONS
    );

    // --------------------------------------------------------
    // Return to normal register set
    // --------------------------------------------------------

    writeRegister(REG_LCR, 0x03);

    // --------------------------------------------------------
    // Enable TCR/TLR register access
    //
    // MCR bit 2 enables access to TCR and TLR when enhanced
    // functions are enabled.
    // --------------------------------------------------------

    uint8_t mcr = readRegister(REG_MCR);

    mcr |= 0x04;

    writeRegister(
        REG_MCR,
        mcr
    );

    // --------------------------------------------------------
    // Configure automatic RTS flow-control thresholds
    //
    // TCR bits:
    //
    //   bits 3:0 = RX FIFO halt threshold / 4
    //   bits 7:4 = RX FIFO resume threshold / 4
    //
    // Halt incoming UART data when RX FIFO reaches 48 bytes:
    //
    //   48 / 4 = 12 = 0x0C
    //
    // Resume incoming UART data when RX FIFO falls to 16 bytes:
    //
    //   16 / 4 = 4 = 0x04
    //
    // Therefore:
    //
    //   TCR = 0x4C
    // --------------------------------------------------------

    writeRegister(
        REG_TCR,
        0x4C
    );

    // --------------------------------------------------------
    // Now enable automatic CTS and RTS flow control
    // --------------------------------------------------------

    writeRegister(REG_LCR, 0xBF);

    writeRegister(
        REG_EFR,
        EFR_ENABLE_ENHANCED_FUNCTIONS |
        EFR_ENABLE_CTS |
        EFR_ENABLE_RTS
    );

    // --------------------------------------------------------
    // 8 data bits, 1 stop bit, no parity
    // --------------------------------------------------------

    writeRegister(
        REG_LCR,
        0x03
    );

    // --------------------------------------------------------
    // Reset TX and RX FIFOs
    //
    // Bit 1 = reset RX FIFO
    // Bit 2 = reset TX FIFO
    // --------------------------------------------------------

    writeRegister(
        REG_FCR,
        0x06
    );

    // --------------------------------------------------------
    // Enable FIFOs
    // --------------------------------------------------------

    writeRegister(
        REG_FCR,
        0x01
    );

    // --------------------------------------------------------
    // Scratchpad test
    //
    // Confirms that register access is working.
    // --------------------------------------------------------

    writeRegister(
        REG_SPR,
        0x55
    );

    if (readRegister(REG_SPR) != 0x55) {
        return false;
    }

    writeRegister(
        REG_SPR,
        0xAA
    );

    if (readRegister(REG_SPR) != 0xAA) {
        return false;
    }

    return true;
}

// ============================================================
// UART interface
// ============================================================

int WiFlyRn131::available()
{
    return readRegister(REG_RXLVL);
}

int WiFlyRn131::read()
{
    if (available() == 0) {
        return -1;
    }

    return readRegister(REG_RHR);
}

void WiFlyRn131::write(uint8_t value)
{
    while (readRegister(REG_TXLVL) == 0) {
    }

    writeRegister(REG_THR, value);
}

void WiFlyRn131::write(const char* text)
{
    while (*text) {
        write(static_cast<uint8_t>(*text));
        ++text;
    }
}

void WiFlyRn131::println(const char* text)
{
    write(text);

    // RN-131 command terminator.
    write('\r');
}

// ============================================================
// Wait for UART to physically finish transmitting
// ============================================================

void WiFlyRn131::waitForTransmitComplete()
{
    while ((readRegister(REG_LSR) & LSR_TEMT) == 0) {
    }
}

// ============================================================
// Flush RN-131 receive FIFO
// ============================================================

void WiFlyRn131::flush()
{
    while (available() > 0) {
        read();
    }
}

// ============================================================
// Wait for a substring from RN-131
// ============================================================

bool WiFlyRn131::waitFor(const char* expected, uint32_t timeoutMs)
{
    const size_t expectedLength = strlen(expected);
    size_t matched = 0;
    const uint32_t start = millis();
    while (millis() - start < timeoutMs) {
        if (available() == 0) {
            continue;
        }

        const int value = read();
        if (value < 0) {
            continue;
        }

        const char c = static_cast<char>(value);
        if (c == expected[matched]) {
            ++matched;
            if (matched == expectedLength) {
                return true;
            }

        } else {
            matched = (c == expected[0])
                ? 1
                : 0;
        }
    }

    return false;
}

// ============================================================
// Read a small RN-131 command response
//
// This is deliberately intended for command responses,
// NOT HTTP bodies.
//
// Keep the reserve small enough for an Uno.
// ============================================================

String WiFlyRn131::readResponse(uint32_t timeoutMs, uint32_t quietTimeMs)
{
    String result;

    /*
     * Command responses such as:
     *
     *   ver
     *   show net
     *   get uart
     *
     * are normally small.
     */
    result.reserve(256);
    const uint32_t start = millis();
    uint32_t lastData = millis();
    bool receivedAnything = false;
    while (millis() - start < timeoutMs) {
        while (available() > 0) {
            const int value = read();
            if (value >= 0) {
                result += static_cast<char>(value);
                lastData = millis();
                receivedAnything = true;
            }
        }

        if (receivedAnything && millis() - lastData >= quietTimeMs) {
            break;
        }
    }

    return result;
}

// ============================================================
// Determine whether RN-131 is already in command mode
// ============================================================

bool WiFlyRn131::isCommandMode()
{
    flush();
    println("ver");
    waitForTransmitComplete();
    return waitFor("wifly-GSX Ver:", 1500);
}

// ============================================================
// Enter RN-131 command mode
// ============================================================

bool WiFlyRn131::enterCommandMode()
{
    /*
     * The module may already be in command mode.
     */
    if (isCommandMode()) {
        return true;
    }

    flush();

    /*
     * Guard interval before $$$.
     */
    delay(500);

    /*
     * Exactly $$$.
     *
     * No CR.
     * No LF.
     */
    write('$');
    write('$');
    write('$');

    /*
     * Wait until the final '$' has physically
     * left the UART.
     */
    waitForTransmitComplete();

    /*
     * Guard interval after $$$.
     */
    delay(500);
    if (waitFor("CMD", 2000)) {
        return true;
    }

    /*
     * CMD may have been missed.
     * Verify command mode using "ver".
     */
    return isCommandMode();
}

// ============================================================
// Public command()
// ============================================================

bool WiFlyRn131::command(const char* cmd, String* response, uint32_t timeoutMs)
{
    if (!enterCommandMode()) {
        return false;
    }

    flush();
    println(cmd);
    waitForTransmitComplete();
    String result = readResponse(timeoutMs);
    if (response != nullptr) {
        *response = result;
    }

    return true;
}

// ============================================================
// join()
// ============================================================

bool WiFlyRn131::join(
    const char* ssid,
    const char* password,
    uint32_t timeoutMs
)
{
    if (!enterCommandMode()) {
        return false;
    }

    char commandBuffer[120];

    // --------------------------------------------------------
    // Enable RN-131 hardware flow control
    // --------------------------------------------------------

    flush();
    println("set uart flow 1");
    waitForTransmitComplete();

    if (!waitFor("AOK", 2000)) {
        return false;
    }

    // --------------------------------------------------------
    // Disable remote *HELLO* greeting
    // --------------------------------------------------------

    flush();
    println("set comm remote 0");
    waitForTransmitComplete();

    if (!waitFor("AOK", 2000)) {
        return false;
    }

    // --------------------------------------------------------
    // WPA2-PSK / AES
    // --------------------------------------------------------

    flush();
    println("set wlan auth 4");
    waitForTransmitComplete();

    if (!waitFor("AOK", 2000)) {
        return false;
    }

    // --------------------------------------------------------
    // Scan all channels
    // --------------------------------------------------------

    flush();
    println("set wlan channel 0");
    waitForTransmitComplete();

    if (!waitFor("AOK", 2000)) {
        return false;
    }

    // --------------------------------------------------------
    // SSID
    // --------------------------------------------------------

    snprintf(
        commandBuffer,
        sizeof(commandBuffer),
        "set wlan ssid %s",
        ssid
    );

    flush();
    println(commandBuffer);
    waitForTransmitComplete();

    if (!waitFor("AOK", 2000)) {
        return false;
    }

    // --------------------------------------------------------
    // WPA/WPA2 passphrase
    // --------------------------------------------------------

    snprintf(
        commandBuffer,
        sizeof(commandBuffer),
        "set wlan phrase %s",
        password
    );

    flush();
    println(commandBuffer);
    waitForTransmitComplete();

    if (!waitFor("AOK", 2000)) {
        return false;
    }

    // --------------------------------------------------------
    // Join network
    // --------------------------------------------------------

    flush();
    println("join");
    waitForTransmitComplete();

    if (!waitFor("Associated!", timeoutMs)) {
        return false;
    }

    // Allow DHCP to complete.
    delay(2000);

    return true;
}

// ============================================================
// httpGet()
//
// IMPORTANT:
//
// The HTTP response is streamed directly to the caller.
//
// Nothing except tiny local variables is used to hold the body.
// This makes it suitable for the Arduino Uno's 2 KB SRAM.
// ============================================================

bool WiFlyRn131::httpGet(
    const char* host,
    uint16_t port,
    const char* path,
    Stream& output,
    uint32_t timeoutMs
)
{
    // --------------------------------------------------------
    // Enter RN-131 command mode
    // --------------------------------------------------------

    if (!enterCommandMode()) {
        return false;
    }

    // --------------------------------------------------------
    // Open TCP connection
    // --------------------------------------------------------

    char openCommand[100];

    snprintf(
        openCommand,
        sizeof(openCommand),
        "open %s %u",
        host,
        port
    );

    flush();
    println(openCommand);
    waitForTransmitComplete();

    /*
     * Successful connection:
     *
     *     *OPEN*
     *
     * After this the RN-131 enters TCP data mode.
     */
    if (!waitFor("*OPEN*", 10000)) {
        return false;
    }

    // --------------------------------------------------------
    // Send HTTP request
    // --------------------------------------------------------

    write("GET ");
    write(path);
    write(" HTTP/1.0\r\n");

    write("Host: ");
    write(host);
    write("\r\n");

    write("Connection: close\r\n");
    write("User-Agent: RN131-Arduino\r\n");
    write("\r\n");

    waitForTransmitComplete();

    // --------------------------------------------------------
    // Stream HTTP response
    //
    // Drain the SC16IS750 RX FIFO efficiently.
    //
    // Avoid calling read() here because read() performs another
    // RXLVL check for every single byte. We have already obtained
    // the number of bytes waiting from available().
    // --------------------------------------------------------

    const uint32_t start = millis();
    uint32_t lastData = millis();

    bool receivedAnything = false;
    bool overrunDetected = false;

    constexpr uint32_t QUIET_TIMEOUT_MS = 10000;

    while (millis() - start < timeoutMs) {

        // ----------------------------------------------------
        // Check UART status before draining the FIFO.
        // ----------------------------------------------------

        const uint8_t lsr = readRegister(REG_LSR);

        if (lsr & LSR_OVERRUN_ERROR) {
            overrunDetected = true;
        }

        // ----------------------------------------------------
        // Find out how many bytes are currently waiting.
        // ----------------------------------------------------

        const int bytesAvailable = available();

        if (bytesAvailable > 0) {

            // ------------------------------------------------
            // Drain exactly that many bytes directly from RHR.
            //
            // This avoids:
            //
            //   available()
            //   read()
            //       -> available() again
            //
            // for every character.
            // ------------------------------------------------

            for (int i = 0; i < bytesAvailable; ++i) {

                const uint8_t value =
                    readRegister(REG_RHR);

                output.write(value);

                receivedAnything = true;
            }

            lastData = millis();
        }

        // ----------------------------------------------------
        // Safety timeout.
        //
        // HTTP/1.0 with Connection: close should normally cause
        // the RN-131 connection to close naturally.
        // ----------------------------------------------------

        if (
            receivedAnything &&
            millis() - lastData >= QUIET_TIMEOUT_MS
        ) {
            break;
        }
    }

    // --------------------------------------------------------
    // Diagnostic
    // --------------------------------------------------------

    if (overrunDetected) {
        output.println();
        output.println(
            F("*** SC16IS750 RX OVERRUN DETECTED ***")
        );
    }

    return receivedAnything;
}

// WiFlyRn131.cpp

bool WiFlyRn131::httpPost(
    const char* host,
    uint16_t port,
    const char* path,
    const char* contentType,
    const char* apiKey,
    const char* body,
    Stream& output,
    uint32_t timeoutMs
)
{
    if (!enterCommandMode()) {
        return false;
    }

    char openCommand[100];

    snprintf(
        openCommand,
        sizeof(openCommand),
        "open %s %u",
        host,
        port
    );

    flush();
    println(openCommand);
    waitForTransmitComplete();

    if (!waitFor("*OPEN*", 10000)) {
        return false;
    }

    const size_t bodyLength = strlen(body);

    char contentLength[16];

    snprintf(
        contentLength,
        sizeof(contentLength),
        "%u",
        static_cast<unsigned int>(bodyLength)
    );

    write("POST ");
    write(path);
    write(" HTTP/1.0\r\n");

    write("Host: ");
    write(host);
    write("\r\n");

    write("Content-Type: ");
    write(contentType);
    write("\r\n");

    write("Content-Length: ");
    write(contentLength);
    write("\r\n");

    if (apiKey != nullptr && apiKey[0] != '\0') {
        write("X-API-KEY: ");
        write(apiKey);
        write("\r\n");
    }

    write("Connection: close\r\n");
    write("User-Agent: RN131-Arduino\r\n");
    write("\r\n");

    write(body);

    waitForTransmitComplete();

    const uint32_t start = millis();
    uint32_t lastData = millis();

    bool receivedAnything = false;

    constexpr uint32_t QUIET_TIMEOUT_MS = 10000;

    while (millis() - start < timeoutMs) {
        bool receivedThisPass = false;

        while (available() > 0) {
            const int value = read();

            if (value < 0) {
                continue;
            }

            output.write(static_cast<uint8_t>(value));

            receivedAnything = true;
            receivedThisPass = true;
        }

        if (receivedThisPass) {
            lastData = millis();
        }

        if (
            receivedAnything &&
            millis() - lastData >= QUIET_TIMEOUT_MS
        ) {
            break;
        }
    }

    return receivedAnything;
}