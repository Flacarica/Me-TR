# Me-TR

A robust, remote meteorological station powered by the **ESP32** microcontroller and **LoRa** technology. Me-TR is designed for reliable, long-range weather data collection, transmission, and easy on-site configuration.

## System Architecture
The Me-TR system consists of two dedicated assemblies:

* **Transmitter (Tx):** The remote sensor node. It continuously gathers environmental telemetry and broadcasts the data packets over LoRa.
* **Receiver (Rx):** The base station. It is responsible for listening, receiving, and decoding the data packets sent by the Tx node for further processing and display.

## Key Features

### Environmental Telemetry
The Tx module interfaces with various sensors to measure a comprehensive suite of meteorological data:
* **Wind:** Speed and Direction
* **Precipitation:** Rain Volume
* **Climate:** Temperature & Relative Humidity
* **Solar:** UV Index

### BLE Configuration Shell
The Tx node features a built-in **Bluetooth Low Energy (BLE) shell** to which you can connect to via a smartphone BLE terminal to dynamically adjust critical LoRa parameters on the fly, including:
* Transmission Power
* Spreading Factor (SF)
* Coding Rate (CR)

## Hardware Stack

* **Microcontroller:** ESP32 (Tx & Rx)
* **Radio:** 433MHz LoRa Modules (e.g., SX1278 / Ra-02)
* **Sensors:** *GYML8511 for the UV, BMP180 for temperature and pressure, the rest of the sensors are homebrew.*
