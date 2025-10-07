# Battery Monitor with INA226 and SenseESP

## Overview
This project is a DIY battery monitor for our trawler. It uses an ESP32 to measure current, voltage, and power with the INA226 sensor. The collected data is integrated into Signal K via SenseESP. Additionally, a OneWire temperature sensor is used to monitor battery temperature.

## Why Low-Side Current Sensing?

Low-side current sensing means placing the shunt resistor between the negative battery terminal and ground (system GND). This method is recommended for higher battery voltages (e.g. 48V), as it keeps the INA226 within its voltage specifications and avoids damage or incorrect measurements.

**Advantages:**
- The INA226 always operates within its safe voltage range (max. 36V common-mode).
- Simple and safe wiring for most installations (vehicles, boats).
- No need for a voltage divider at the INA226 input.

**Note:**  
The measured current may have an inverted sign. This is corrected in the software.

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
- **3A Mini DC-DC Buck Step Down Converter** – Voltage converter
- **Shunt resistor** – For current measurement (adapted to the maximum current)

## Terminal Assignments
| Terminal | Label             | Description            |
|----------|------------------|------------------------|
| **U1**   | Power IN         | + / - (6 - 48V)       |
| **U2**   | Temperature Sensor | + / - / Data          |
| **U4**   | INA226           | VBUS / IN- / IN+      |

## Signal K Data Points
- `electrical.batteries.<battery_name>.voltage` – Battery voltage in V
- `electrical.batteries.<battery_name>.current` – Battery current in A
- `electrical.batteries.<battery_name>.power` – Battery power in W
- `electrical.batteries.<battery_name>.stateOfCharge` – State of charge in % (coulomb counting)
- `electrical.batteries.<battery_name>.stateOfCharge_voltageSoC` – State of charge based on voltage in %
- `electrical.batteries.<battery_name>.ampHours` – Remaining Ah
- `electrical.batteries.<battery_name>.timeToGo` – Remaining time in seconds
- `electrical.batteries.<battery_name>.temperature` – Battery temperature in °K
- `electrical.batteries.<battery_name>.alerts.shuntOverVoltage` – Alarm for shunt overvoltage
- `electrical.batteries.<battery_name>.alerts.busOverVoltage` – Alarm for bus overvoltage
- `electrical.batteries.<battery_name>.alerts.powerOverLimit` – Alarm for power limit exceeded

## Installation
### 1. Preparation
- Install PlatformIO (recommended)
  - [PlatformIO IDE for VSCode](https://platformio.org/install/ide?install=vscode)
  - [PlatformIO IDE for Atom](https://platformio.org/install/ide?install=atom)
  - [PlatformIO Core (CLI)](https://docs.platformio.org/en/latest/core/installation.html)
- Install required libraries:
  - [SignalK/SenseESP](https://github.com/SignalK/SensESP)
  - [sensesp/OneWire](https://github.com/PaulStoffregen/OneWire)
  - [INA226](https://github.com/RobTillaart/INA226)

### 2. Clone the repository
- Clone or download this repository to your local machine:
  ```sh
  git clone https://github.com/mw8010/Batteriemonitor.git
  cd Batteriemonitor
  ```

### 3. Upload the code
Open the project in PlatformIO.
Upload the code to your ESP32.

### 4. Configure SenseESP
After the first start, the ESP32 will open an access point.
Connect to the network and enter the Wi-Fi credentials (Wi-Fi password: thisisfine).
Use the SenseESP web UI to configure the battery (see Configuration) and connect the ESP32 to the network where the Signal K server is running.
The data should now appear on the Signal K server under the configured paths.

## Configuration
The following parameters can be adjusted via the SenseESP web UI:
- Battery type (LiFePO4, AGM, Gel, Lead-Acid)
- Battery voltage (12V, 24V, 48V (LiFePO4 only))
- Battery capacity in Ah
- Maximum shunt current
- Shunt voltage in mV
- Battery name for Signal K

**After each configuration change, the ESP32 must be restarted.**

## Usage
- Once set up, the ESP32 continuously transmits measurement values to Signal K.
- Configuration changes can be made via the web UI.
- Alarm notifications are triggered when defined thresholds are exceeded.

### Hardware Installation Notes
**Shunt Resistor (Low-Side Measurement)**

- The shunt resistor must be installed between the negative battery terminal and the system ground (GND).
- Connect the negative battery terminal to one end of the shunt resistor.
- Connect the other end of the shunt resistor to the system ground (GND).
- All consumers and the ESP32 remain connected to system ground as usual.
- Ensure that all connections are tight and secure to ensure accurate measurements.

**INA226 Sensor (Low-Side Wiring)**

- Connect the INA226 sensor to the shunt resistor according to the schematic:
  - `IN+` of the INA226 to the end of the shunt connected to the negative battery terminal.
  - `IN-` of the INA226 to the end of the shunt connected to system ground (GND).
  - `VBUS` connected to system ground (GND).
- Connect the INA226 sensor to the ESP32 according to the schematic.

**ESP32**

- Connect the ESP32 to the INA226 sensor and the OneWire temperature sensor according to the schematic.
- Ensure that the ESP32 is properly powered (e.g., via a DC-DC Buck Step Down Converter).

**PCB**

- To facilitate the connection of the components, you can use the Gerber files in the gerber folder to order a suitable PCB.

![PCB](pic/platine.jpeg)

### Enclosure

In the `/3d-files` folder, you will find STEP files for a suitable enclosure that you can print to protect and mount the finished PCB.

## Used Components
- [ESP32 D1 Mini](https://de.aliexpress.com/item/1005006267267848.html)
- [INA226 CJMCU](https://de.aliexpress.com/item/1005001972537281.html)
- [3A Mini DC-DC Buck Step Down Converter](https://de.aliexpress.com/item/1005005505907937.html)
- [DS18B20 Temperature Sensor](https://de.aliexpress.com/item/1005005488542048.html)

## Repository Files
- **`/gerber`** – Contains Gerber files for the PCB.
- **`/schematic`** – Contains the circuit diagram.
- **`/3d-files`** – Contains the STEP files for a suitable enclosure.

## License
- **Software:** [GNU General Public License v3.0](license.md)  
- **Hardware:** [CERN Open Hardware Licence v2 - Strongly Reciprocal](license.md)

## Acknowledgments
This project is based on the [Battery Monitor Project by Techstyleuk](https://github.com/Techstyleuk/SensESP_3_Battery_Monitor).