# Pinout and Net Map - ESP32-C6 MPPT Controller

## Overview

This document specifies the complete GPIO pinout for the ESP32-C6-WROOM-1 module and net naming conventions for the MPPT 150W boost converter schematic.

## ESP32-C6-WROOM-1 Module Pinout

### Module Pin Count
The ESP32-C6-WROOM-1 is available in two variants:
- **WROOM-1-N4**: 4MB flash, 30 pins
- **WROOM-1-N8**: 8MB flash, 30 pins

We recommend **WROOM-1-N8** for future firmware expansion (Matter stack, OTA images).

### Power Pins
| Pin | Name | Function | Connection |
|-----|------|----------|------------|
| 1 | GND | Ground | Connect to GND plane |
| 2 | 3V3 | Power supply | 3.3V from LDO (470µF + 100nF decoupling) |
| 3-9 | GND | Ground | Connect to GND plane (thermal vias under module) |

**Note**: ESP32-C6 operating voltage is 3.0–3.6V. Use a stable 3.3V ±5% supply.

### GPIO Assignments

#### PWM and Power Control
| GPIO | Pin | Function | Signal Net | Description |
|------|-----|----------|------------|-------------|
| GPIO3 | 15 | PWM Output | `PWM_BOOST` | Boost converter PWM (LEDC CH0, 50kHz) |

**Configuration**:
- LEDC peripheral, low-speed mode
- Timer 0, 13-bit resolution (8192 steps)
- Frequency: 50kHz (configurable in firmware)
- Duty cycle: 0–100% (firmware controlled via MPPT algorithm)

#### I²C Bus (Sensors)
| GPIO | Pin | Function | Signal Net | Description |
|------|-----|----------|------------|-------------|
| GPIO8 | 12 | I²C SDA | `I2C_SDA` | Data line for INA226 sensors |
| GPIO9 | 13 | I²C SCL | `I2C_SCL` | Clock line for INA226 sensors |

**Configuration**:
- I²C Master mode, 400kHz (Fast Mode)
- External pull-ups: 2.2kΩ to 3.3V
- Devices on bus:
  - INA226 #1 (PV side): Address 0x40
  - INA226 #2 (Output side): Address 0x41
  - BMS (optional, if I²C interface): Address TBD (e.g., 0x55)

#### Status LED (Thread FDTRGB)
| GPIO | Pin | Function | Signal Net | Description |
|------|-----|----------|------------|-------------|
| GPIO5 | 18 | RMT Output | `LED_FDTRGB_DATA` | WS2812B addressable RGB LED data |

**Configuration**:
- RMT peripheral for precise timing (800kHz WS2812B protocol)
- 5V logic level recommended (use 74HCT125 level shifter if needed)
- LED power: 5V rail, separate from 3.3V logic

#### LED Driver Control
| GPIO | Pin | Function | Signal Net | Description |
|------|-----|----------|------------|-------------|
| GPIO6 | 19 | PWM/DAC | `LED_DIM_PWM` | LED driver dimming (0–10V analog or PWM) |

**Configuration**:
- Option 1: PWM output → RC filter + op-amp buffer → 0–10V analog
- Option 2: I²C DAC (MCP4725) controlled via I²C bus
- Frequency: 1kHz PWM if used directly by LED driver (check driver specs)

#### BMS Communication
| GPIO | Pin | Function | Signal Net | Description |
|------|-----|----------|------------|-------------|
| GPIO16 | 26 | UART RX | `BMS_RX` | UART receive from BMS (if UART interface) |
| GPIO17 | 27 | UART TX | `BMS_TX` | UART transmit to BMS (if UART interface) |

**Alternative Configurations**:
- **I²C BMS**: Use GPIO8/9 (same bus as sensors, different address)
- **CAN BMS**: GPIO16/17 connect to MCP2551 or TJA1050 CAN transceiver
  - GPIO16 → CAN_TX (CAN transceiver input)
  - GPIO17 → CAN_RX (CAN transceiver output)

