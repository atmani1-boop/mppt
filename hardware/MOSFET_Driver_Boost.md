# MOSFET Gate Driver - Boost Converter

## Overview

This document specifies the gate driver circuit for the synchronous boost converter MOSFETs (Q1 low-side, Q2 high-side). The driver provides fast, high-current gate drive capability for efficient switching at 50kHz.

## Driver IC Selection

### Recommended: IR2110 or IR2104 (Infineon/International Rectifier)

Both ICs are high-voltage, high-speed MOSFET drivers suitable for half-bridge and boost converter applications.

#### IR2110 (Dual Channel, Most Common)
- **High-Side + Low-Side**: Independent HO and LO outputs
- **Vcc Range**: 10–20V (logic supply)
- **Vbs Range**: 10–20V (bootstrap supply)
- **Offset Voltage**: Up to 600V (high-side floating)
- **Output Current**: 2A source/sink
- **Deadtime**: None (must be added externally or in firmware)
- **Input Logic**: 3.3V CMOS compatible (VIH ≥2.5V, VIL ≤0.8V)
- **Package**: 14-pin DIP or SOIC
- **Supplier**: Mouser #942-IR2110PBF (DIP) or #942-IR2110S (SOIC)
- **Cost**: ~$2.50

#### IR2104 (Dual Channel, Similar to IR2110)
- **Differences**: Slightly different pinout, same electrical specs
- **Package**: 8-pin DIP or SOIC
- **Supplier**: Mouser #942-IR2104PBF (DIP) or #942-IR2104S (SOIC)

**Selection**: Use **IR2110** for breadboarding (DIP) or **IR2110S** for production (SOIC). IR2104 is also acceptable with minor schematic changes.

## Functional Block Diagram

```
                              Bootstrap Circuit
                              ┌─────────────────┐
                              │  D1 (UF4007)    │
VCC_12V ───────────────────┬──┤                 ├──┐
                           │  └─────────────────┘  │
                           │                       │
                           │                   C_boot (10µF)
                           │                       │
                           │                       │
    ┌──────────────────────┼───────────────────────┼──────────┐
    │      IR2110          │                       │          │
    │                      │                       │          │
    │  VCC (10) ───────────┘                       │          │
    │  VB  (7) ────────────────────────────────────┘          │
    │  VS  (5) ────────────────────┬──────────────────────────┤
    │                              │                          │
    │  HIN (11) <───────┐          │  HO (1) ─────────┐       │
    │  SD  (9)  ────┐   │          │                  │       │
    │  LIN (12) <───┼───┼───── PWM_BOOST (from ESP32-C6)     │
    │  COM (2)  ────┼───┼──────────┼──────┬───────────┼───────┤
    │  GND (3,13)───┼───┘          │      │           │       │
    │               │              │      │           │       │
    │  LO (4) ──────┼──────────────┼──────┼───────────┼───┐   │
    └───────────────┼──────────────┼──────┼───────────┼───┼───┘
                    │              │      │           │   │
                    │              │      │           │   │
                R_LO_G           Q2       │         R_HO_G│
                10-22Ω         (High-   BOOST_SW   10-22Ω│
                    │          Side)      │           │   │
                    │            │        │           │   │
                    ├──> Gate ───┤        │           ├──> Gate
                    │    Q1      │        │           │    Q2
                    │  (Low-     │        │           │
                    │   Side)    │        │           │
                R_LO_PD       Source   Drain      Source
                10kΩ             │        │           │
                    │            │        │           │
                   GND          GND     PV_PROT+     BOOST_SW
```

## Pin Connections (IR2110)

