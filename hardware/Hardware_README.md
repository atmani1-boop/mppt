# MPPT 150W Boost Converter - Hardware Documentation

## Overview

This documentation describes the hardware design for a smart MPPT (Maximum Power Point Tracking) 150W boost converter with ESP32-C6 microcontroller and comprehensive smart home integration.

**Key Features:**
- **Power Stage**: 18–40V PV input → 48–129V boost output, 150W maximum
- **MCU**: ESP32-C6-WROOM-1 (RISC-V, WiFi 6, BLE 5.3, Thread IEEE 802.15.4)
- **Smart Home Integration**: Matter over Thread, DALI+ over IP, WiFi 6, Bluetooth Mesh
- **BMS Interface**: 1S LiFePO4 cell (3.2V nominal, 304Ah) with external voltage booster
- **LED Driver**: 48–129V PWM dimmable output with DALI+ IP control
- **Status Indication**: Thread FDTRGB addressable RGB LED (WS2812B)
- **Sensors**: 2× INA226 (I²C) for voltage and current monitoring
- **Protections**: Reverse polarity, OVP, OCP, thermal shutdown
- **Efficiency Target**: >94% at rated power

## System Architecture

The system combines:
1. **Solar PV Input** (18–40V, max 10A) with reverse polarity protection
2. **Synchronous Boost Converter** (150W, 50kHz PWM) controlled by ESP32-C6
3. **High Voltage Output** (48–129V) for BMS and LED driver
4. **Smart Connectivity**: Matter/Thread/DALI+/BLE Mesh/WiFi for smart home integration
5. **Precision Monitoring**: INA226 sensors for real-time voltage/current/power measurements
6. **Visual Status**: Thread FDTRGB LED with 6-state color coding
7. **BMS Communication**: I²C/UART/CAN interface to external BMS

## Thread FDTRGB Status LED

The system uses a 6-pin addressable RGB LED (WS2812B) to indicate operational status:

- **F (Fault)**: Red blinking - OVP, OCP, thermal, or BMS fault
- **D (Disconnect)**: Orange solid - PV disconnected or no sunlight
- **T (Thread)**: Green (connected to Thread network), Blue (joining/commissioning)
- **R (Running)**: Green pulsing - MPPT actively tracking maximum power point
- **G (Good)**: Solid green - Charging BMS, no faults
- **B (Battery)**: Color gradient based on SOC (Red <20%, Yellow 20–80%, Green >80%)

## Smart Home Integration

The MPPT controller supports multiple smart home protocols:

### Matter over Thread
- **Device Type**: Multi-endpoint (Solar Panel + Battery + LED Driver)
- **Clusters**: Power Source, On/Off, Level Control, Temperature, Electrical Measurement
- **Commissioning**: QR code or manual pairing
- **OTA Updates**: Firmware upgrade over Thread or WiFi

### DALI+ over IP (IEC 62386-104)
- **Port**: TCP 55825
- **Discovery**: mDNS service `_dali._tcp.local`
- **Function**: LED driver control (dimming, color, scenes)
- **Integration**: Synced with Matter Level Control cluster

### WiFi 6 & Bluetooth Mesh
- **WiFi**: Telemetry, cloud connectivity, OTA updates
- **BLE Mesh**: Provisioning and local control

### Thread Border Router
- **Mode**: Optional Thread Border Router if WiFi/Ethernet uplink available
- **Mesh**: 802.15.4 @ 2.4 GHz for low-power device connectivity

## BMS Interface (1S 304Ah LiFePO4)

The system interfaces with an external BMS managing a single LiFePO4 cell:

- **Cell**: 1S LiFePO4, 3.2V nominal, 304Ah capacity
- **Voltage Boost**: External BMS includes DC-DC booster to 48–129V output rail
- **Signals**:
  - Charge Enable/Disable (GPIO output from ESP32-C6)
  - Discharge Enable/Disable
  - SOC Readout (I²C/UART/CAN from BMS)
  - Fault Detection (GPIO input)
  - Cell Temperature (NTC or digital sensor)

## Documentation Files

This hardware documentation package includes the following files:

### Design Specifications
1. **Schematics_Requirements.md** - Functional blocks, topology, component selection
2. **Power_Spec_Boost_Converter.md** - Boost converter design calculations and dimensioning
3. **Pinout_and_NetMap.md** - ESP32-C6 GPIO assignments and net naming conventions

### Interface Documentation
4. **Sensor_Interface_INA226.md** - INA226 current/voltage sensor specifications
5. **MOSFET_Driver_Boost.md** - IR2110/IR2104 gate driver circuit
6. **Protection_Circuits.md** - OVP, OCP, thermal protection implementations
7. **BMS_LED_Interface.md** - BMS communication and LED driver interface
8. **Connector_Pinning.md** - All connectors and pinout specifications

### Smart Home Integration
9. **Matter_Thread_Integration.md** - Matter device types, clusters, and Thread mesh
10. **DALI_Plus_IP.md** - DALI+ over IP (IEC 62386-104) for LED control
11. **Thread_FDTRGB_LED.md** - Status LED specifications and color coding