**UART Configuration** (if used):
- Baud rate: 9600 or 115200 (BMS dependent)
- Format: 8N1 (8 data bits, no parity, 1 stop bit)
- Pull-up on RX recommended (10kΩ)

#### BMS Control Signals
| GPIO | Pin | Function | Signal Net | Description |
|------|-----|----------|------------|-------------|
| GPIO18 | 28 | Digital Output | `BMS_CHG_EN` | Charge enable to BMS |
| GPIO19 | 29 | Digital Output | `BMS_DISCHG_EN` | Discharge enable to BMS |
| GPIO7 | 11 | Digital Input | `BMS_FAULT` | Fault signal from BMS (active low) |

**Configuration**:
- `BMS_CHG_EN`: Active high (3.3V = enable charge). Use transistor or opto-isolator if BMS requires higher voltage.
- `BMS_DISCHG_EN`: Active high (3.3V = enable discharge).
- `BMS_FAULT`: Input with pull-up (10kΩ). Interrupt-driven for fast fault response.

#### Protection Signals
| GPIO | Pin | Function | Signal Net | Description |
|------|-----|----------|------------|-------------|
| GPIO10 | 14 | Digital Input | `OVP_LATCH` | Over-voltage protection latch (active high) |
| GPIO11 | 23 | ADC Input | `THERMAL_SENSE` | NTC thermistor voltage (MOSFET temperature) |

**Configuration**:
- `OVP_LATCH`: Input with pull-down (10kΩ). When high, over-voltage detected (>140V).
- `THERMAL_SENSE`: ADC1 channel, 12-bit resolution
  - NTC 10kΩ thermistor in voltage divider (10kΩ + NTC to 3.3V)
  - Temperature calculation: Steinhart-Hart equation or lookup table
  - Shutdown threshold: 85°C

#### USB and Programming
| GPIO | Pin | Function | Signal Net | Description |
|------|-----|----------|------------|-------------|
| GPIO12 | 24 | USB D− | `USB_DN` | USB programming and debug (internal USB-JTAG) |
| GPIO13 | 25 | USB D+ | `USB_DP` | USB programming and debug (internal USB-JTAG) |

**Configuration**:
- USB-C connector for programming and debug
- ESP32-C6 internal USB-JTAG/Serial (no external USB-to-UART needed)
- Automatic driver installation on Windows/macOS/Linux

#### Boot and Reset
| GPIO | Pin | Function | Signal Net | Description |
|------|-----|----------|------------|-------------|
| EN | 10 | Enable/Reset | `ESP_EN` | Active high enable, reset button pulls low |
| GPIO9 | 13 | Boot Mode | `I2C_SCL` | (Shared with I²C SCL, default pull-up) |

**Boot Circuit**:
- EN pin: 10kΩ pull-up to 3.3V + 1µF capacitor to GND
- Manual reset button: SPST switch between EN and GND
- GPIO9: Default pull-up (no special boot mode selection needed for normal operation)

#### Antenna (Thread/WiFi/BLE)
| GPIO | Pin | Function | Signal Net | Description |
|------|-----|----------|------------|-------------|
| RF | 30 | RF Output | `ANT_2G4` | 2.4GHz antenna connection |

**Antenna Options**:
1. **PCB Trace Antenna**: Meandered or inverted-F design on board edge
   - Requires 50Ω impedance matching (pi-network: C-L-C)
   - Keep-out zone: No copper/components within 5mm of antenna
   
2. **External Antenna**: U.FL connector
   - U.FL connector on PCB, cable to external ceramic chip antenna
   - Recommended for better range and flexibility
   - Part example: Taoglas FXP.10 (2.4GHz ceramic patch antenna)

**Matching Network**:
- Follow ESP32-C6 Hardware Design Guidelines
- Typical: 0Ω series resistor + pi-network (2.7pF + 3.9nH + 1.5pF) for 50Ω
- Tune with VNA (Vector Network Analyzer) or accept reference design

