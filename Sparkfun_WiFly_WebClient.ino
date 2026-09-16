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

    if (!wifi.httpGet(SERVER, PORT, PATH, Serial)) {
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
    #ifdef HTTP_POST_ENDPOINT_AVAILABLE
    const char json[] = "{\"temperature\":21.5,\"humidity\":67}";
    if (!wifi.httpPost(SERVER, PORT, PATH, "application/json", API_KEY, json, Serial)) {
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
