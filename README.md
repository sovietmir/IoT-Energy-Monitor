# IoT Energy Monitor with ESP8266

## Overview
This project is an IoT-based energy monitoring system that uses an ESP8266 microcontroller to measure and report:
- Mains electricity parameters (voltage, current, power, etc.) using a PZEM-004T sensor
- Environmental conditions (temperature and humidity) using an AHT10 sensor

The system provides:
- Real-time monitoring via MQTT
- Web interface for configuration and data visualization
- OTA (Over-The-Air) firmware updates
- LED indicators for system status

## Hardware Components

### Main Components
- **ESP8266-12E** microcontroller (WiFi enabled)
- **PZEM-004T** energy monitoring module
- **AHT10** temperature and humidity sensor
- **5V Power Supply** for the PZEM module
- **3.3V Voltage Regulator** for the ESP8266
- **LED Indicators** (Green for normal operation, Red for errors)
- **Resistors** for proper circuit operation

### Pin Connections
| ESP8266 Pin | Connected To          |
|-------------|-----------------------|
| GPIO12      | PZEM-004T TX          |
| GPIO13      | PZEM-004T RX          |
| GPIO5 (SCL) | AHT10 SCL             |
| GPIO4 (SDA) | AHT10 SDA             |
| GPIO14      | Green LED             |
| GPIO16      | Red LED               |
|             |                       |

To ensure proper booting and operation of the ESP8266, GPIO15 must be pulled down to GND via a 10 kΩ resistor, and CH_PD must be pulled up to VCC (3.3V) via a 10 kΩ resistor.

## Software Features

### Core Functionality
- WiFi connection management with fallback to AP mode
- MQTT client for publishing sensor data
- HTTP server for configuration and monitoring
- OTA updates capability
- Configuration management with JSON files

### Sensor Monitoring
- **MainsMonitor** class handles PZEM-004T sensor:
  - Voltage, current, power, energy, frequency, power factor
  - Periodic readings (configurable interval)
  - Data validation
  - HTTP and MQTT reporting

- **AHT10Monitor** class handles AHT10 sensor:
  - Temperature and humidity
  - Periodic readings
  - HTTP and MQTT reporting

### Status Indicators
- **LEDHandler** class manages LED indicators:
  - Multiple blinking modes (once, twice, fast, etc.)
  - Green LED for normal operation
  - Red LED for errors

## Setup Instructions

1. **Hardware Assembly**:
   - Connect components according to the schematic
   - Ensure proper power supply (5V for PZEM, 3.3V for ESP8266)

2. **Software Configuration**:
   - Set up WiFi credentials in `config.json`
   - Configure MQTT broker details
   - Adjust sensor reading intervals as needed

3. **Initial Operation**:
   - Device will attempt to connect to configured WiFi
   - Falls back to AP mode if connection fails
   - Access web interface to configure settings

## Usage

### Web Interface
- Access the device's IP address in a web browser
- Configure WiFi, MQTT, and sensor settings
- View real-time sensor data

### MQTT Topics
The device publishes to the following topics (with configurable prefix):
- `voltage`
- `current`
- `power`
- `energy`
- `frequency`
- `powerfactor`
- `temperature`
- `humidity`

### LED Indicators
- **Green LED**:
  - Solid: Normal operation
  - Blinking: System activity
- **Red LED**:
  - Blinking patterns indicate specific errors
  - Continuous: Critical error

## Dependencies

- Arduino Framework
- ESP8266 Core for Arduino
- PZEM004Tv30 Library
- AHT10 Library
- ArduinoJson
- IoTesp8266Framework (custom framework)

## Schematic
![Circuit Diagram](documentation/wiring.svg)

## License
This project is released into the public domain.
Anyone can use it for any purpose.

## Future Enhancements
- Enhance Web Interface
   - Add historical data visualization with charts/graphs
   - Include real-time data streaming via WebSocket
- LED Indicator Optimization
   - Implement standardized blink patterns (Morse-style codes) for specific error types
   - Add status documentation overlay in web interface explaining LED patterns
