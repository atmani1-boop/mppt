# Schematics Requirements - MPPT 150W Boost Converter

## Overview

This document specifies the schematic requirements for the MPPT 150W boost converter with ESP32-C6 and smart home integration. The design is organized into functional blocks for clarity.

## Functional Blocks

### 1. Power Stage (Boost Converter)
**Function**: Step-up DC-DC converter, 18–40V PV input → 48–129V output

**Topology**: Synchronous boost converter (continuous conduction mode preferred)

**Key Components**:
- **Inductor (L1)**: 47–100µH, saturation current ≥12A, DCR <10mΩ
  - Recommended: Würth 744355247 (47µH, 17A sat) or Bourns SRP1265A-100M (100µH, 12.5A sat)
  - Shielded type preferred for EMI reduction
  
- **Low-Side MOSFET (Q1)**: 60V Vds, Rds(on) ≤20mΩ @ 10V Vgs, Id ≥20A
  - Recommended: IRFB4115 (60V, 104A, 3.7mΩ) or equivalent
  - TO-220 or D2PAK package with thermal pad
  
- **High-Side MOSFET (Q2)**: 150V Vds, Rds(on) ≤50mΩ @ 10V Vgs, Id ≥5A
  - Recommended: IPP60R125CP (CoolMOS, 600V, 125mΩ) or IPP80N06S2L-07 (60V, 80A, 7mΩ) for synchronous rectifier
  - Alternative: Use Schottky diode (MBR40250, 40A, 250V) if non-synchronous
  
- **Input Capacitor (C_in)**: 470µF/63V low ESR polymer or electrolytic
  - Parallel with 10µF ceramic (X7R, 50V) for high frequency filtering
  
- **Output Capacitor (C_out)**: 2× 220µF/160V low ESR electrolytic or polymer
  - Series voltage rating: ≥160V (for 129V + margin)
  - ESR <100mΩ for ripple current handling
  - Parallel with 4.7µF ceramic (X7R, 200V) for HF filtering

**Gate Driver**: See dedicated section (MOSFET_Driver_Boost.md)

### 2. Microcontroller Unit (ESP32-C6)
**Component**: ESP32-C6-WROOM-1 module

**Power Supply**:
- 3.3V regulated supply from LDO or buck converter
- Input: PV rail (18–40V) stepped down via buck converter (e.g., LM2596, TPS54331) or LDO if low power
- Recommended: Buck converter for efficiency (TPS54331 or similar, 3.3V @ 500mA minimum)
- Decoupling: 10µF + 100nF ceramic near ESP32-C6 VDD pins

**Connections**:
- PWM output (GPIO3) → Gate driver input
- I²C bus (GPIO8=SDA, GPIO9=SCL) → INA226 sensors
- Thread FDTRGB LED (GPIO5) → WS2812B data input
- BMS interface (GPIO16/17 or as needed) → UART/I²C/CAN transceiver
- LED driver PWM (GPIO6) → 0–10V DAC or PWM output circuit
- Protection inputs: OVP latch (GPIO10), thermal sense (GPIO11 ADC)
- Antenna: PCB trace or U.FL connector for 2.4GHz (Thread/WiFi/BLE)

**Peripherals on ESP32-C6**:
- LEDC (PWM): GPIO3 for boost converter control
- I²C: GPIO8/9 for INA226 sensors (400kHz)
- RMT: GPIO5 for WS2812B LED control
- UART: GPIO16/17 for BMS communication (or USB-CDC for debug)
- ADC: GPIO11 for thermal sensor (NTC)
- DAC or PWM: GPIO6 for LED driver dimming (0–10V analog control)

**Boot and Reset**:
- EN pin: Pull-up 10kΩ + 1µF capacitor to GND, manual reset button optional
- GPIO0: Pull-up 10kΩ (boot mode selection if needed)
- USB-JTAG: Internal on ESP32-C6, connect USB-C for programming/debug

### 3. Current and Voltage Sensors (INA226)
**Component**: 2× INA226 (I²C current/voltage monitor)

#### INA226 #1 (PV Side, I²C address 0x40)
- **Vbus**: PV voltage (0–40V) direct connection via voltage divider if >36V, or direct if <36V
- **Shunt**: 0.001Ω (1mΩ), 3W rated
  - Kelvin connection (4-wire) to shunt resistor
  - Placement: Between PV+ and boost converter input
- **I²C Address**: 0x40 (A0=GND, A1=GND)
- **Voltage Range**: 0–40V (Vbus max = 36V, use divider if PV >36V)
- **Current Range**: 0–10A max

