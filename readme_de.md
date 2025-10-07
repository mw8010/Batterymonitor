# Batteriemonitor mit INA226 und SenseESP

## Überblick
Dieses Projekt ist ein DIY-Batteriemonitor für unseren Trawler. Es nutzt einen ESP32, um mit dem INA226-Sensor Strom, Spannung und Leistung zu messen. Die erfassten Daten werden über SenseESP in Signal K integriert. Zudem wird ein OneWire-Temperatursensor zur Überwachung der Batterietemperatur verwendet.

## Warum Low-Side-Strommessung?
Die Low-Side-Strommessung bedeutet, dass der Shunt-Widerstand zwischen dem negativen Batteriepol und dem Massepunkt (GND) platziert wird.  
Diese Methode ist besonders für höhere Batteriespannungen (z. B. 48 V) empfohlen, da der INA226 so immer innerhalb seiner Spezifikation arbeitet und keine Fehlmessungen oder Schäden auftreten.

**Vorteile:**
- Der INA226 bleibt immer im sicheren Spannungsbereich (max. 36 V Common-Mode).
- Einfache und sichere Verdrahtung für die meisten Installationen (Fahrzeuge, Boote).
- Kein Spannungsteiler am INA226 nötig.

**Hinweis:**  
Das gemessene Strom-Vorzeichen kann invertiert sein. Dies wird im Code korrigiert.