### Reserved/NC Pins
| GPIO | Pin | Function | Notes |
|------|-----|----------|-------|
| GPIO0 | 16 | Boot/Strapping | Pull-up 10kΩ (default high for normal boot) |
| GPIO1, GPIO2, GPIO4 | 17, 22, 20 | General Purpose | Reserved for future expansion |
| GPIO14, GPIO15 | - | Internal | Not exposed on WROOM-1 module |

**Strapping Pins**:
- GPIO9: Boot mode (default high via pull-up on I²C bus)
- GPIO8: JTAG enable (default low, not critical for USB-JTAG)

## Net Naming Conventions

### Power Rails
| Net Name | Voltage | Description | Max Current |
|----------|---------|-------------|-------------|
| `PV+` | 18–40V | PV panel positive input | 10A |
| `PV-` or `GND` | 0V | PV panel negative / ground | 10A |
| `PV_PROT+` | 18–40V | After reverse polarity MOSFET | 10A |
| `BOOST_SW` | 0–129V | Switch node (Q1/Q2/L1 junction) | 10A peak |
| `OUT+` | 48–129V | High voltage output to BMS/LED | 3A |
| `OUT-` or `GND` | 0V | Output negative / ground | 3A |
| `VCC_12V` | 12V | Gate driver supply | 100mA |
| `VCC_5V` | 5V | Peripherals (WS2812B, etc.) | 200mA |
| `VCC_3V3` | 3.3V | Logic supply (ESP32-C6) | 500mA |
| `GND` | 0V | Common ground | All |
| `PGND` | 0V | Power ground (if separated) | High current |

**Grounding Strategy**:
- Single-point star ground at ESP32-C6 GND pins
- Power ground (PGND) for high current paths (PV, boost, output)
- Logic ground (GND) for sensors, MCU, low-current signals
- Connect PGND and GND at single point near ESP32-C6 or input capacitor

### Signal Nets - PWM and Control
| Net Name | Type | Source | Destination | Description |
|----------|------|--------|-------------|-------------|
| `PWM_BOOST` | Output | ESP32-C6 GPIO3 | IR2110 HIN or LIN | Boost PWM signal |
| `GATE_Q1_LOW` | Output | IR2110 LO | Q1 Gate | Low-side MOSFET gate |
| `GATE_Q2_HIGH` | Output | IR2110 HO | Q2 Gate | High-side MOSFET gate |
| `LED_DIM_PWM` | Output | ESP32-C6 GPIO6 | 0–10V circuit | LED dimming control |

### Signal Nets - I²C Bus
| Net Name | Type | Devices | Description |
|----------|------|---------|-------------|
| `I2C_SDA` | Bidirectional | ESP32-C6, INA226 #1, INA226 #2, BMS (optional) | I²C data |
| `I2C_SCL` | Output | ESP32-C6, INA226 #1, INA226 #2, BMS (optional) | I²C clock |

**Pull-ups**: 2.2kΩ to 3.3V on both SDA and SCL

### Signal Nets - Current and Voltage Sensing
| Net Name | Type | Connected To | Description |
|----------|------|--------------|-------------|
| `SHUNT_PV_P` | Sense | INA226 #1 V+ | Kelvin connection, positive side of PV shunt |
| `SHUNT_PV_N` | Sense | INA226 #1 V- | Kelvin connection, negative side of PV shunt |
| `PV_BUS` | Power/Sense | INA226 #1 Vbus | PV voltage sensing (0–40V) |
| `SHUNT_OUT_P` | Sense | INA226 #2 V+ | Kelvin connection, positive side of output shunt |
| `SHUNT_OUT_N` | Sense | INA226 #2 V- | Kelvin connection, negative side of output shunt |
| `OUT_BUS_DIV` | Sense | INA226 #2 Vbus | Output voltage after 5:1 divider (9.6–25.8V) |

**Kelvin Connection**: Use 4-wire sensing for shunt resistors. Power current flows through wide traces, sense traces are separate and thin (0.2mm width, directly to INA226 V+/V- pins).