| Pin | Name | Function | Connection | Notes |
|-----|------|----------|------------|-------|
| 1 | HO | High-Side Output | Q2 Gate via 10–22Ω resistor | Source 2A, high-side drive |
| 2 | COM | Logic Ground | GND (common with ESP32-C6) | Reference for logic inputs |
| 3 | GND | Power Ground | GND (star ground at input cap) | Low-side return |
| 4 | LO | Low-Side Output | Q1 Gate via 10–22Ω resistor | Source 2A, low-side drive |
| 5 | VS | High-Side Floating Supply Return | BOOST_SW (switch node) | Bootstrap reference |
| 6 | NC | No Connection | - | Leave unconnected |
| 7 | VB | Bootstrap Supply | Bootstrap cap positive | VB - VS = 10–20V |
| 8 | NC | No Connection | - | Leave unconnected |
| 9 | SD | Shutdown (active low) | VCC_12V (via 10kΩ pull-up) | Tie high for normal operation |
| 10 | VCC | Logic Supply | VCC_12V (12V rail) | 10–20V supply for logic |
| 11 | HIN | High-Side Input | PWM_BOOST (from ESP32-C6 GPIO3) | 3.3V PWM signal |
| 12 | LIN | Low-Side Input | PWM_BOOST_INV (inverted) or GND | Complementary to HIN for synchronous |
| 13 | VSS | Logic Ground | GND (common with COM) | Reference for VCC |
| 14 | NC | No Connection | - | Leave unconnected |

**Note**: For boost converter, we need complementary drive (Q1 ON when Q2 OFF, and vice versa). Options:
1. **Firmware Deadtime**: Generate complementary PWM signals (HIN and LIN) with deadtime in ESP32-C6 firmware.
2. **External Inverter**: Use a small logic inverter (e.g., 74HC04) to invert PWM_BOOST for LIN input.
3. **Simple Boost**: Tie LIN to GND (Q1 always OFF) if using diode rectifier instead of synchronous Q2.

**Recommended**: Use firmware complementary PWM with deadtime (500ns) for synchronous boost.

## Bootstrap Circuit

The high-side driver requires a floating power supply (VB - VS) to drive Q2 gate. This is provided by a **bootstrap circuit**.

### Components

#### D1: Bootstrap Diode
- **Part**: UF4007 (Ultrafast Recovery Diode)
- **Specifications**:
  - Vrrm: 1000V
  - If: 1A average
  - Reverse Recovery: <75ns (ultrafast)
- **Connection**: Anode to VCC_12V, Cathode to VB (IR2110 pin 7)
- **Function**: Charges C_boot when Q1 is ON (VS pulled to GND)
- **Supplier**: Mouser #512-UF4007 or #863-UF4007-E3/54

Alternative: 1N4148 (signal diode, lower current but adequate for small gate charge)

#### C_boot: Bootstrap Capacitor
- **Value**: 10µF
- **Voltage**: 25V minimum (VB - VS ≈ 12V)
- **Type**: Ceramic (X7R or X5R) for low ESR
- **Package**: 1206 or larger (ensure voltage rating)
- **Connection**: Between VB (pin 7) and VS (pin 5) of IR2110
- **Function**: Stores charge to drive Q2 gate
- **Supplier**: Murata GRM31CR61E106KA12L (10µF, 25V, X7R, 1206)

### How Bootstrap Works

1. **Q1 ON, Q2 OFF**: 
   - VS (switch node) is pulled to GND through Q1
   - D1 is forward biased (VCC_12V > VB)
   - Current flows: VCC_12V → D1 → C_boot → VS (GND)
   - C_boot charges to ~12V

2. **Q1 OFF, Q2 ON**:
   - VS rises to output voltage (48–129V)
   - D1 is reverse biased (VB ≈ VS + 12V, much higher than VCC_12V)
   - C_boot discharges to provide gate current for Q2
   - VB - VS remains ~12V (floating supply for high-side driver)

3. **Cycle Repeats**:
   - Every PWM cycle, when Q1 turns ON, C_boot recharges
   - This requires Q1 to turn ON at least once every few milliseconds (not an issue at 50kHz)

**Capacitor Sizing**:
```
Q_gate = Qg × N_pulses (charge needed for N gate drive pulses)
For Q2 with Qg = 48nC, and C_boot = 10µF:
ΔV = Qg / C_boot = 48e-9 / 10e-6 = 4.8mV per pulse
```
This is negligible voltage drop. 10µF is more than sufficient.

## Gate Resistors

### Series Gate Resistors (R_HO_G, R_LO_G)

**Function**: Limit gate charge/discharge current, damp gate ringing, control switching speed.

**Value**: 10–22Ω
- Lower (10Ω): Faster switching, higher switching losses (ringing, EMI)
- Higher (22Ω): Slower switching, reduced EMI, slightly higher switching losses

**Power Rating**: 0.125W (1/8W) SMD resistor (0603 or 0805)

**Placement**: Close to MOSFET gate pin (<10mm)

