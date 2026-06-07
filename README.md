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

## Hardware Stack

* **Microcontroller:** ESP32 (Tx & Rx)
* **Radio:** 433MHz LoRa Module SX1278 Ra-01 with a 5dBi monopole antenna for the Tx and E220-400T30D with a custom 8dBi Yagi-Uda style antenna for the Rx 
* **Sensors:** *GYML8511 for the UV, BMP180 for temperature and pressure, the rest of the sensors are homebrew.*

## Rx Antenna
<img width="904" height="647" alt="image" src="https://github.com/user-attachments/assets/a8f2ae47-9be0-49c3-8182-19ba48af2e9e" />

The Rx antenna is a homebrew Yagi-Uda style antenna. It is made using YagiCAD, and should have ~8dBi gain at 430MHz-440MHz. The graphs for it aswell as the YagiCAD save file are included. 

# Future plans
I plan on making an antenna tracker using a couple of Yagi-Uda antennas and some stepper motors so that I can track the Tx signal from a long distance, even in rough conditions. I also plan on making a windows program that can be used to modify the Rx/Tx settings.
