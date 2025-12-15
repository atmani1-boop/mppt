# Connector Pinning - MPPT 150W Boost Converter

## Overview

This document specifies all external connectors, their pinouts, and electrical specifications for the MPPT boost converter.

## Connector List

| Connector | Type | Function | Voltage | Current | Notes |
|-----------|------|----------|---------|---------|-------|
| J1 | MC4 or Phoenix 2-pin | PV input | 18–40V | 10A | 10mm² wire |
| J2 | XT60 or Anderson Powerpole | BMS/LED HV output | 48–129V | 10A | High voltage rated |
| J3 | USB-C | Programming/debug | 5V | 500mA | USB-JTAG internal |
| J4 | Phoenix 4-pin | I²C debug header | 3.3V | 50mA | Optional |
| J5 | U.FL | Antenna (2.4GHz) | - | - | External antenna |
| J6 | Phoenix 2-pin | LED driver dimming | 0–10V | <10mA | Analog control |
| J7 | Phoenix 8-pin | BMS interface | Mixed | Varies | UART/I²C/CAN + control |

---

## J1: PV Input Connector

### Option 1: MC4 Connectors (Recommended for Solar)

**Type**: MC4 (Multi-Contact 4mm), industry standard for PV panels

**Specifications**:
- **Voltage Rating**: 1000V DC
- **Current Rating**: 30A
- **Wire Size**: 2.5–6mm² (10–14 AWG)
- **Protection**: IP67 (weatherproof)
- **Mating**: Male and female pair

**Pinout**:
| Pin | Signal | Wire Color (typical) |
|-----|--------|---------------------|
| + | PV+ | Red (positive) |
| − | PV− | Black (negative/ground) |

**Recommended Part**: Renogy MC4 Connectors (pair)

**Supplier**: Amazon, Renogy, or solar distributors

### Option 2: Phoenix Contact MSTB 2-Pin (PCB Mount)

**Type**: Phoenix Contact pluggable terminal block

**Specifications**:
- **Voltage Rating**: 320V DC
- **Current Rating**: 12A
- **Wire Size**: Up to 10mm² (8 AWG)
- **Pitch**: 5.08mm
- **Protection**: IP20 (indoor use)

**Pinout**:
| Pin | Signal | Description |
|-----|--------|-------------|
| 1 | PV+ | Positive from solar panel |
| 2 | PV− | Negative/ground |

**Recommended Part**: Phoenix Contact MSTB 2,5/ 2-ST-5,08

**Supplier**: Mouser #651-1757242, Digikey

**PCB Footprint**: 2-pin, 5.08mm pitch, through-hole or SMD variant

### Wiring Notes

- **Wire Gauge**: 10 AWG (5.26mm²) for 10A current
- **Length**: Keep PV wiring short to minimize voltage drop (<2m if possible)
- **Fusing**: 15A fast-blow fuse recommended on PV+ line (see Protection_Circuits.md)
- **Polarity**: Clearly mark PV+ and PV− on PCB silkscreen

---

## J2: High Voltage Output Connector (BMS/LED Driver)

### Option 1: XT60 Connector (Recommended)

**Type**: XT60 (Amass XT series, common in RC/battery applications)

**Specifications**:
- **Voltage Rating**: 500V DC (XT60H variant)
- **Current Rating**: 60A continuous
- **Contacts**: Gold-plated copper
- **Keying**: Polarized (prevents reverse connection)

**Pinout**:
| Pin | Signal | Description |
|-----|--------|-------------|
| + (male pin) | OUT+ | High voltage positive (48–129V) |
| − (female pin) | OUT− | High voltage negative/ground |

**Recommended Part**: XT60H-M (male) + XT60H-F (female) pair

**Supplier**: Amazon, HobbyKing, or Amass distributors

### Option 2: Anderson Powerpole (45A)

**Type**: Anderson Powerpole (modular power connector)

**Specifications**:
- **Voltage Rating**: 600V DC
- **Current Rating**: 45A
- **Contacts**: Silver-plated copper
- **Keying**: Reversible but color-coded (red = positive, black = negative)

**Pinout**:
| Color | Signal | Description |
|-------|--------|-------------|
| Red | OUT+ | High voltage positive |
| Black | OUT− | High voltage negative/ground |

**Recommended Part**: Anderson Powerpole 1327 (red) + 1327G6 (black)

**Supplier**: Powerwerx, Mouser, Digikey

### Option 3: Phoenix Contact (PCB Mount)

**Type**: Phoenix Contact MSTB 2-pin, 5.08mm pitch

**Specifications**: Same as PV connector option 2

