# Bill of Materials - MPPT 150W Boost Converter

## Overview

Complete bill of materials for the MPPT 150W boost converter with ESP32-C6, smart home integration, and BMS interface.

## BOM Summary

- **Total Components**: ~80 (approximate)
- **Estimated Cost**: $60–80 USD (quantities of 1-10, excluding PCB)
- **Lead Time**: 2–4 weeks (standard components)

---

## Main Components

| Ref Des | Description | Part Number | Manufacturer | Qty | Unit Price | Supplier | Supplier Part # |
|---------|-------------|-------------|--------------|-----|------------|----------|-----------------|
| U1 | MCU Module ESP32-C6-WROOM-1 (8MB) | ESP32-C6-WROOM-1-N8 | Espressif | 1 | $3.50 | Mouser | 356-ESP32C6WROOM1N8 |
| U2 | MOSFET Driver IR2110 | IR2110S | Infineon | 1 | $2.50 | Mouser | 942-IR2110S |
| U3 | Current/Voltage Monitor INA226 | INA226AIDGSR | Texas Instruments | 1 | $2.80 | Mouser | 595-INA226AIDGSR |
| U4 | Current/Voltage Monitor INA226 | INA226AIDGSR | Texas Instruments | 1 | $2.80 | Mouser | 595-INA226AIDGSR |
| U5 | Buck Converter 3.3V | TPS54331DR | Texas Instruments | 1 | $1.80 | Mouser | 595-TPS54331DR |
| U6 | Comparator (OVP) | LM393DR | Texas Instruments | 1 | $0.40 | Mouser | 926-LM393DR |
| U7 | Op-Amp (LED dimming) | LM358DR | Texas Instruments | 1 | $0.50 | Mouser | 926-LM358DR |
| Q1 | MOSFET Low-Side 60V | IRFB4115PBF | Infineon | 1 | $2.50 | Mouser | 942-IRFB4115PBF |
| Q2 | MOSFET High-Side 600V | IPP60R125CP | Infineon | 1 | $1.80 | Mouser | 726-IPP60R125CP |
| Q3 | P-MOSFET Reverse Protection | IRF4905PBF | Infineon | 1 | $1.50 | Mouser | 942-IRF4905PBF |
| L1 | Inductor 47µH 17A | 744355247 | Würth Elektronik | 1 | $3.50 | Mouser | 710-744355247 |
| D1 | Diode Ultrafast Bootstrap | UF4007 | ON Semiconductor | 1 | $0.15 | Mouser | 512-UF4007 |
| D2 | Zener 18V 1W | 1N4746A | ON Semiconductor | 1 | $0.20 | Mouser | 512-1N4746A |
| LED1 | RGB Addressable LED | WS2812B | Worldsemi | 1 | $0.50 | Adafruit | #1655 |

**Subtotal (Main ICs)**: ~$24

---

## Power Components

| Ref Des | Description | Value | Package | Qty | Unit Price | Supplier Part # |
|---------|-------------|-------|---------|-----|------------|-----------------|
| C_in1 | Capacitor Polymer | 470µF/63V | Radial | 2 | $1.50 | Panasonic EEF-UE0J471R |
| C_in_cer | Capacitor Ceramic | 10µF/50V X7R | 1206 | 2 | $0.30 | Murata GRM31CR61H106KA12L |
| C_out1 | Capacitor Electrolytic | 220µF/160V | Radial | 2 | $2.00 | Nichicon UPW1C221MPD |
| C_out_cer | Capacitor Ceramic | 4.7µF/100V X7R | 1206 | 2 | $0.40 | TDK C3216X7R2A475K160AA |
| C_boot | Capacitor Ceramic | 10µF/25V X7R | 1206 | 1 | $0.20 | Murata GRM31CR61E106KA12L |
| C_VCC | Capacitor Ceramic | 10µF/25V X7R | 1206 | 2 | $0.20 | Murata GRM31CR61E106KA12L |
| R_SHUNT_PV | Shunt Resistor | 1mΩ 3W | 3921 | 1 | $1.50 | Vishay WSLP3921L1000FEA |
| R_SHUNT_OUT | Shunt Resistor | 10mΩ 1W | 2512 | 1 | $0.80 | Vishay WSLP2512R0100FEA |

**Subtotal (Power Components)**: ~$20

---

## Passive Components

### Resistors (0603, 1%, 0.125W unless noted)