### Signal Nets - BMS Interface
| Net Name | Type | Source | Destination | Description |
|----------|------|--------|-------------|-------------|
| `BMS_TX` | Output | ESP32-C6 GPIO17 | BMS RX | UART transmit to BMS |
| `BMS_RX` | Input | BMS TX | ESP32-C6 GPIO16 | UART receive from BMS |
| `BMS_CHG_EN` | Output | ESP32-C6 GPIO18 | BMS charge control | Charge enable (active high) |
| `BMS_DISCHG_EN` | Output | ESP32-C6 GPIO19 | BMS discharge control | Discharge enable (active high) |
| `BMS_FAULT` | Input | BMS fault output | ESP32-C6 GPIO7 | Fault signal (active low) |
| `BMS_TEMP` | Analog | Temperature sensor | ESP32-C6 GPIO11 (alt) | If separate temp sensor |

**Alternative**: If BMS uses CAN:
- `CAN_TX` (GPIO17) → MCP2551 TXD
- `CAN_RX` (GPIO16) ← MCP2551 RXD
- `CAN_H`, `CAN_L` → BMS CAN bus

### Signal Nets - Status LED (Thread FDTRGB)
| Net Name | Type | Source | Destination | Description |
|----------|------|--------|-------------|-------------|
| `LED_FDTRGB_DATA` | Output | ESP32-C6 GPIO5 | WS2812B DIN | Addressable LED data |
| `VCC_5V` | Power | 5V rail | WS2812B VDD | LED power supply |
| `GND` | Power | Ground | WS2812B GND | LED ground |

**Level Shifter** (if needed):
- If GPIO5 is 3.3V and WS2812B requires 5V logic:
  - Use 74HCT125 buffer (powered by 5V, inputs tolerate 3.3V)
  - Or use WS2812B-V5 variant (accepts 3.3V input at 5V VDD)

### Signal Nets - Protection
| Net Name | Type | Source | Destination | Description |
|----------|------|--------|-------------|-------------|
| `OVP_LATCH` | Input | LM393 comparator output | ESP32-C6 GPIO10 | Over-voltage detect (active high) |
| `THERMAL_SENSE` | Analog | NTC voltage divider | ESP32-C6 GPIO11 | MOSFET temperature (ADC) |
| `OCP_LATCH` | Input (optional) | OCP comparator | ESP32-C6 GPIO (TBD) | Over-current detect |

### Signal Nets - Antenna and RF
| Net Name | Type | Description |
|----------|------|-------------|
| `ANT_2G4` | RF | 2.4GHz antenna connection from ESP32-C6 RF pin |
| `ANT_MATCH_1`, `_2`, `_3` | RF | Matching network nodes (C-L-C pi-network) |

**Impedance**: 50Ω from ESP32-C6 RF pin through matching network to antenna

### Debug and Programming
| Net Name | Type | Description |
|----------|------|-------------|
| `USB_DP` | USB | USB D+ signal (GPIO13 on ESP32-C6) |
| `USB_DN` | USB | USB D− signal (GPIO12 on ESP32-C6) |
| `ESP_EN` | Reset | ESP32-C6 enable/reset (active high) |

## Schematic Pages and Net Organization

### Page 2: Power Input and Protection
- `PV+`, `PV-` (input connector)
- `PV_PROT+` (after P-MOSFET)
- `GND`, `PGND`

### Page 3: Boost Converter Power Stage
- `PV_PROT+` (input)
- `BOOST_SW` (switch node)
- `OUT+`, `OUT-` (output)
- `GATE_Q1_LOW`, `GATE_Q2_HIGH`
- `VCC_12V` (gate driver supply)

### Page 4: Microcontroller (ESP32-C6)
- `VCC_3V3`, `GND`
- `PWM_BOOST`
- `I2C_SDA`, `I2C_SCL`
- `LED_FDTRGB_DATA`, `LED_DIM_PWM`
- `BMS_TX`, `BMS_RX`, `BMS_CHG_EN`, `BMS_DISCHG_EN`, `BMS_FAULT`
- `OVP_LATCH`, `THERMAL_SENSE`
- `USB_DP`, `USB_DN`, `ESP_EN`
- `ANT_2G4`