#### INA226 #2 (Output Side, I²C address 0x41)
- **Vbus**: Output voltage (48–129V) via resistive divider to keep <36V (5:1 divider, e.g., 100kΩ + 25kΩ)
- **Shunt**: 0.01Ω (10mΩ), 1W rated
  - Kelvin connection (4-wire) to shunt resistor
  - Placement: Between boost output and BMS/LED output connector
- **I²C Address**: 0x41 (A0=VCC, A1=GND)
- **Voltage Range**: 48–129V → 9.6–25.8V after 5:1 divider
- **Current Range**: 0–3A max

**I²C Bus**:
- Pull-up resistors: 2.2kΩ to 3.3V on SDA and SCL
- Bus speed: 400kHz (fast mode)
- Decoupling: 100nF ceramic on each INA226 VCC pin

**PCB Layout**:
- Kelvin traces for shunts (separate sense and power traces)
- Short, wide traces for high current paths
- Ground plane under INA226 for noise immunity

### 4. MOSFET Gate Driver (Boost Converter)
**Component**: IR2110 or IR2104 (high-voltage half-bridge driver)

See dedicated document: `MOSFET_Driver_Boost.md`

**Summary**:
- PWM input from ESP32-C6 (3.3V logic, GPIO3)
- High-side and low-side outputs for Q1 and Q2
- Bootstrap circuit for high-side drive (UF4007 diode + 10µF ceramic cap)
- VCC supply: 12V from PV rail via small buck or direct connection
- Deadtime: ~500ns (integrated in IR2110/IR2104)
- Gate resistors: 10–22Ω series, 10kΩ pull-down on MOSFET gates

### 5. BMS Interface (1S 304Ah LiFePO4)
**Battery**: Single LiFePO4 cell, 3.2V nominal, 304Ah capacity

**External BMS**: Includes voltage booster to 48–129V output rail

**Signals to/from ESP32-C6**:
- **Charge Enable** (GPIO output): Digital output to BMS charge control relay/MOSFET
  - Logic level: 3.3V (use level shifter if BMS requires 5V)
  - Drive circuit: GPIO → transistor or opto-isolator → relay coil or MOSFET gate
  
- **Discharge Enable** (GPIO output): Digital output for discharge control
  
- **SOC Readout** (I²C/UART/CAN): Battery State of Charge from BMS
  - If I²C: Use same bus as INA226 (different address)
  - If UART: GPIO16/17 (RX/TX)
  - If CAN: Add CAN transceiver (e.g., MCP2551, TJA1050) to GPIO pins
  
- **Fault Detection** (GPIO input): BMS fault signal (active low or high)
  - Pull-up/pull-down resistor on GPIO
  - Optionally: Use opto-isolator for isolation
  
- **Cell Temperature** (NTC or digital sensor):
  - If NTC: 10kΩ thermistor → voltage divider → ESP32-C6 ADC (GPIO11)
  - If digital (e.g., DS18B20, TMP102): I²C or 1-Wire interface

**Connector**: XT60 or Anderson Powerpole for high voltage output (48–129V, 10A rated minimum)

See detailed specification: `BMS_LED_Interface.md`

### 6. LED Driver Interface
**Output**: 48–129V high voltage rail for LED driver

**Dimming Control**:
- **DALI+ over IP**: Network-based control (TCP port 55825, see `DALI_Plus_IP.md`)
- **PWM Output**: 0–10V analog or PWM signal for LED driver dimming
  - GPIO6 (ESP32-C6) → PWM signal
  - PWM to 0–10V conversion:
    - Option 1: RC filter (1kΩ + 10µF) + op-amp buffer (e.g., LM358) + voltage divider
    - Option 2: DAC IC (e.g., MCP4725) via I²C, output 0–10V with op-amp stage
  
**Isolation** (optional):
- If LED driver is on separate HV rail, use opto-coupler for PWM signal isolation
- Recommended: TLP250 or similar for PWM isolation

**Connector**: Phoenix Contact 2-pin (5.08mm pitch) or dedicated LED driver connector

### 7. Thread FDTRGB Status LED
**Component**: WS2812B or APA102 addressable RGB LED (6-pin or 4-pin package)

**Control**:
- GPIO5 (ESP32-C6) → WS2812B data input
- RMT peripheral for precise timing (800kHz data rate for WS2812B)
- Logic level: 5V data preferred (use level shifter if needed, e.g., 74HCT125)

