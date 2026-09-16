#pragma once

constexpr char WIFI_SSID[] = "ESP8266-HTTPS-Proxy";
constexpr char WIFI_PASSWORD[] = "proxy1234";

constexpr char SERVER[] = "192.168.4.1";
constexpr int PORT = 80;
constexpr char PATH[] = "/proxy";
constexpr char API_KEY[] = "";

static const char PROXY_GET_REMOTE_SERVER[] = "";
static constexpr int PROXY_GET_REMOTE_SERVER_PORT = 443;
static const char PROXY_GET_REMOTE_SERVER_PATH[] = "/";

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