**Selection**:
- **Q1 (Low-Side)**: 10Ω (needs fast switching due to high current)
- **Q2 (High-Side)**: 15Ω (can be slightly slower)

**Recommended Parts**:
- Yageo RC0603FR-0710RL (10Ω, 1%, 0603)
- Yageo RC0603FR-0715RL (15Ω, 1%, 0603)

### Pull-Down Resistors (R_LO_PD, R_HO_PD)

**Function**: Ensure MOSFET gates are pulled to source when driver is not actively driving (e.g., during power-up).

**Value**: 10kΩ
- Weak pull-down, doesn't interfere with driver operation
- Prevents floating gate (which could cause unintended turn-on)

**Connection**:
- Q1: Gate to Source (GND)
- Q2: Gate to Source (BOOST_SW, the switch node)

**Power Rating**: 0.125W (1/8W) SMD resistor (0603 or 0805)

**Recommended Part**:
- Yageo RC0603FR-0710KL (10kΩ, 1%, 0603)

## PWM Input (from ESP32-C6)

### PWM Signal (GPIO3)

**Logic Levels**:
- ESP32-C6 outputs 3.3V CMOS logic (VOH ≈ 3.3V, VOL ≈ 0V)
- IR2110 input thresholds: VIH ≥ 2.5V, VIL ≤ 0.8V
- **Compatible**: 3.3V logic directly drives IR2110 inputs

**Connection**:
- PWM_BOOST (GPIO3) → HIN (pin 11)
- For synchronous boost: PWM_BOOST_INV → LIN (pin 12)
  - Generate inverted signal via firmware or external inverter gate

**Trace Routing**:
- Keep PWM trace short (<100mm)
- Route away from high-current power traces (avoid coupling)
- Add series resistor (100Ω) at GPIO3 output if EMI issues occur

### Deadtime Generation

**Requirement**: Q1 and Q2 must not be ON simultaneously (shoot-through protection).

**Deadtime**: 300–500ns (typical for MOSFETs at 50kHz)

**Implementation Options**:

#### Option 1: Firmware Deadtime (Recommended)
Use ESP32-C6 LEDC peripheral with deadtime generation (if supported), or manually generate complementary PWM signals:

```c
// Pseudocode for complementary PWM with deadtime
void mppt_set_duty(float duty) {
    uint32_t period_ticks = (APB_CLK_FREQ / PWM_FREQ_HZ);
    uint32_t on_ticks = (uint32_t)(duty * period_ticks);
    uint32_t deadtime_ticks = (uint32_t)(500e-9 * APB_CLK_FREQ);  // 500ns
    
    // Low-side (Q1): ON from 0 to (on_ticks - deadtime)
    // High-side (Q2): ON from (on_ticks + deadtime) to period
    
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, on_ticks - deadtime_ticks);  // Q1
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
    
    // If using second LEDC channel for Q2:
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1, period_ticks - on_ticks - deadtime_ticks);  // Q2
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1);
}
```

**Note**: Check ESP32-C6 LEDC documentation for deadtime or complementary PWM support. If not available, use two GPIO pins with software-controlled timing.

#### Option 2: External Deadtime Generator
Use a discrete logic IC (e.g., CD4049 inverter + RC delay network) or dedicated deadtime IC (e.g., UCC27201).

#### Option 3: Non-Synchronous (Diode Rectifier)
If using a Schottky diode instead of Q2, tie LIN (pin 12) to GND permanently. Only Q1 is driven.

## VCC Supply (12V for IR2110)

### Power Requirements

**IR2110 Supply Current**:
- Quiescent: 0.5mA (typical)
- Switching: ~10mA average (at 50kHz, driving MOSFET gates)
- Peak: ~100mA (during gate charge/discharge transients)

**Total Current**: ~50–100mA (with margin)

### Supply Options

#### Option 1: Buck Converter from PV Rail (Recommended)
- Use small buck module (e.g., LM2596, TPS54331)
- Input: PV_PROT+ (18–40V)
- Output: 12V @ 200mA
- Efficiency: 85–90%
- Pro: Works across full PV voltage range
- Con: Requires additional buck converter

