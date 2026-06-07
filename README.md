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
* **Radio:** 433MHz LoRa Module SX1278 Ra-01 with a 5dBi gain monopole antenna for the Tx and an E220-400T30D with a 8dBi gain Yagi-Uda style antenna for the Rx.
* **Sensors:** *GYML8511 for the UV, BMP180 for temperature and pressure, the rest of the sensors are homebrew.*
# The interface
<img width="1121" height="639" alt="image" src="https://github.com/user-attachments/assets/6fc84583-f084-48f7-948a-84132c1d30f8" />
This is the interface for the Me-TR. It is a simple cmd prompt C++ based program that will handle the changing of parameters and also reading them if .csv support is not possible. In it's current state, it only displays placeholder data but I will soon update it to interface with either the Rx or the Tx via serial.