### Physical Design
12. **BOM_MPPT_150W_Boost.md** - Complete bill of materials with part numbers
13. **Hardware_Block_Diagram.md** - System block diagram (ASCII art)
14. **Mechanical_Notes.md** - PCB layout, thermal management, clearances

### Designer Resources
15. **Hardware_Questions_for_Designer.md** - Checklist and clarifications needed

## Firmware Integration

The hardware is designed to work with the existing MPPT firmware in this repository:

### Existing Firmware Components
- **MPPT Algorithm**: P&O (Perturb & Observe) implemented in `mppt_pno.c`
- **ADC Driver**: Voltage and current sensing in `adc.c`
- **PWM Control**: LEDC peripheral for boost converter switching
- **Main Application**: `main/main.c` implements MPPT control loop

### Additional Firmware Requirements
- **ESP-IDF**: ≥5.1 for ESP32-C6 support
- **Matter SDK**: esp-matter for Thread/Matter endpoints
- **OpenThread**: Thread Border Router stack
- **Bluetooth Mesh**: ESP-BLE-MESH for provisioning
- **DALI+ IP Client**: TCP socket library for DALI PDU encoding/decoding
- **INA226 Driver**: I²C driver for current/voltage sensors
- **WS2812B Driver**: RMT peripheral for Thread FDTRGB LED control
- **BMS Driver**: I²C/UART/CAN communication (BMS model dependent)

### Firmware-Hardware Interface
The firmware controls:
- PWM duty cycle (GPIO3) for boost converter MOSFET switching
- I²C communication (GPIO8/9) with INA226 sensors
- Thread FDTRGB LED data (GPIO5) for status indication
- BMS communication (GPIO16/17 or as configured)
- LED driver PWM (GPIO6) for dimming control
- Protection signals (OVP latch, thermal sense)

## Safety and Compliance

### High Voltage Warnings
⚠️ **WARNING**: This design operates at voltages up to 129V DC, which can be lethal. Proper safety measures must be implemented:
- Minimum 3mm clearance between high voltage traces and logic/ground
- High voltage rated components (capacitors, connectors rated ≥160V)
- Conformal coating for outdoor/humid environments
- Proper enclosure with warning labels
- Fusing on PV input (15A fast-blow recommended)

### Standards Compliance
- **IEC 62109**: Safety requirements for PV power converters
- **IEC 62386-104**: DALI+ over IP protocol
- **Matter CSA Certification**: Required for Matter compatibility
- **EMI/EMC**: Design for minimal electromagnetic interference
- **Thread Certification**: Required for Thread mesh networking

### PCB Layout Best Practices
- 4-layer FR4 PCB recommended
- Dedicated power and ground planes
- High current traces ≥2mm width (PV input, output)
- Thermal vias under MOSFETs and inductance
- Kelvin connections for current sense shunts
- Minimize switch node area to reduce EMI
- Separate analog and digital grounds (star ground at MCU)

## Getting Started

### For Schematic Designers
1. Start with `Schematics_Requirements.md` for overall architecture
2. Review `Power_Spec_Boost_Converter.md` for boost converter design
3. Use `Pinout_and_NetMap.md` for ESP32-C6 connections
4. Refer to individual interface docs for sensor, driver, and protection circuits
5. Check `Hardware_Questions_for_Designer.md` before finalizing design

### For PCB Layout Engineers
1. Review `Mechanical_Notes.md` for physical constraints
2. Check `BOM_MPPT_150W_Boost.md` for component footprints
3. Follow high voltage clearance requirements (≥3mm)
4. Implement thermal zones for power components
5. Use Kelvin connections for current sensing

### For Firmware Developers
1. Refer to existing firmware in `/mppt_pno.c`, `/adc.c`, `/main/main.c`
2. Implement additional drivers per interface specifications
3. Use `Pinout_and_NetMap.md` for GPIO assignments
4. Integrate Matter/Thread/DALI+ protocol stacks

## Support and Resources

### Component Suppliers
- **Mouser Electronics**: https://www.mouser.com
- **Digikey**: https://www.digikey.com
- **LCSC**: https://www.lcsc.com (for cost-effective alternatives)

### Reference Designs
- ESP32-C6-DevKitC-1 schematics (Espressif)
- IR2110 application notes (Infineon)
- INA226 datasheet and reference designs (Texas Instruments)
- Matter sample devices (CSA Alliance)
- DALI+ Alliance implementation guides

### Development Tools
- **ESP-IDF**: https://github.com/espressif/esp-idf
- **Matter SDK**: https://github.com/espressif/esp-matter
- **KiCad**: Recommended for schematic and PCB design
- **LTspice**: For power stage simulation

## License

This hardware documentation is provided as-is for reference and development purposes. Consult with appropriate regulatory bodies for certification requirements before commercial deployment.

## Authors

- atmani1-boop
- Hardware documentation created for MPPT 150W boost converter project

## Revision History

- **v1.0** (2025-12-15): Initial hardware documentation release
  - 16 comprehensive documentation files
  - ESP32-C6 smart MPPT with Matter/Thread/DALI+ integration
  - 1S 304Ah LiFePO4 BMS interface
  - Thread FDTRGB status LED specification