#### Option 2: Linear Regulator from PV Rail
- Use LDO (e.g., LM7812, AMS1117-12)
- Input: PV_PROT+ (18–40V)
- Output: 12V @ 100mA
- Pro: Simple, low cost
- Con: High power dissipation: (Vin - 12V) × 100mA
  - At Vin = 40V: (40 - 12) × 0.1 = 2.8W dissipation (requires heatsink)
  - At Vin = 18V: (18 - 12) × 0.1 = 0.6W (acceptable without heatsink)

#### Option 3: Separate 12V Supply
- If system has a 12V rail from elsewhere (e.g., from BMS or external supply)
- Directly use that rail for VCC_12V

**Selection**: Use **Option 1** (buck converter) for efficiency and reliability. See Power_Spec_Boost_Converter.md for buck converter recommendations.

### Decoupling

**Capacitors on VCC_12V**:
- **C_VCC**: 10µF ceramic (X7R, 25V, 1206) close to IR2110 VCC pin (pin 10)
- **C_VCC_bulk**: 47µF electrolytic (25V) at buck converter output (if using buck)

## PCB Layout Guidelines

### Trace Routing

1. **Gate Driver to MOSFET Gates**:
   - HO (pin 1) → R_HO_G → Q2 Gate: <50mm, wide trace (0.5mm)
   - LO (pin 4) → R_LO_G → Q1 Gate: <50mm, wide trace (0.5mm)
   - Minimize inductance (short, direct traces)

2. **Bootstrap Loop**:
   - VCC_12V → D1 anode → D1 cathode → VB (pin 7)
   - C_boot between VB (pin 7) and VS (pin 5)
   - Keep loop area small (<5cm²)

3. **Switch Node (VS)**:
   - VS (pin 5) connects to BOOST_SW (switch node)
   - This node has high dV/dt (voltage slew rate)
   - Keep trace short, direct to Q1 drain and Q2 source

4. **Grounds**:
   - COM (pin 2), GND (pin 3), VSS (pin 13) → GND plane
   - Star ground at input capacitor negative terminal

### Component Placement

```
        VCC_12V ──[D1]──┬──[C_boot]──┐
                        │            │
                     ┌──┴────────────┴──┐
         PWM ────────│ IR2110           │
                     │                  │
                     │  HO          VS  ├── BOOST_SW (to Q1 drain, L1, Q2 source)
                     │  LO          VB  │
                     └──┬──────────┬────┘
                        │          │
                   [R_LO_G]    [R_HO_G]
                        │          │
                        ├──────────┼──> To Q1 and Q2 gates (short traces)
                        │          │
                   [R_LO_PD]  [R_HO_PD]
                        │          │
                       GND     BOOST_SW
```

**Guidelines**:
- IR2110 close to MOSFETs Q1 and Q2 (within 30mm)
- Bootstrap diode D1 and capacitor C_boot close to IR2110 VB and VS pins
- Gate resistors close to MOSFET gate pins
- VCC decoupling capacitor close to IR2110 VCC pin

### Ground Plane

- Solid ground plane under IR2110 (Layer 2)
- COM, GND, VSS pins via to ground plane
- Avoid splitting ground plane in this area

## Testing and Verification

### Functional Tests

1. **Static Test (No Load)**:
   - Apply 12V to VCC
   - Apply 24V to PV_PROT+ (input)
   - Send 50% duty cycle PWM from ESP32-C6
   - Measure gate signals on Q1 and Q2 with oscilloscope:
     - Q1 gate: 0–12V square wave at 50kHz
     - Q2 gate: 0–12V square wave at 50kHz (inverted from Q1)
     - Deadtime: 300–500ns visible between Q1 falling edge and Q2 rising edge

2. **Bootstrap Charging Test**:
   - Measure VB - VS with oscilloscope (differential probe or two channels)
   - When Q1 is ON: VB - VS ≈ 12V
   - When Q2 is ON: VB - VS ≈ 11–12V (small voltage drop from gate charge)
   - Confirm C_boot holds voltage between cycles

3. **Gate Drive Strength Test**:
   - Measure gate charge/discharge time (10% to 90%)
   - Should be <100ns rise/fall time with 10Ω gate resistors
   - If too slow: reduce gate resistor value
   - If excessive ringing: increase gate resistor value or add gate-to-source capacitor (100pF)

### Protection Verification

1. **Shoot-Through Protection**:
   - Verify deadtime is present (Q1 and Q2 never ON simultaneously)
   - Measure switch node current (BOOST_SW) during transitions
   - No large current spikes (>2× normal) indicating shoot-through