**Use Case**: If all connectors should match (PV and output both Phoenix)

### Wiring Notes

- **Wire Gauge**: 10 AWG (5.26mm²) for 10A current at high voltage
- **Insulation**: Use wire rated for ≥300V (high voltage application)
- **Clearance**: Maintain ≥3mm clearance between OUT+ and GND traces on PCB
- **Labeling**: Mark voltage range (48–129V DC) on connector or silkscreen

---

## J3: USB Programming/Debug Connector

### Type: USB-C Receptacle

**Function**: ESP32-C6 internal USB-JTAG for programming and serial debug

**Specifications**:
- **USB Standard**: USB 2.0 (Full Speed)
- **Voltage**: 5V (not used for power, only data)
- **Data Lines**: D+ (GPIO13), D− (GPIO12)

**Pinout** (USB-C receptacle, simplified):
| Pin | Signal | Description |
|-----|--------|-------------|
| A1, B1, A12, B12 | GND | Ground |
| A4, B4, A9, B9 | VBUS | 5V (connected to 5V rail for detection) |
| A6 | D+ | USB data positive (GPIO13) |
| A7 | D− | USB data negative (GPIO12) |
| B6 | D+ | USB data positive (duplicate) |
| B7 | D− | USB data negative (duplicate) |

**Recommended Part**: GCT USB4105-GF-A (mid-mount USB-C receptacle)

**Supplier**: Mouser #640-USB4105-GF-A, Digikey

**PCB Footprint**: USB-C 16-pin, mid-mount or through-hole variant

**Note**: ESP32-C6 has internal USB-JTAG, no external USB-to-UART chip needed.

---

## J4: I²C Debug Header (Optional)

### Type: 0.1" (2.54mm) Pin Header, 4-pin

**Function**: Debug access to I²C bus (for testing sensors, BMS, etc.)