**Power**:
- 5V supply from buck converter (separate from 3.3V logic rail)
- Current: <60mA per LED at full white brightness
- Decoupling: 100µF capacitor near LED

**Color Coding** (see `Thread_FDTRGB_LED.md`):
- F (Fault): Red blinking
- D (Disconnect): Orange solid
- T (Thread): Green (connected), Blue (joining)
- R (Running): Green pulsing
- G (Good): Solid green
- B (Battery): Color gradient by SOC

### 8. Protection Circuits
See detailed specification: `Protection_Circuits.md`

#### Reverse Polarity Protection (PV Input)
- **P-MOSFET**: IRF4905 or similar (55V, -74A, Rds(on) 20mΩ)
- **Zener Diode**: 18V Zener on gate (protects gate if PV > Vgs max)
- **Placement**: Source to PV+, Drain to boost converter input
- **Gate**: Connected to PV− (GND) via 10kΩ resistor

#### Over-Voltage Protection (OVP)
- **Comparator**: LM393 or TL331
- **Threshold**: 140V (set by resistive divider from output voltage)
  - Divider: 470kΩ + 4.7kΩ (140V → 1.4V at comparator input)
  - Reference: 1.25V (TL431 shunt regulator or voltage divider from 3.3V)
- **Output**: Latch to ESP32-C6 GPIO (GPIO10) and hardware shutdown
  - Latch circuit: SR flip-flop (CD4043 or discrete) or software latch
  - Hardware action: Disable PWM via AND gate on PWM signal

#### Over-Current Protection (OCP)
- **Software**: Via INA226 current readings (10A PV side, 3A output side)
- **Hardware**: Optional fast comparator on shunt voltage
  - Threshold: 10mV for 10A on 1mΩ shunt (PV side)
  - Action: Latch PWM disable via GPIO

#### Thermal Protection
- **Sensor**: NTC 10kΩ thermistor on MOSFET Q1 heatsink/case
- **Circuit**: Voltage divider (10kΩ NTC + 10kΩ resistor to 3.3V) → ESP32-C6 ADC (GPIO11)
- **Threshold**: 85°C (software monitoring and shutdown)
- **Alternative**: Hardware comparator (LM35 temp sensor + LM393) for immediate shutdown

#### Input Fuse
- **Type**: 15A fast-blow fuse (optional but recommended)
- **Placement**: On PV+ input, before reverse polarity protection

### 9. Matter/Thread/WiFi/BLE Antenna
**Antenna**: 2.4GHz for Thread, WiFi 6, and BLE 5.3

**Options**:
1. **PCB Trace Antenna**: Meandered or inverted-F antenna on PCB edge
   - Requires 50Ω impedance matching network
   - Keep-out area around antenna (no copper, no components)
   - Ground plane cutout for proper radiation
   
2. **External Antenna**: U.FL connector on PCB
   - Recommended for better range and flexibility
   - 2.4GHz ceramic chip antenna or whip antenna
   - Connector: U.FL or IPEX

**Matching Network**:
- Pi or L-network for impedance matching (50Ω to ESP32-C6 RF output)
- Use VNA or follow ESP32-C6 reference design

### 10. Power Supply for ESP32-C6 and Peripherals
**Input**: PV rail (18–40V)

**Outputs**:
- 3.3V @ 500mA for ESP32-C6 and logic
- 5V @ 200mA for WS2812B LED and peripherals
- 12V @ 100mA for IR2110 gate driver (optional, can derive from PV if 12V available)

**Topology**:
- **Buck Converter**: TPS54331 or LM2596 (18–40V → 5V @ 1A)
  - Output 5V for peripherals
  - Second stage: LDO (LM1117-3.3 or AMS1117-3.3) for 3.3V logic
  - Alternative: Use buck-buck cascade (18–40V → 5V → 3.3V both buck)
  
- **12V for Gate Driver**:
  - Option 1: Linear regulator from PV (only if PV ≥15V)
  - Option 2: Small buck converter (18–40V → 12V @ 100mA)

**Decoupling**:
- Input: 10µF ceramic on buck converter input
- Output 5V: 22µF + 100nF ceramic
- Output 3.3V: 10µF + 100nF ceramic near ESP32-C6

## Schematic Organization (Multi-Page)

Recommended schematic pages:

1. **Page 1: Cover Sheet**
   - Title block, revision history, bill of materials summary
   