| Ref Des | Value | Qty | Unit Price | Total | Supplier Part # |
|---------|-------|-----|------------|-------|-----------------|
| R_HO_G, R_LO_G | 10Ω, 15Ω | 2 | $0.02 | $0.04 | Yageo RC0603FR-07 series |
| R_HO_PD, R_LO_PD, R_pullup | 10kΩ | 10 | $0.02 | $0.20 | Yageo RC0603FR-0710KL |
| R_OVP1 | 470kΩ 1% 0.5W | 1 | $0.10 | $0.10 | Vishay CRCW0805470KFKEA |
| R_OVP2 | 4.7kΩ | 1 | $0.02 | $0.02 | Vishay CRCW08054K70FKEA |
| R_SDA, R_SCL (I²C pull-up) | 2.2kΩ | 2 | $0.02 | $0.04 | Yageo RC0603FR-072K2L |
| R_misc (dividers, filters) | Various | 20 | $0.02 | $0.40 | Yageo RC0603 series |

**Subtotal (Resistors)**: ~$0.80

### Capacitors (0603/0805, X7R/X5R unless noted)

| Ref Des | Value | Voltage | Qty | Unit Price | Total | Supplier Part # |
|---------|-------|---------|-----|------------|-------|-----------------|
| C_decouple (ESP32, ICs) | 100nF | 50V | 10 | $0.05 | $0.50 | Murata GRM188R71H104KA93D |
| C_LED_power | 100µF | 6.3V | 1 | $0.30 | $0.30 | Panasonic EEE-FK0J101P |
| C_misc (filters) | Various | 50V | 10 | $0.05 | $0.50 | Murata/TDK series |

**Subtotal (Capacitors)**: ~$1.30

---

## Connectors

| Ref Des | Description | Pins | Qty | Unit Price | Supplier Part # |
|---------|-------------|------|-----|------------|-----------------|
| J1 | Phoenix Contact Terminal | 2 | 1 | $0.80 | Phoenix MSTB 2,5/ 2-ST-5,08 |
| J2 | XT60 Connector (male) | 2 | 1 | $1.50 | Amass XT60H-M |
| J3 | USB-C Receptacle | 16 | 1 | $0.80 | GCT USB4105-GF-A |
| J4 | Pin Header 2.54mm | 4 | 1 | $0.20 | Sullins PRPC001SAAN-RC |
| J5 | U.FL Antenna Connector | - | 1 | $0.50 | Hirose U.FL-R-SMT-1(10) |
| J6 | Phoenix Contact Terminal | 2 | 1 | $0.80 | Phoenix MSTB 2,5/ 2-ST-5,08 |
| J7 | Phoenix Contact Terminal | 8 | 1 | $2.50 | Phoenix MSTB 2,5/ 8-ST-5,08 |

**Subtotal (Connectors)**: ~$7.10

---

## Optional Components

| Description | Part Number | Qty | Unit Price | Supplier Part # | Notes |
|-------------|-------------|-----|------------|-----------------|-------|
| CAN Transceiver (if CAN BMS) | MCP2551-I/SN | 1 | $1.20 | Microchip MCP2551-I/SN | Optional |
| I²C DAC (if used for LED dimming) | MCP4725A0T-E/CH | 1 | $1.50 | Microchip MCP4725A0T-E/CH | Alternative to PWM |
| Level Shifter (LED data) | SN74HCT125DR | 1 | $0.40 | TI SN74HCT125DR | Optional if WS2812B accepts 3.3V |
| Fuse 15A | 0215015.MXP | 1 | $0.50 | Littelfuse 0215015.MXP | Recommended |
| Fuse Holder | 3557-2 | 1 | $0.80 | Keystone 3557-2 | If fuse used |
| External 2.4GHz Antenna | FXP.10 | 1 | $3.00 | Taoglas FXP.10 | If external antenna |
| Heatsink for Q1 | TO-220 heatsink | 1 | $1.00 | Aavid 576802B00000G | If needed |

**Subtotal (Optional)**: ~$5–10 (depending on options)

---

## PCB

| Description | Specifications | Qty | Unit Price | Notes |
|-------------|----------------|-----|------------|-------|
| PCB 4-layer FR4 | 100×80mm, 1.6mm thick, ENIG finish | 1 | $15–25 | JLCPCB, PCBWay, or OSH Park |

**PCB Cost**: ~$15–25 (prototype quantities 1–10)

---

## Assembly Hardware

| Description | Qty | Unit Price | Notes |
|-------------|-----|------------|-------|
| M3 Screws (PCB mounting) | 4 | $0.10 | 8–10mm length |
| M3 Standoffs (brass or nylon) | 4 | $0.15 | 10mm height |
| Thermal Grease (for MOSFETs) | 1g | $1.00 | Arctic MX-4 or equivalent |
| Heat-shrink tubing (wire protection) | 1m | $1.00 | Various sizes |

