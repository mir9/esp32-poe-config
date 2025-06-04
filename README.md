# ESP32-S3-ETH PoE with BME280

This sketch configures an ESP32-S3-ETH PoE board as a WiFi access point to update settings for Ethernet and MQTT. Measurements from a BME280 sensor are sent via MQTT over Ethernet at a configurable interval.

## Wiring


| ESP32-S3-ETH Pin | BME280 Pin |
|------------------|------------|
| 3V3 (Pin 36)     | VIN        |
| GND (Pin 38)     | GND        |
| GPIO17 (Pin 34)  | SCL        |
| GPIO18 (Pin 31)  | SDA        |

Ethernet uses a W5500 module on SPI:

```
ETH_CS   -> GPIO14
ETH_RST  -> GPIO9
SPI_MOSI -> GPIO11
SPI_MISO -> GPIO12
SPI_SCK  -> GPIO13
```


## Configuration Website

1. After flashing, the device starts an access point named **ESP32_CONFIG** with password **password**.
2. Connect to this network and browse to `http://192.168.4.1/` to view the configuration page.
3. Update the Ethernet IP, MQTT settings, sampling time, and tag names. When saved the configuration is stored in flash.

Configuration can also be updated by publishing a JSON document to the MQTT topic `esp32/config` while MQTT is enabled.

## Building


This repository includes a `platformio.ini` configuration. Build and upload with PlatformIO:

```bash
pio run -t upload
```

The entry point is `src/main.cpp`.