2. **Page 2: Power Input and Protection**
   - PV input connector
   - Reverse polarity protection (P-MOSFET)
   - Input fuse
   - Input capacitor
   
3. **Page 3: Boost Converter Power Stage**
   - Inductor L1
   - MOSFETs Q1, Q2
   - Gate driver (IR2110/IR2104)
   - Output capacitors
   - Switch node and high voltage output
   
4. **Page 4: Microcontroller (ESP32-C6)**
   - ESP32-C6-WROOM-1 module
   - 3.3V power supply (LDO)
   - Boot and reset circuits
   - USB programming interface
   - Antenna matching network
   
5. **Page 5: Sensors and I²C Bus**
   - INA226 #1 (PV side)
   - INA226 #2 (Output side)
   - Shunt resistors with Kelvin connections
   - I²C pull-ups and bus connections
   
6. **Page 6: Protection Circuits**
   - OVP comparator and latch
   - Thermal sensor (NTC)
   - OCP comparator (optional)
   - Protection GPIOs to ESP32-C6
   
7. **Page 7: BMS and LED Interfaces**
   - BMS connector and signals (Charge/Discharge Enable, SOC, Fault, Temp)
   - LED driver PWM output (0–10V circuit)
   - Thread FDTRGB status LED (WS2812B)
   - Level shifters if needed
   
8. **Page 8: Connectors and Debug**
   - PV input connector (MC4 or Phoenix)
   - BMS/LED output connector (XT60)
   - Programming header (UART or USB)
   - I²C debug header
   - Antenna connector (U.FL)

## Net Naming Conventions

Use clear, descriptive net names:

### Power Nets
- `PV+`, `PV-`: PV input power
- `PV_PROT+`: After reverse polarity protection
- `BOOST_SW`: Switch node (between Q1 and Q2/L1)
- `OUT+`, `OUT-`: High voltage output (48–129V)
- `VCC_12V`: 12V rail for gate driver
- `VCC_5V`: 5V rail for peripherals
- `VCC_3V3`: 3.3V logic rail
- `GND`: Common ground (star ground at ESP32-C6)
- `PGND`: Power ground (separate from logic ground if possible)