**Subtotal (Hardware)**: ~$2

---

## BOM Cost Breakdown

| Category | Cost (USD) | Notes |
|----------|------------|-------|
| Main ICs and Modules | $24 | ESP32-C6, drivers, sensors |
| Power Components | $20 | Capacitors, inductors, MOSFETs |
| Passive Components | $2 | Resistors, small capacitors |
| Connectors | $7 | Phoenix, XT60, USB-C |
| Optional Components | $5–10 | Fuse, antenna, level shifter |
| PCB (4-layer) | $15–25 | Prototype qty 1–10 |
| Assembly Hardware | $2 | Screws, standoffs, thermal grease |
| **Total** | **$75–90** | Excluding labor, shipping |

**Note**: Prices are estimates based on single-unit quantities. Bulk pricing (100+) can reduce total cost by 30–50%.

---

## Sourcing Notes

### Primary Suppliers
- **Mouser Electronics**: https://www.mouser.com (USA, worldwide shipping)
- **Digikey**: https://www.digikey.com (USA, worldwide shipping)
- **LCSC**: https://www.lcsc.com (China, lower cost for bulk)

### PCB Fabrication
- **JLCPCB**: https://jlcpcb.com (low-cost PCBs, SMT assembly available)
- **PCBWay**: https://www.pcbway.com (quality PCBs, good service)
- **OSH Park**: https://oshpark.com (USA-based, purple PCBs, higher quality)

### Lead Times
- **Standard Components**: 1–2 weeks (Mouser, Digikey)
- **ESP32-C6 Module**: 2–4 weeks (check stock, may be longer)
- **PCB Fabrication**: 1–2 weeks (standard service), 2–5 days (expedited)
- **Total Project Lead Time**: 3–6 weeks (design to assembly)

---

## Availability and Alternatives

### Critical Components (Check Stock)

| Component | Primary Part # | Alternative Part # | Notes |
|-----------|----------------|-------------------|-------|
| ESP32-C6-WROOM-1 | ESP32-C6-WROOM-1-N8 | ESP32-C6-WROOM-1-N4 | N4 has 4MB flash (adequate) |
| Inductor 47µH | Würth 744355247 | Bourns SRP1265A-100M | 100µH alternative |
| Q1 MOSFET | IRFB4115PBF | IPB80N06S2L-07 | Similar specs |
| Q2 MOSFET/Diode | IPP60R125CP | MBR40250G (diode) | Diode = non-synchronous |
| INA226 | INA226AIDGSR | INA219AIDCNR | INA219 = lower voltage range |

### Obsolescence Risk
- **Low Risk**: Most TI, Infineon, Würth components are active and widely stocked
- **Medium Risk**: ESP32-C6 is relatively new (2022), ensure adequate stock or use ESP32-C3/S3 as alternative (requires firmware changes)

---

## Downloadable BOM

**Format**: CSV (Excel-compatible)

**Columns**: Ref Des, Qty, Description, Manufacturer, Part Number, Supplier, Supplier Part #, Unit Price, Total Price

**Availability**: Available in repository as `hardware/BOM_MPPT_150W_Boost.csv` (to be generated from this document)

---

## Assembly Notes

### SMD vs. Through-Hole
- **SMD Components**: U1–U7, resistors, capacitors (0603/0805/1206)
- **Through-Hole**: L1 (inductor), Q1-Q3 (MOSFETs), D1-D2, electrolytic capacitors, connectors

### Assembly Options
1. **Hand Soldering**: Feasible for prototype quantities (requires fine soldering iron, flux, magnification)
2. **Reflow Oven**: Recommended for SMD components (use solder paste and stencil)
3. **PCB Assembly Service**: JLCPCB, PCBWay offer SMT assembly for $20–50 (check component availability in their libraries)

### Special Considerations
- **IR2110 SOIC-14**: Hand-solderable but requires fine pitch (0.65mm)
- **ESP32-C6 Module**: Pre-assembled module, no soldering of internal components needed
- **WS2812B LED**: Sensitive to heat, use low temperature or hand-solder carefully

---

## References

- Mouser Electronics Catalog
- Digikey Product Index
- JLCPCB Parts Library
- Manufacturer Datasheets (linked in individual specification documents)

## Revision History

- **v1.0** (2025-12-15): Initial BOM for MPPT 150W boost converter

---

**End of BOM**