## Was ist Signal K?
[Signal K](https://signalk.org/) ist ein offenes Datenformat für die maritime Elektronik. Es ermöglicht die Integration und den Austausch von Sensordaten zwischen verschiedenen Geräten und Anwendungen an Bord eines Boots.

## Was ist SenseESP?
[SenseESP](https://github.com/SignalK/SensESP) ist ein Open-Source-Framework, das die einfache Entwicklung von Sensoren ermöglicht, die ihre Daten über WLAN an einen Signal K-Server senden. Es basiert auf dem ESP32 und nutzt eine modulare Architektur für flexible Anpassungen.

## Funktionen
- Messung von Batteriespannung, Strom und Leistung mit INA226
- Berechnung des Ladezustands (SoC) basierend auf Batterietyp und Spannung
- Berechnung der verbleibenden Amperestunden (Ah) und der verbleibenden Laufzeit
- Konfigurierbare Batterieparameter über die Web-UI von SenseESP
- Alarmmeldungen für Shunt-Überspannung, Bus-Überspannung und Leistungsgrenzen
- Temperaturüberwachung über OneWire-Sensor (z.B. DS18B20)

## Hardware-Anforderungen
- **ESP32 D1 Mini** – Mikrocontroller
- **INA226 CJMCU** – Stromsensor
- **DS18B20** – Temperatursensor
- **3A Mini DC-DC Buck Step Down Converter** – Spannungswandler
- **Shunt-Widerstand** – zur Strommessung (angepasst an den maximalen Strom)

## Klemmenbelegung
| Klemme | Bezeichnung         | Beschreibung            |
|--------|---------------------|-------------------------|
| **U1** | Power IN           | + / - (6 - 48V)        |
| **U2** | Temperatursensor   | + / - / Data           |
| **U4** | INA226             | VBUS / IN- / IN+       |

## Signal K Datenpunkte
- `electrical.batteries.<battery_name>.voltage` – Batteriespannung in V
- `electrical.batteries.<battery_name>.current` – Batteriestrom in A
- `electrical.batteries.<battery_name>.power` – Batterieleistung in W
- `electrical.batteries.<battery_name>.stateOfCharge` – Ladezustand in % (Coulomb-Zählung)
- `electrical.batteries.<battery_name>.stateOfCharge_voltageSoC` – Ladezustand in % (spannungsbasiert)
- `electrical.batteries.<battery_name>.ampHours` – verbleibende Ah
- `electrical.batteries.<battery_name>.timeToGo` – verbleibende Zeit in Sekunden
- `electrical.batteries.<battery_name>.temperature` – Batterietemperatur in °K
- `electrical.batteries.<battery_name>.alerts.shuntOverVoltage` – Alarm bei Shunt-Überspannung
- `electrical.batteries.<battery_name>.alerts.busOverVoltage` – Alarm bei Bus-Überspannung
- `electrical.batteries.<battery_name>.alerts.powerOverLimit` – Alarm bei Leistungsüberschreitung

## Installation
### 1. Vorbereitung
- PlatformIO installieren (empfohlen)
  - [PlatformIO IDE für VSCode](https://platformio.org/install/ide?install=vscode)
  - [PlatformIO IDE für Atom](https://platformio.org/install/ide?install=atom)
  - [PlatformIO Core (CLI)](https://docs.platformio.org/en/latest/core/installation.html)
- Benötigte Bibliotheken installieren:
  - [SignalK/SenseESP](https://github.com/SignalK/SensESP)
  - [sensesp/OneWire](https://github.com/SensESP/OneWire)
  - [INA226](https://github.com/RobTillaart/INA226)

### 2. Repository klonen
- Klonen oder laden Sie dieses Repository auf Ihren lokalen Rechner:
  ```sh
  git clone https://github.com/mw8010/Batteriemonitor.git
  cd Batteriemonitor
  ```

### 3. Code hochladen
- Öffnen Sie das Projekt in PlatformIO.
- Laden Sie den Code auf Ihren ESP32.

### 4. SensESP konfigurieren
- Nach dem ersten Start öffnet der ESP32 einen Access Point.
- Verbinden Sie sich mit dem Netzwerk und geben Sie die WLAN-Zugangsdaten ein (WLAN Passwort: thisisfine).
- Über die Web-UI von SensESP können Sie die Batterie konfigurieren (siehe Konfiguration) und den ESP32 in das Netzwerk einbinden, in dem der Signal K Server läuft.
- Die Daten sollten nun im Signal K Server unter den konfigurierten Pfaden erscheinen.

## Konfiguration
Die folgenden Parameter können über die Web-UI von SenseESP angepasst werden:
- Batterietyp (LiFePO4, AGM, Gel, Bleisäure)
- Batteriespannung (12V, 24V oder 48V (nur LiFePO4))
- Batteriekapazität in Ah
- Maximalstrom des Shunts
- Shunt-Spannung in mV
- Batteriename für Signal K

**Nach jeder Änderung der Konfiguration muss der ESP32 neu gestartet werden.**

## Nutzung
- Nach erfolgreicher Einrichtung überträgt der ESP32 kontinuierlich die Messwerte an Signal K.
- Änderungen an der Konfiguration können über die Web-UI vorgenommen werden.
- Alarmmeldungen werden bei Über- oder Unterschreiten der definierten Grenzwerte ausgelöst.

### Hinweise zur Hardware-Installation
**Shunt-Widerstand (Low-Side-Messung)**
- Der Shunt-Widerstand muss zwischen dem negativen Batteriepol und dem System-Massepunkt (GND) installiert werden.
- Verbinden Sie den negativen Batteriepol mit einem Ende des Shunt-Widerstands.
- Verbinden Sie das andere Ende des Shunt-Widerstands mit dem Massepunkt (GND).
- Alle Verbraucher und der ESP32 bleiben wie gewohnt mit Masse verbunden.
- Achten Sie auf sichere und feste Verbindungen für genaue Messungen.

**INA226-Sensor (Low-Side-Verdrahtung)**
- Verbinden Sie den INA226-Sensor mit dem Shunt-Widerstand gemäß Schaltplan:
  - `IN+` des INA226 an das Ende des Shunt, das mit dem negativen Batteriepol verbunden ist.
  - `IN-` des INA226 an das Ende des Shunt, das mit dem Massepunkt (GND) verbunden ist.
  - `VBUS` ebenfalls mit Masse (GND) verbinden.
- Verbinden Sie den INA226-Sensor mit dem ESP32 gemäß Schaltplan.

**ESP32**
- Verbinden Sie den ESP32 mit dem INA226-Sensor und dem OneWire-Temperatursensor gemäß Schaltplan.
- Stellen Sie sicher, dass der ESP32 korrekt mit Strom versorgt wird (z.B. über einen DC-DC Buck Step Down Converter).

**Platine**
- Zur leichteren Verbindung der Komponenten können Sie die Gerber-Dateien im Ordner `/gerber` verwenden, um eine passende Platine zu bestellen.

![Platine](pic/platine.jpeg)

**Gehäuse**
- Im Ordner `/3d-files` finden Sie STEP-Dateien für ein passendes Gehäuse, das Sie ausdrucken können, um die fertige Platine zu schützen und zu montieren.

## Verwendete Komponenten
- [ESP32 D1 Mini](https://de.aliexpress.com/item/1005006267267848.html)
- [INA226 CJMCU](https://de.aliexpress.com/item/1005001972537281.html)
- [3A Mini DC-DC Buck Step Down Converter](https://de.aliexpress.com/item/1005005505907937.html)
- [DS18B20 Temperatursensor](https://de.aliexpress.com/item/1005005488542048.html)

## Dateien im Repository
- **`/gerber`** – Enthält die Gerber-Dateien für die Platine.
- **`/schaltplan`** – Enthält den Schaltplan.
- **`/3d-files`** – Enthält die STEP-Dateien für ein passendes Gehäuse.

## Lizenz
- **Software:** GNU General Public License v3.0
- **Hardware:** CERN Open Hardware Licence v2 - Strongly Reciprocal

## Danksagung
Dieses Projekt basiert auf dem [Battery Monitor Project von Techstyleuk](https://github.com/Techstyleuk/SensESP_3_Battery_Monitor).