**Specifications**:
- **Pitch**: 2.54mm (0.1")
- **Pins**: 4-pin, single row
- **Style**: Male header (accepts female jumper wires)

**Pinout**:
| Pin | Signal | Voltage | Description |
|-----|--------|---------|-------------|
| 1 | VCC_3V3 | 3.3V | Power supply for I²C devices |
| 2 | GND | 0V | Ground |
| 3 | SDA | 3.3V | I²C data line (GPIO8) |
| 4 | SCL | 3.3V | I²C clock line (GPIO9) |

**Recommended Part**: Sullins PRPC001SAAN-RC (1×4 pin header, 2.54mm)

**Supplier**: Mouser #200-PRPC001SAANRC, Digikey

**Usage**: Connect logic analyzer or I²C debugger (e.g., Bus Pirate, Saleae Logic)

**Note**: Do NOT connect 5V I²C devices (use logic level shifter if needed).

---

## J5: Antenna Connector (2.4GHz)

### Type: U.FL (IPEX) Connector

**Function**: External 2.4GHz antenna for Thread/WiFi/BLE

**Specifications**:
- **Frequency**: 2.4–2.5GHz
- **Impedance**: 50Ω
- **Connector**: U.FL female receptacle (on PCB), U.FL male on antenna cable

**Pinout**:
| Pin | Signal | Description |
|-----|--------|-------------|
| Center | ANT_2G4 | RF signal (from ESP32-C6 RF pin) |
| Shield | GND | Ground (RF shield) |

**Recommended Part**: Hirose U.FL-R-SMT-1(10) (PCB receptacle)

**Supplier**: Mouser #798-U.FL-R-SMT-110, Digikey

**Antenna Cable**: U.FL to RP-SMA cable (100mm, 1.13mm coax)

**External Antenna**: 2.4GHz ceramic chip antenna or whip antenna (e.g., Taoglas FXP.10)

**Alternative**: Omit J5 if using PCB trace antenna (see Pinout_and_NetMap.md)

---

## J6: LED Driver Dimming Connector

### Type: Phoenix Contact MSTB 2-pin, 5.08mm pitch

**Function**: 0–10V analog dimming output to LED driver

**Specifications**:
- **Voltage Range**: 0–10V
- **Current**: <10mA (input impedance of LED driver is typically >10kΩ)
- **Pitch**: 5.08mm

**Pinout**:
| Pin | Signal | Description |
|-----|--------|-------------|
| 1 | DIM+ | 0–10V analog dimming signal |
| 2 | DIM− | Dimming ground (common with LED driver) |

**Recommended Part**: Phoenix Contact MSTB 2,5/ 2-ST-5,08

**Supplier**: Mouser #651-1757242

**Wiring**: Use shielded twisted pair cable if long run (>1m) to reduce noise

---

## J7: BMS Interface Connector

### Type: Phoenix Contact MSTB 8-pin, 5.08mm pitch (or equivalent)

**Function**: Communication and control signals to/from BMS

**Specifications**:
- **Pins**: 8-pin (expandable to 10-pin if more signals needed)
- **Voltage**: Mixed (3.3V logic, high voltage power)
- **Current**: Varies (logic <10mA, power up to 10A)

**Pinout**:
| Pin | Signal | Voltage | Direction | Description |
|-----|--------|---------|-----------|-------------|
| 1 | OUT+ | 48–129V | Output | High voltage positive from BMS |
| 2 | OUT− | 0V | Output | High voltage negative/ground |
| 3 | CHG_EN | 3.3V | Input to BMS | Charge enable (ESP32 GPIO18) |
| 4 | DISCHG_EN | 3.3V | Input to BMS | Discharge enable (ESP32 GPIO19) |
| 5 | FAULT | 3.3V | Output from BMS | Fault signal (to ESP32 GPIO7) |
| 6 | COMM_A | 3.3V/5V | Bidirectional | UART TX / CAN_H / SDA |
| 7 | COMM_B | 3.3V/5V | Bidirectional | UART RX / CAN_L / SCL |
| 8 | GND | 0V | Common | Signal ground |

**Recommended Part**: Phoenix Contact MSTB 2,5/ 8-ST-5,08

**Supplier**: Mouser #651-1757287

**Alternative**: Use separate connectors for power (XT60, pins 1–2) and signals (Phoenix 6-pin, pins 3–8)

**Wiring Notes**:
- **OUT+/OUT−**: Use high voltage rated wire (≥300V insulation), 10 AWG
- **Logic Signals**: Use 22–24 AWG stranded wire
- **COMM_A/B**: Twisted pair for UART or CAN (reduces EMI)

---

## PCB Connector Placement

### Recommended Layout

```
Top Edge:
┌──────────────────────────────────────────┐
│  [J1 PV Input]         [J2 HV Output]    │
│                                          │
│              [PCB Components]             │
│                                          │
│  [J3 USB-C]  [J4 I2C]  [J6 LED Dim]     │
└──────────────────────────────────────────┘
Bottom Edge:
        [J7 BMS Interface]    [J5 Antenna]
```

**Guidelines**:
- **Power Connectors (J1, J2)**: On opposite edges for clean cable routing
- **Programming (J3)**: Accessible from front/edge for easy connection
- **Debug (J4)**: Near ESP32-C6 for short traces
- **Antenna (J5)**: On edge or corner, away from power traces (EMI)
- **BMS (J7)**: Grouped near control circuits

---

## Connector BOM

| Ref Des | Type | Pins | Pitch | Qty | Supplier Part # | Notes |
|---------|------|------|-------|-----|-----------------|-------|
| J1 | Phoenix Contact | 2 | 5.08mm | 1 | Phoenix MSTB 2,5/ 2-ST-5,08 | PV input (alt: MC4) |
| J2 | XT60 | 2 | - | 1 | Amass XT60H-M | HV output (alt: Anderson) |
| J3 | USB-C Receptacle | 16 | - | 1 | GCT USB4105-GF-A | USB programming |
| J4 | Pin Header | 4 | 2.54mm | 1 | Sullins PRPC001SAAN-RC | I²C debug (optional) |
| J5 | U.FL Receptacle | - | - | 1 | Hirose U.FL-R-SMT-1(10) | Antenna (optional) |
| J6 | Phoenix Contact | 2 | 5.08mm | 1 | Phoenix MSTB 2,5/ 2-ST-5,08 | LED dimming |
| J7 | Phoenix Contact | 8 | 5.08mm | 1 | Phoenix MSTB 2,5/ 8-ST-5,08 | BMS interface |

---

## Design Checklist

- [ ] All connectors selected and footprints verified
- [ ] PV input connector rated for 10A, 40V minimum
- [ ] HV output connector rated for 10A, 129V minimum (use 500V+ rated)
- [ ] USB-C footprint matches ESP32-C6 D+/D− connections
- [ ] I²C debug header accessible from board edge
- [ ] Antenna connector placed on edge, away from power traces
- [ ] LED dimming connector isolated from high voltage traces
- [ ] BMS connector pinout documented and labeled on silkscreen
- [ ] All connectors have strain relief or mounting holes for stability

---

## References

- Phoenix Contact Terminal Block Catalog
- Amass XT60 Datasheet
- Anderson Powerpole Specifications
- USB-C Connector Standards (USB Implementers Forum)

## Revision History

- **v1.0** (2025-12-15): Initial connector pinning specification
