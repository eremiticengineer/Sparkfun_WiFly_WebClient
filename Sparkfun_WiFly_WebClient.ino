#include "WiFlyRn131.h"
#include "secrets.hpp"

// ------------------------------------------------------------
// SparkFun WiFly Shield pins
// ------------------------------------------------------------
constexpr uint8_t WIFLY_CS   = 10;
constexpr uint8_t WIFLY_MOSI = 11;
constexpr uint8_t WIFLY_MISO = 12;
constexpr uint8_t WIFLY_SCK  = 13;

// ------------------------------------------------------------
// WiFly instance
// ------------------------------------------------------------
WiFlyRn131 wifi(
    WIFLY_CS,
    WIFLY_MOSI,
    WIFLY_MISO,
    WIFLY_SCK
);

// ------------------------------------------------------------
// setup
// ------------------------------------------------------------
void setup()
{
    Serial.begin(115200);

    delay(1000);

    Serial.println();
    Serial.println(F("RN-131 WiFly test"));

    // --------------------------------------------------------
    // Initialise shield
    // --------------------------------------------------------
    Serial.println(F("Initialising WiFly..."));

    if (!wifi.begin()) {
        Serial.println(F("WiFly initialisation failed."));
        while (true) {}
    }

    Serial.println(F("WiFly initialised."));

    // --------------------------------------------------------
    // Join WiFi
    // --------------------------------------------------------
    Serial.println(F("Joining WiFi..."));

    if (!wifi.join(WIFI_SSID, WIFI_PASSWORD)) {
        Serial.println(F("WiFi join failed."));
        while (true) {}
    }

    Serial.println(F("WiFi connected."));

    // --------------------------------------------------------
    // Query network information
    // --------------------------------------------------------
    String networkInfo;

    if (wifi.command("show net", &networkInfo, 1500)) {
        Serial.println();
        Serial.println(F("Network information:"));
        Serial.println(networkInfo);
    }
    else {
        Serial.println();
        Serial.println(F("Failed to read network information."));
    }

    // --------------------------------------------------------
    // HTTP GET
    //
    // The response is streamed directly to Serial.
    //
    // This is important on the Arduino Uno because it only has
    // 2 KB of SRAM. We do not store the HTTP response in a
    // String.
    // --------------------------------------------------------
    Serial.println();
    Serial.println(F("Performing HTTP GET..."));
    Serial.println();
    Serial.println(F("HTTP response:"));
    Serial.println(F("----------------------------------------"));

    // Create the remote server/path param list
    char proxyPath[180];
    snprintf(proxyPath, sizeof(proxyPath),
        "%s?server=%s&port=%d&path=%s",
        PROXY_SERVER_PATH, REMOTE_SERVER, REMOTE_SERVER_PORT, REMOTE_SERVER_PATH
    );

    if (!wifi.httpGet(PROXY_SERVER, PROXY_SERVER_PORT, proxyPath, Serial)) {
        Serial.println();
        Serial.println(F("----------------------------------------"));
        Serial.println(F("HTTP GET failed."));
    }
    else {
        Serial.println();
        Serial.println(F("----------------------------------------"));
        Serial.println(F("HTTP GET complete."));
    }

    // --------------------------------------------------------
    // HTTP POST
    //
    // The response is streamed directly to Serial.
    //
    // This is important on the Arduino Uno because it only has
    // 2 KB of SRAM. We do not store the HTTP response in a
    // String.
    // --------------------------------------------------------
    #define HTTP_POST_ENDPOINT_AVAILABLE
    #ifdef HTTP_POST_ENDPOINT_AVAILABLE
    if (!wifi.httpPost(PROXY_SERVER, PROXY_SERVER_PORT, PROXY_SERVER_PATH, "application/json", nullptr, PAYLOAD_FOR_PROXY_POST, Serial)) {
        Serial.println(F("HTTP POST failed."));
    }
    #endif
}

// ------------------------------------------------------------
// loop
// ------------------------------------------------------------
void loop()
{
}