2. **Under-Voltage Lockout (UVLO)**:
   - IR2110 has internal UVLO: VCC must be >8.5V for operation
   - Test: Ramp VCC from 0V to 12V, verify driver starts outputting at ~9V

## BOM for Gate Driver Circuit

| Ref Des | Part | Value | Package | Qty | Supplier Part # | Notes |
|---------|------|-------|---------|-----|-----------------|-------|
| U2 | IR2110 | - | SOIC-14 or DIP-14 | 1 | Infineon IR2110S (SOIC) | Gate driver IC |
| D1 | UF4007 | - | DO-41 or SMA | 1 | ON Semi UF4007-E3/54 | Bootstrap diode |
| C_boot | Capacitor | 10µF, 25V, X7R | 1206 | 1 | Murata GRM31CR61E106KA12L | Bootstrap cap |
| C_VCC | Capacitor | 10µF, 25V, X7R | 1206 | 1 | Murata GRM31CR61E106KA12L | VCC decoupling |
| R_HO_G | Resistor | 15Ω, 1% | 0603 | 1 | Yageo RC0603FR-0715RL | High-side gate |
| R_LO_G | Resistor | 10Ω, 1% | 0603 | 1 | Yageo RC0603FR-0710RL | Low-side gate |
| R_HO_PD | Resistor | 10kΩ, 1% | 0603 | 1 | Yageo RC0603FR-0710KL | High-side pull-down |
| R_LO_PD | Resistor | 10kΩ, 1% | 0603 | 1 | Yageo RC0603FR-0710KL | Low-side pull-down |
| R_SD_PU | Resistor | 10kΩ, 1% | 0603 | 1 | Yageo RC0603FR-0710KL | Shutdown pull-up |

## Design Checklist

- [ ] IR2110 or IR2104 IC selected and pinout verified
- [ ] Bootstrap circuit (D1 + C_boot) designed and placed close to IC
- [ ] Gate resistors (10–22Ω) selected and placed close to MOSFET gates
- [ ] Pull-down resistors (10kΩ) on all MOSFET gates
- [ ] VCC_12V supply designed (buck converter or LDO)
- [ ] VCC decoupling capacitor (10µF) close to IR2110
- [ ] PWM input from ESP32-C6 routed to HIN and LIN (with deadtime)
- [ ] Gate drive traces short (<50mm) and wide (≥0.5mm)
- [ ] Bootstrap loop area minimized
- [ ] Ground plane solid under IR2110
- [ ] Tested gate signals with oscilloscope (verify voltage levels and deadtime)

## Common Issues and Troubleshooting

### Issue 1: High-Side (Q2) Not Turning On
- **Cause**: Bootstrap capacitor not charging
- **Check**: 
  - Q1 must turn ON at least once to charge C_boot
  - D1 orientation (anode to VCC, cathode to VB)
  - C_boot connection (VB to VS)
- **Solution**: Verify Q1 is switching, check diode polarity

### Issue 2: Excessive Shoot-Through Current
- **Cause**: Insufficient deadtime between Q1 and Q2
- **Check**: Deadtime in firmware or external circuit
- **Solution**: Increase deadtime to 500ns or more

### Issue 3: Slow Switching (Long Rise/Fall Times)
- **Cause**: Gate resistor too high or MOSFET Ciss too large
- **Check**: Oscilloscope on gate signals
- **Solution**: Reduce gate resistor value (e.g., 10Ω → 5Ω) or use MOSFET with lower Qg

### Issue 4: Gate Ringing or Oscillation
- **Cause**: Gate loop inductance + MOSFET Ciss resonance
- **Check**: Oscilloscope on gate signals (look for overshoot/undershoot)
- **Solution**: 
  - Increase gate resistor (10Ω → 22Ω)
  - Add gate-to-source capacitor (100–470pF) to damp resonance
  - Shorten gate traces

## References

- IR2110 Datasheet (Infineon, IR2110.pdf)
- Application Note AN-978: "HV Floating MOS-Gate Driver IC" (Infineon)
- Application Note AN-1182: "Bootstrap Component Selection for Control IC's" (Infineon)
- ESP32-C6 LEDC Peripheral Documentation (ESP-IDF)

## Revision History

- **v1.0** (2025-12-15): Initial gate driver specification for IR2110/IR2104 boost converter
