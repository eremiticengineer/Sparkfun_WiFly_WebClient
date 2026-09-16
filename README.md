# SparkFun WiFly Web Client

A modernised Arduino example for the original SparkFun WiFly Shield using the RN-131 Wi-Fi module and SC16IS750 SPI-to-UART bridge.

This project does **not** use the original SparkFun WiFly library. Instead, it communicates directly with the SC16IS750 over SPI and controls the RN-131 through its command interface.

The goal is to keep the original SparkFun WiFly Shield useful on modern Arduino toolchains despite the original software library no longer working reliably with current Arduino environments.

## Features

* Direct SC16IS750 SPI communication
* Direct RN-131 command interface
* Wi-Fi association using WPA/WPA2 credentials
* Hardware RTS/CTS flow control
* Plain HTTP GET requests
* HTTP responses streamed directly rather than buffered in RAM
* Suitable for the Arduino Uno's limited 2 KB SRAM
* No dependency on the original SparkFun WiFly library
* Builds with `arduino-cli`

## Hardware

This project currently targets:

* Arduino Uno
* SparkFun WiFly Shield
* RN-131 Wi-Fi module
* SC16IS750 SPI-to-UART bridge

The SparkFun WiFly Shield uses the standard Arduino shield SPI pin arrangement:

```text
D10  CS
D11  MOSI
D12  MISO
D13  SCK
```

The `WiFlyRn131` class uses software SPI on these pins.

## Project Structure

A typical project layout is:

```text
Sparkfun_WiFly_WebClient/
├── Sparkfun_WiFly_WebClient.ino
├── WiFlyRn131.cpp
├── WiFlyRn131.h
├── secrets.example.hpp
├── secrets.hpp
├── .gitignore
└── README.md
```

`secrets.hpp` should not be committed to source control.

## Wi-Fi Secrets

Copy the supplied example file:

```bash
cp secrets.example.hpp secrets.hpp
```

Then edit `secrets.hpp` and add your Wi-Fi credentials.

For example:

```cpp
#pragma once

constexpr char WIFI_SSID[] = "your-ssid";
constexpr char WIFI_PASSWORD[] = "your-password";
```

The project should contain a `.gitignore` entry for:

```text
secrets.hpp
```

A blank `secrets.example.hpp` can safely be committed:

```cpp
#pragma once

constexpr char WIFI_SSID[] = "";
constexpr char WIFI_PASSWORD[] = "";
```

## Building

The project uses the Arduino CLI.

Compile for the Arduino Uno with:

```bash
arduino-cli compile --fqbn arduino:avr:uno Sparkfun_WiFly_WebClient
```

## Uploading

Connect the Arduino Uno and upload with:

```bash
arduino-cli upload --port /dev/ttyACM0 --fqbn arduino:avr:uno Sparkfun_WiFly_WebClient
```

Change `/dev/ttyACM0` if your Arduino appears on a different serial device.

You can list detected Arduino boards with:

```bash
arduino-cli board list
```

## Typical Build and Upload

From the directory containing the `Sparkfun_WiFly_WebClient` project:

```bash
cp secrets.example.hpp secrets.hpp

arduino-cli compile \
    --fqbn arduino:avr:uno \
    Sparkfun_WiFly_WebClient

arduino-cli upload \
    --port /dev/ttyACM0 \
    --fqbn arduino:avr:uno \
    Sparkfun_WiFly_WebClient
```

The `cp` command only needs to be performed when initially creating your local `secrets.hpp`.

## Serial Monitor

The example sketch uses:

```text
115200 baud
```

You can monitor the Arduino using:

```bash
arduino-cli monitor \
    --port /dev/ttyACM0 \
    --config baudrate=115200
```

Typical output includes Wi-Fi initialisation, association information, network configuration and the HTTP response.

## Example Usage

The WiFly interface is created with:

```cpp
WiFlyRn131 wifi(
    WIFLY_CS,
    WIFLY_MOSI,
    WIFLY_MISO,
    WIFLY_SCK
);
```

Initialise the shield:

```cpp
if (!wifi.begin()) {
    Serial.println(F("WiFly initialisation failed."));
}
```

Join the Wi-Fi network:

```cpp
if (!wifi.join(WIFI_SSID, WIFI_PASSWORD)) {
    Serial.println(F("WiFi join failed."));
}
```

RN-131 commands can be sent directly:

```cpp
String networkInfo;

wifi.command(
    "show net",
    &networkInfo,
    1500
);

Serial.println(networkInfo);
```

A plain HTTP GET can be streamed directly to `Serial`:

```cpp
wifi.httpGet(
    "google.com",
    80,
    "/",
    Serial
);
```

Streaming the HTTP response avoids storing the complete response in memory, which is particularly important on the Arduino Uno.

## RN-131 Configuration

The library configures the RN-131 for hardware flow control using:

```text
set uart flow 1
```

The SC16IS750 side is also configured for RTS/CTS hardware flow control.

The default RN-131 remote greeting is disabled using:

```text
set comm remote 0
```

This is required for HTTP because the default RN-131 behaviour prepends:

```text
*HELLO*
```

to data sent when a TCP connection opens.

Without disabling the greeting, an HTTP request such as:

```text
GET / HTTP/1.0
```

would arrive at the web server as:

```text
*HELLO*GET / HTTP/1.0
```

and would therefore be rejected.

The RN-131 may still emit local status messages such as:

```text
*OPEN*
*CLOS*
```

when TCP connections open and close.

## HTTP Support

The RN-131 works well for plain TCP and HTTP connections.

For example:

```text
http://example.com
```

is supported.

Modern HTTPS is **not supported directly** by this implementation. The RN-131 does not provide a modern TLS stack suitable for current HTTPS servers.

For HTTPS applications, a practical architecture is to send plain HTTP to a trusted local gateway and allow the gateway to perform HTTPS communication with external services.

For example:

```text
Arduino Uno
    |
    | HTTP
    v
RN-131
    |
    | Wi-Fi
    v
Local gateway
    |
    | HTTPS
    v
Internet service
```

A Raspberry Pi, ESP32, Pico W or similar device can be used as the gateway.

## Memory Considerations

The Arduino Uno has only 2 KB of SRAM.

For this reason, HTTP responses are streamed directly to a supplied Arduino `Stream` rather than being accumulated in an Arduino `String`.

For example:

```cpp
wifi.httpGet(
    "example.com",
    80,
    "/",
    Serial
);
```

This allows responses much larger than the available SRAM to be processed without holding the complete response in memory.

Small RN-131 command responses such as `show net` may still use Arduino `String` objects.

## Background

The original SparkFun WiFly Shield software was written for much older Arduino environments and eventually became difficult to use with modern versions of the Arduino toolchain.

The hardware itself remains perfectly usable.

This project replaces the old software stack with a small direct driver:

```text
Arduino
   |
   | SPI
   v
SC16IS750
   |
   | UART
   v
RN-131
   |
   | Wi-Fi
   v
Network
```

This allows existing WiFly Shields to remain useful rather than becoming obsolete purely because their original software library is no longer maintained.

## Limitations

* Plain HTTP/TCP only
* No modern HTTPS/TLS support
* Designed around the SparkFun WiFly Shield hardware
* Currently tested with the Arduino Uno
* Software SPI is used on D10-D13
* RN-131 command responses should remain relatively small on AVR boards

## License

This project is licensed under the MIT License.

See the [LICENSE](LICENSE) file for details.

## Acknowledgements

The original SparkFun WiFly library stopped working many years ago following changes to the Arduino development environment, and the hardware was subsequently sidelined due to time constraints.

After rediscovering the hardware, the project was revived through a collaborative development process with ChatGPT. The replacement codebase was written by ChatGPT from the project requirements, with the design guided and refined through hardware testing, debugging, and feedback on the class structure and behaviour.

In practical terms, ChatGPT acted much like a highly efficient software contractor brought in to revive otherwise obsolete hardware. The result is a modern, lightweight replacement for the original library that allows the WiFly Shield to be useful again on a local network.

Because the RN-131 predates modern TLS requirements, the intended architecture is to use the shield for HTTP communication within a trusted LAN, with a gateway providing HTTPS connectivity to external services.

This project is therefore both a software restoration exercise and an example of extending the useful life of older hardware rather than discarding it simply because its original software support has become obsolete.
