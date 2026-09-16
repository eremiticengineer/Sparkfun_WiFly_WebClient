#pragma once

// The ESP-01 wifi network to connect to
constexpr char WIFI_SSID[] = "ESP8266-HTTPS-Proxy";
constexpr char WIFI_PASSWORD[] = "proxy1234";

// The proxy server running on the ESP-01
constexpr char PROXY_SERVER[] = "192.168.4.1";
constexpr int PROXY_SERVER_PORT = 80;
constexpr char PROXY_SERVER_PATH[] = "/proxy";

// The https://server:port/path the proxy server should retrieve
static const char REMOTE_SERVER[] = "eremiticengineer.com";
static constexpr int REMOTE_SERVER_PORT = 443;
static const char REMOTE_SERVER_PATH[] = "/sitemap.xml";

/*
 * The payload to send to the proxy server.
 * The proxy will extract server:port/path, set the apiKey and POST the payload, e.g.
 * POST https://_theremoteserver_.com:443/path/on/the/remote/server apiKey payload
 */
static const char PAYLOAD_FOR_PROXY_POST[] = R"EOF(
{
"server": "_theremoteserver_.com",
"port": 443,
"path": "/path/on/the/remote/server",
"apiKey": "",
"payload": {
    "packetType": 1,
    "temperature": 22.8,
    "humidity": 90.1,
    "pressure": 998.7
}
}
)EOF";
