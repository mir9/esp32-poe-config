# ESP32-S3-ETH PoE with BME280

This sketch configures an ESP32-S3-ETH PoE board as a WiFi access point to update settings for Ethernet and MQTT. Measurements from a BME280 sensor are sent via MQTT over Ethernet at a configurable interval.

## Wiring

```
ESP32-S3-ETH PoE        BME280
--------------------    ------
3V3 (3.3V)  ----------> VIN
GND          ----------> GND
GPIO10 (SDA) ----------> SDA
GPIO11 (SCL) ----------> SCL
```

Ethernet is built into the PoE board using the RJ45 connector.

## Configuration Website

1. After flashing, the device starts an access point named **ESP32_CONFIG** with password **password**.
2. Connect to this network and browse to `http://192.168.4.1/` to view the configuration page.
3. Update the Ethernet IP, MQTT settings, sampling time, and tag names. When saved the configuration is stored in flash.

Configuration can also be updated by publishing a JSON document to the MQTT topic `esp32/config` while MQTT is enabled.

## Building

Use the Arduino IDE or PlatformIO with the ESP32 board package. The entry point is `src/main.cpp`.
