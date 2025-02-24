# Battery Monitor with INA226 and SenseESP

## Overview
This project is a DIY battery monitor for our trawler. It uses an ESP32 to measure current, voltage, and power with the INA226 sensor. The collected data is integrated into Signal K via SenseESP. Additionally, a OneWire temperature sensor is used to monitor battery temperature.

## What is Signal K?
[Signal K](https://signalk.org/) is an open data format for marine electronics. It enables the integration and exchange of sensor data between various devices and applications on board a boat.

## What is SenseESP?
[SenseESP](https://github.com/SignalK/SensESP) is an open-source framework that simplifies the development of sensors that transmit their data to a Signal K server via Wi-Fi. It is based on the ESP32 and uses a modular architecture for flexible customization.

## Features
- Measurement of battery voltage, current, and power with INA226
- Calculation of the state of charge (SoC) based on battery type and voltage
- Calculation of remaining amp-hours (Ah) and estimated runtime
- Configurable battery parameters via the SenseESP web UI
- Alarm notifications for shunt overvoltage, bus overvoltage, and power limits
- Temperature monitoring via a OneWire sensor (e.g., DS18B20)

## Hardware Requirements
- **ESP32 D1 Mini** – Microcontroller
- **INA226 CJMCU** – Current sensor
- **DS18B20** – Temperature sensor
- **3A Mini DC-DC Buck Step Down Converter** – Voltage regulator
- **Shunt resistor** – For current measurement (adapted to the maximum current)

## Terminal Assignments
| Terminal | Label             | Description            |
|----------|------------------|------------------------|
| **U1**   | Power IN         | + / - (6 - 32V)       |
| **U2**   | Temperature Sensor | + / - / Data          |
| **U4**   | INA226           | VBUS / IN - / IN +    |

## Signal K Data Points
- `electrical.batteries.<battery_name>.voltage` – Battery voltage in V
- `electrical.batteries.<battery_name>.current` – Battery current in A
- `electrical.batteries.<battery_name>.power` – Battery power in W
- `electrical.batteries.<battery_name>.stateOfCharge` – State of charge in %
- `electrical.batteries.<battery_name>.ampHours` – Remaining Ah
- `electrical.batteries.<battery_name>.timeToGo` – Remaining time in seconds
- `electrical.batteries.<battery_name>.temperature` – Battery temperature in °K
- `electrical.batteries.<battery_name>.alerts.shuntOverVoltage` – Alarm for shunt overvoltage
- `electrical.batteries.<battery_name>.alerts.busOverVoltage` – Alarm for bus overvoltage
- `electrical.batteries.<battery_name>.alerts.powerOverLimit` – Alarm for power limit exceeded

## Installation
### 1. Preparation
- Install Arduino IDE or PlatformIO
- Install required libraries:
  - [SignalK/SenseESP](https://github.com/SignalK/SensESP)
  - [sensesp/OneWire](https://github.com/PaulStoffregen/OneWire)
  - [INA226](https://github.com/RobTillaart/INA226)

### 2. Uploading the Code
- Use the Arduino IDE or PlatformIO to upload the code to the ESP32.

### 3. Configuring Signal K Server
- After the first boot, the device will create an access point.
- Connect to the network and enter the Wi-Fi credentials.
- The data should now appear on the Signal K server under the configured paths.

## Configuration
The following parameters can be adjusted via the SenseESP web UI:
- Battery type (LiFePO4, AGM, Gel, Lead-Acid)
- Battery voltage (12V or 24V)
- Battery capacity in Ah
- Maximum shunt current
- Shunt voltage in mV
- Battery name for Signal K

**After each configuration change, the ESP32 must be restarted.**

## Usage
- Once set up, the ESP32 continuously transmits measurement values to Signal K.
- Configuration changes can be made via the web UI.
- Alarm notifications are triggered when defined thresholds are exceeded.

## Used Components
- [ESP32 D1 Mini](https://de.aliexpress.com/item/1005006267267848.html)
- [INA226 CJMCU](https://de.aliexpress.com/item/1005001972537281.html)
- [3A Mini DC-DC Buck Step Down Converter](https://de.aliexpress.com/item/1005005505907937.html)
- [DS18B20 Temperature Sensor](https://de.aliexpress.com/item/1005005488542048.html)

## Repository Files
- **`/gerber`** – Contains Gerber files for the PCB.
- **`/schematic`** – Contains the circuit diagram.

## License
- **Software:** [GNU General Public License v3.0](LICENSE)  
- **Hardware:** [CERN Open Hardware Licence v2 - Strongly Reciprocal](LICENSE-HARDWARE)

## Acknowledgments
This project is based on the [Battery Monitor Project by Techstyleuk](https://github.com/Techstyleuk/SensESP_3_Battery_Monitor).