### Signal Nets
- `PWM_BOOST`: PWM signal from ESP32-C6 to gate driver
- `I2C_SDA`, `I2C_SCL`: I²C bus
- `SHUNT_PV_P`, `SHUNT_PV_N`: Kelvin connection for PV shunt (INA226 #1)
- `SHUNT_OUT_P`, `SHUNT_OUT_N`: Kelvin connection for output shunt (INA226 #2)
- `BMS_TX`, `BMS_RX`: UART to BMS
- `BMS_CHG_EN`: Charge enable to BMS
- `BMS_FAULT`: Fault input from BMS
- `LED_FDTRGB_DATA`: WS2812B data line
- `LED_DIM_PWM`: LED driver PWM output
- `OVP_LATCH`: OVP signal to ESP32-C6
- `THERMAL_SENSE`: NTC thermistor voltage to ADC

### Protection Nets
- `OVP_COMP_OUT`: Comparator output for over-voltage
- `TEMP_ADC`: Temperature sensor to ADC

## Layout Constraints

### High Voltage Clearances
- **HV traces (48–129V)**: Minimum 3mm clearance to logic/ground traces
- **Creepage and clearance**: Follow IEC 60664-1 for pollution degree 2
  - 130V DC: 1.5mm minimum creepage, 3mm recommended for safety margin

### Thermal Zones
- **MOSFET Q1**: Copper area ≥10cm² or heatsink attachment
- **MOSFET Q2**: Copper area ≥10cm² (less critical if synchronous rectifier)
- **Inductor L1**: Copper area ≥10cm² for heat dissipation
- **Shunt Resistors**: Wide traces (≥2mm) to/from shunts

### EMI Considerations
- **Minimize switch node area**: Keep BOOST_SW net as small as possible (star connection at Q1, Q2, L1)
- **Short gate traces**: Gate driver to MOSFET gates <50mm, low inductance
- **Ground plane**: Solid ground plane on layer 2 (4-layer PCB)
- **Power plane**: Separate power plane for high current paths
- **Filter capacitors**: Close to noise sources (input/output caps near MOSFETs)

### PCB Stackup (4-Layer Recommended)
1. **Layer 1 (Top)**: Signal, components, high voltage traces
2. **Layer 2**: Ground plane (GND)
3. **Layer 3**: Power plane (VCC_3V3, VCC_5V, PV+)
4. **Layer 4 (Bottom)**: Signal, additional routing, thermal pads

### Current Handling
- **PV+ traces**: ≥2mm width (10A current)
- **OUT+ traces**: ≥1.5mm width (3A current)
- **Switch node (BOOST_SW)**: ≥2mm width, short length
- **Thermal vias**: 0.3mm diameter, 0.6mm pitch under MOSFETs and inductor

## Standards and Certifications

### Safety Standards
- **IEC 62109-1**: Safety of power converters for PV systems (general)
- **IEC 62109-2**: Particular requirements for inverters
- **IEC 60664-1**: Insulation coordination for low-voltage equipment

### Communication Standards
- **Matter (CSA)**: Certification required for Matter branding and interoperability
- **Thread**: IEEE 802.15.4, Thread Group certification
- **DALI+ (IEC 62386-104)**: DALI over IP protocol specification
- **Bluetooth Mesh**: Bluetooth SIG qualification

### EMI/EMC Standards
- **FCC Part 15**: Radiated and conducted emissions (USA)
- **CE/EN 55022**: Electromagnetic compatibility (Europe)
- **CISPR 22**: Information technology equipment emissions

## Component Selection Guidelines

### MOSFETs
- Low Rds(on) for efficiency (Q1: <20mΩ, Q2: <50mΩ if sync rectifier)
- Low Qg (gate charge) for switching losses
- Avalanche rated for robustness
- Thermal resistance: Rθ(JC) <1°C/W for Q1

### Inductor
- Saturation current ≥12A (peak current in CCM)
- Low DCR (<10mΩ) for efficiency
- Shielded type for EMI reduction
- Core material: Ferrite or powdered iron

### Capacitors
- **Input**: Low ESR (<100mΩ), ripple current rating >2A RMS
- **Output**: High voltage rated (≥160V), low ESR, ripple current >1A RMS
- **Ceramic**: X7R or X5R dielectric (not Y5V, due to capacitance loss)

### Gate Driver
- Bootstrap capable (IR2110, IR2104)
- Integrated deadtime generation
- 3.3V logic compatible input (or use level shifter)

## Design Validation Checklist

Before finalizing schematics:

- [ ] Boost converter duty cycle range verified (17%–86% for 18–40V → 48–129V)
- [ ] MOSFET voltage ratings confirmed (Q1: 60V, Q2: 150V minimum)
- [ ] Inductor saturation current ≥12A verified
- [ ] Capacitor voltage ratings confirmed (Input: 63V, Output: 160V)
- [ ] INA226 Vbus within limits (PV <36V direct or via divider, Output via 5:1 divider)
- [ ] ESP32-C6 GPIO assignments reviewed and no conflicts
- [ ] I²C addresses unique (INA226 #1: 0x40, #2: 0x41)
- [ ] High voltage clearances ≥3mm on layout
- [ ] Thermal management for MOSFETs and inductor planned
- [ ] Antenna matching network designed or reference design followed
- [ ] All connectors rated for voltage and current (PV: 10A, Output: 10A, HV rated)
- [ ] Protection circuits tested (OVP threshold 140V, OCP 10A/3A, Thermal 85°C)

## Reference Designs

- **ESP32-C6-DevKitC-1**: Espressif official schematics for ESP32-C6 module
- **IR2110 Application Note AN-978**: High and low side driver application
- **INA226 EVM**: Texas Instruments evaluation module schematics
- **Boost Converter Design**: TI Power Stage Designer tool or AN-1197

## Notes for Designer

1. **BMS Voltage Boost Clarification**: Confirm with BMS supplier how 1S 3.2V cell is boosted to 48–129V output (integrated in BMS or external DC-DC module).

2. **Synchronous vs. Diode Rectifier**: Decide if high-side Q2 is synchronous MOSFET (higher efficiency, more complex control) or Schottky diode (simpler, lower efficiency).

3. **Thread Antenna**: PCB trace antenna requires RF expertise and tuning. External antenna (U.FL + chip antenna) is easier for prototyping.

4. **LED Driver Integration**: Clarify if LED driver is external module (e.g., Mean Well HLG series with 0–10V dimming) or integrated on this PCB.

5. **Enclosure**: If DIN rail mounting or IP-rated enclosure is required, mechanical design must account for connector placement and thermal management.

6. **Certifications**: Matter CSA and DALI Alliance certifications require compliance testing. Budget time and cost for certification labs.

See `Hardware_Questions_for_Designer.md` for complete checklist.