### Page 5: Sensors and I²C Bus
- `I2C_SDA`, `I2C_SCL`, `VCC_3V3`, `GND`
- `PV_BUS`, `SHUNT_PV_P`, `SHUNT_PV_N` (INA226 #1)
- `OUT_BUS_DIV`, `SHUNT_OUT_P`, `SHUNT_OUT_N` (INA226 #2)

### Page 6: Protection Circuits
- `OUT+` (voltage divider input)
- `OVP_LATCH` (comparator output)
- `THERMAL_SENSE` (NTC divider output)
- `VCC_3V3`, `GND`

### Page 7: BMS and LED Interfaces
- `BMS_TX`, `BMS_RX`, `BMS_CHG_EN`, `BMS_DISCHG_EN`, `BMS_FAULT`
- `LED_DIM_PWM` (to 0–10V converter)
- `LED_FDTRGB_DATA`, `VCC_5V`, `GND`

### Page 8: Connectors
- `PV+`, `PV-` (MC4 connector)
- `OUT+`, `OUT-` (XT60 connector)
- `USB_DP`, `USB_DN` (USB-C connector)
- `I2C_SDA`, `I2C_SCL`, `VCC_3V3`, `GND` (debug header)
- `ANT_2G4` (U.FL connector or PCB trace)

## GPIO Pin Summary Table

| GPIO | Pin | Dir | Function | Net Name | Notes |
|------|-----|-----|----------|----------|-------|
| GPIO3 | 15 | Out | PWM | `PWM_BOOST` | LEDC CH0, 50kHz |
| GPIO5 | 18 | Out | RMT | `LED_FDTRGB_DATA` | WS2812B |
| GPIO6 | 19 | Out | PWM/DAC | `LED_DIM_PWM` | 0–10V LED dimming |
| GPIO7 | 11 | In | Digital | `BMS_FAULT` | Pull-up, active low |
| GPIO8 | 12 | I/O | I²C SDA | `I2C_SDA` | 2.2kΩ pull-up |
| GPIO9 | 13 | Out | I²C SCL | `I2C_SCL` | 2.2kΩ pull-up |
| GPIO10 | 14 | In | Digital | `OVP_LATCH` | Pull-down, active high |
| GPIO11 | 23 | In | ADC | `THERMAL_SENSE` | ADC1 CH0, temperature |
| GPIO12 | 24 | I/O | USB | `USB_DN` | USB D− (internal) |
| GPIO13 | 25 | I/O | USB | `USB_DP` | USB D+ (internal) |
| GPIO16 | 26 | In | UART RX | `BMS_RX` | UART from BMS |
| GPIO17 | 27 | Out | UART TX | `BMS_TX` | UART to BMS |
| GPIO18 | 28 | Out | Digital | `BMS_CHG_EN` | Charge enable |
| GPIO19 | 29 | Out | Digital | `BMS_DISCHG_EN` | Discharge enable |
| EN | 10 | In | Reset | `ESP_EN` | 10kΩ pull-up + button |
| RF | 30 | RF | Antenna | `ANT_2G4` | 2.4GHz |

**Unused GPIOs**: GPIO0, GPIO1, GPIO2, GPIO4 (reserved for future expansion)

## PCB Layout Considerations

### High-Speed Signals
- **PWM_BOOST**: Keep trace <100mm, route away from sensitive analog signals
- **I2C_SDA/SCL**: Max 200mm trace length, matched length ±10mm, avoid crossing high current paths

### Kelvin Connections (Critical!)
Shunt resistor current sensing requires **Kelvin (4-wire) connections**:

```
     Power Path (wide, 2mm trace)
PV+ ──────┬─[Shunt 1mΩ]─┬────────> Boost Input
          │              │
          │              │
     Sense (thin, 0.2mm trace, direct to INA226)
       SHUNT_PV_P    SHUNT_PV_N
          │              │
          └──> INA226 V+ │
               INA226 V- <┘
```

**Key Points**:
- Power current does NOT flow through sense traces
- Sense traces connect directly to shunt at the resistor pads (not at the wide power traces)
- Minimizes voltage drop errors in current measurement

### Ground Planes
- Solid ground plane on Layer 2
- Star ground at ESP32-C6 GND pins (connect all grounds here)
- Avoid splitting ground plane under RF antenna

### Thermal Vias
- Under ESP32-C6 module: 3×3 array of 0.3mm vias to GND plane
- Under MOSFETs: 5×5 array connected to thermal pad (copper pour on top + bottom layers)
- Under inductor: Copper pour on bottom layer for heat dissipation

## Reference Designators

Use standard reference designators for clarity:

- **U1**: ESP32-C6-WROOM-1 module
- **U2**: IR2110/IR2104 gate driver
- **U3**: INA226 #1 (PV side)
- **U4**: INA226 #2 (Output side)
- **U5**: Buck converter for 3.3V (TPS54331 or similar)
- **U6**: LM393 comparator (OVP)
- **Q1**: Low-side MOSFET (boost converter)
- **Q2**: High-side MOSFET or Schottky diode (boost converter)
- **Q3**: P-MOSFET (reverse polarity protection)
- **L1**: Inductor (boost converter)
- **D1**: Bootstrap diode (gate driver)
- **D2**: Zener diode (P-MOSFET gate protection)
- **LED1**: WS2812B (Thread FDTRGB status LED)
- **R_SHUNT_PV**: 1mΩ shunt (PV current sensing)
- **R_SHUNT_OUT**: 10mΩ shunt (Output current sensing)
- **NTC1**: 10kΩ thermistor (MOSFET temperature)
- **J1**: PV input connector (MC4 or Phoenix)
- **J2**: Output connector (XT60)
- **J3**: USB-C connector (programming)
- **J4**: I²C debug header
- **J5**: U.FL antenna connector (optional)

## Design Validation Checklist

- [ ] All ESP32-C6 GPIOs assigned and documented
- [ ] No GPIO conflicts (no pin used for multiple functions)
- [ ] I²C addresses unique (INA226 #1: 0x40, #2: 0x41)
- [ ] Pull-ups on I²C bus (2.2kΩ to 3.3V)
- [ ] Boot/reset circuit for ESP32-C6 (EN pull-up + button)
- [ ] USB programming interface (GPIO12/13 to USB-C)
- [ ] Antenna matching network designed or reference design used
- [ ] Kelvin connections for shunt resistors verified on layout
- [ ] Power rails decoupled (10µF + 100nF on each supply)
- [ ] Ground plane continuous under ESP32-C6 and RF section
- [ ] Net names consistent across all schematic pages
- [ ] Reference designators unique and sequential

## Notes

1. **Firmware GPIO Configuration**: Update `main/main.c` to match these GPIO assignments.
   
2. **I²C Bus**: Maximum bus capacitance is 400pF for 400kHz operation. Keep traces short.

3. **WS2812B Timing**: RMT peripheral on ESP32-C6 generates precise timing. No external components needed besides level shifter (optional).

4. **Antenna Tuning**: If using PCB trace antenna, reserve space for 0Ω resistors in matching network for tuning.

5. **BMS Interface Flexibility**: Design allows UART, I²C, or CAN interface. Populate components as needed for specific BMS model.

6. **Future Expansion**: GPIO0, GPIO1, GPIO2, GPIO4 available for additional sensors or outputs (e.g., LCD display, relay control).

## References

- ESP32-C6 Datasheet (Espressif Systems)
- ESP32-C6-WROOM-1 Datasheet (Espressif Systems)
- ESP32-C6 Hardware Design Guidelines (Espressif Systems)
- INA226 Datasheet (Texas Instruments)
- WS2812B Datasheet (Worldsemi)
