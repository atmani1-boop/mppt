# Power Specifications - Boost Converter Design

## Overview

This document provides detailed power stage design calculations and component selection for the 150W synchronous boost converter (18–40V PV input → 48–129V output).

## Design Requirements

| Parameter | Value | Notes |
|-----------|-------|-------|
| Input Voltage (Vin) | 18–40V | PV panel voltage range |
| Output Voltage (Vout) | 48–129V | Adjustable or fixed via feedback |
| Maximum Power | 150W | Rated at 40V input, 10A PV current |
| Efficiency Target | >94% | At rated power, 36V input, 96V output |
| PWM Frequency (fsw) | 50kHz | Fixed frequency, MCU controlled |
| Topology | Synchronous Boost | Continuous Conduction Mode (CCM) |
| Control Method | MPPT + Voltage Regulation | ESP32-C6 firmware controlled |

## Boost Converter Theory

### Basic Equations

**Boost Converter Voltage Relationship** (ideal, no losses):
```
Vout = Vin / (1 - D)
```
where D = duty cycle (0 to 1)

**Duty Cycle Calculation**:
```
D = 1 - (Vin / Vout)
```

**Inductor Current** (CCM):
```
IL_avg = Pout / (Vin × η)
IL_peak = IL_avg + (ΔIL / 2)
IL_valley = IL_avg - (ΔIL / 2)
```

**Current Ripple**:
```
ΔIL = (Vin × D) / (L × fsw)
```

## Operating Range Calculations

### Minimum Duty Cycle (Vin = 40V, Vout = 48V)
```
D_min = 1 - (40V / 48V) = 1 - 0.833 = 0.167 (16.7%)
```

### Maximum Duty Cycle (Vin = 18V, Vout = 129V)
```
D_max = 1 - (18V / 129V) = 1 - 0.140 = 0.860 (86.0%)
```

### Typical Operating Point (Vin = 36V, Vout = 96V, 150W)
```
D_typ = 1 - (36V / 96V) = 1 - 0.375 = 0.625 (62.5%)
Iin = 150W / 36V = 4.17A
Iout = 150W / 96V = 1.56A
IL_avg = Iin / (1 - D) = 4.17A / 0.375 = 11.1A
```

**Note**: In synchronous boost, inductor current equals input current when Q1 is ON, and flows through Q2 when Q1 is OFF.

## Inductor Selection

### Inductor Value Calculation

**Design for 20% current ripple** (ΔIL = 0.2 × IL_avg):

At worst case (maximum ripple occurs at different Vin/Vout):
```
For D = 0.5 (50%), Vin = 30V, fsw = 50kHz:
ΔIL = (Vin × D) / (L × fsw)
L = (Vin × D) / (ΔIL × fsw)

Target ΔIL = 0.2 × 10A = 2A (assuming 10A avg current)
L = (30V × 0.5) / (2A × 50kHz) = 15V / 100kA = 150µH
```

**Selected Range**: 47–100µH

- Lower inductance (47µH): Higher ripple, smaller size, lower DCR
- Higher inductance (100µH): Lower ripple, larger size, potentially higher DCR

**Recommendation**: 47µH for compact design, 100µH for lower EMI and ripple

### Inductor Specifications

| Parameter | Minimum | Recommended | Units | Notes |
|-----------|---------|-------------|-------|-------|
| Inductance | 47 | 100 | µH | ±20% tolerance |
| Saturation Current (Isat) | 12 | 15 | A | At 25°C, 20% drop |
| RMS Current (Irms) | 10 | 12 | A | Thermal rating |
| DC Resistance (DCR) | <15 | <10 | mΩ | Lower is better |
| Core Type | Ferrite or Iron Powder | Shielded preferred | - | EMI reduction |

### Recommended Parts

#### Option 1: Würth Elektronik 744355247
- **Inductance**: 47µH ±20%
- **Isat**: 17A (at 20% drop)
- **Irms**: 13A
- **DCR**: 4.8mΩ
- **Size**: 12.5×12.5×8mm (SMD shielded)
- **Supplier**: Mouser #710-744355247

#### Option 2: Bourns SRP1265A-100M
- **Inductance**: 100µH ±20%
- **Isat**: 12.5A (at 30% drop)
- **Irms**: 10.7A
- **DCR**: 9.8mΩ
- **Size**: 12.7×12.7×6.5mm (SMD shielded)
- **Supplier**: Mouser #652-SRP1265A-100M

#### Option 3: Coilcraft SER1360-472ML (High Efficiency)
- **Inductance**: 47µH ±20%
- **Isat**: 16.5A
- **Irms**: 15A
- **DCR**: 3.5mΩ
- **Size**: 13.46×13.46×6mm
- **Supplier**: Digikey #SER1360-472MLCT-ND

### Inductor Power Loss Calculation

**DC Copper Loss**:
```
P_DCR = I²_rms × DCR
For IL_rms ≈ 10A, DCR = 10mΩ:
P_DCR = 10² × 0.010 = 1W
```

**Core Loss** (estimate, frequency and flux dependent):
```
P_core ≈ 0.2W (at 50kHz, typical for ferrite)
```

**Total Inductor Loss**:
```
P_inductor ≈ 1.2W
```

## MOSFET Selection

### Low-Side MOSFET (Q1)

Q1 conducts during the ON-time (duty cycle D). It must handle the full inductor current.

**Requirements**:
- **Vds**: >60V (40V input + margin)
- **Rds(on)**: <20mΩ @ 10V Vgs (lower is better for efficiency)
- **Id**: >20A continuous (derated for temperature)
- **Qg**: Low gate charge for switching efficiency
- **Package**: TO-220, D2PAK, or TO-263 with thermal pad

**Recommended Parts**:

#### IRFB4115PBF (Infineon/IR)
- **Vds**: 60V
- **Rds(on)**: 3.7mΩ @ 10V Vgs (typ)
- **Id**: 104A @ 25°C, 66A @ 100°C
- **Qg**: 140nC
- **Package**: TO-220AB
- **Supplier**: Mouser #942-IRFB4115PBF
- **Cost**: ~$2.50

#### PSMN1R0-30YLD (Nexperia)
- **Vds**: 30V (only suitable if Vin max = 24V, not recommended here)
- Alternative for lower voltage designs

#### IPB80N06S2L-07 (Infineon)
- **Vds**: 60V
- **Rds(on)**: 7mΩ @ 10V Vgs
- **Id**: 80A @ 25°C
- **Qg**: 46nC (lower than IRFB4115)
- **Package**: TO-263
- **Supplier**: Mouser #726-IPB80N06S2L-07

**Selection**: **IRFB4115PBF** for lowest Rds(on), or **IPB80N06S2L-07** for lower switching losses (lower Qg).

### High-Side MOSFET or Diode (Q2)

In synchronous boost, Q2 replaces the freewheeling diode. It conducts during OFF-time (1 - D).

**Requirements**:
- **Vds**: >150V (129V output + switching overshoot)
- **Rds(on)**: <100mΩ @ 10V Vgs (efficiency vs. cost trade-off)
- **Id**: >5A continuous (output current 1.5–3A)
- **Package**: TO-220, D2PAK, or TO-263

**Synchronous Rectifier (Recommended)**:

#### IPP60R125CP (Infineon CoolMOS)
- **Vds**: 600V (huge margin, robust)
- **Rds(on)**: 125mΩ @ 10V Vgs
- **Id**: 24A @ 25°C
- **Qg**: 48nC
- **Package**: TO-220
- **Supplier**: Mouser #726-IPP60R125CP
- **Cost**: ~$1.80

#### IPP80N06S2L-H4 (Infineon)
- **Vds**: 60V (only suitable for Vout <50V, not recommended for 129V)

**Diode Rectifier (Alternative, Non-Synchronous)**:

If synchronous control is too complex, use a Schottky diode:

#### MBR40250G (ON Semi)
- **Vrrm**: 250V
- **If**: 40A average
- **Vf**: 0.85V @ 20A
- **Package**: TO-220AC
- **Supplier**: Mouser #863-MBR40250G
- **Cost**: ~$2.00

**Selection**: **IPP60R125CP** for synchronous boost (higher efficiency), or **MBR40250G** for simpler non-synchronous design.

### MOSFET Power Loss Calculation

#### Q1 (Low-Side) Losses

**Conduction Loss** (ON-time):
```
P_cond_Q1 = I²_rms × Rds(on) × D
For I_rms ≈ 10A, Rds(on) = 3.7mΩ, D = 0.625:
P_cond_Q1 = 10² × 0.0037 × 0.625 = 0.23W
```

**Switching Loss** (turn-on and turn-off):
```
P_sw_Q1 = 0.5 × Vds × Id × (t_rise + t_fall) × fsw
Assuming t_rise + t_fall ≈ 100ns, Vds = 40V, Id = 10A, fsw = 50kHz:
P_sw_Q1 = 0.5 × 40 × 10 × 100e-9 × 50e3 = 1W
```

**Total Q1 Loss**:
```
P_Q1 ≈ 0.23 + 1 = 1.23W
```

#### Q2 (High-Side) Losses (Synchronous)

**Conduction Loss** (OFF-time):
```
P_cond_Q2 = I²_rms × Rds(on) × (1 - D)
For I_rms ≈ 10A, Rds(on) = 125mΩ, (1-D) = 0.375:
P_cond_Q2 = 10² × 0.125 × 0.375 = 4.7W
```

**Switching Loss** (estimated lower than Q1 due to ZVS at turn-on in some conditions):
```
P_sw_Q2 ≈ 0.5W (estimated)
```

**Total Q2 Loss**:
```
P_Q2 ≈ 4.7 + 0.5 = 5.2W
```

**Note**: Q2 has higher conduction loss due to higher Rds(on). This is acceptable as output current is lower.

#### Q2 (Diode Rectifier, Non-Synchronous)

**Forward Loss**:
```
P_diode = Vf × Iout × (1 - D)
For Vf = 0.85V, Iout ≈ 1.5A, (1-D) = 0.375:
P_diode = 0.85 × 1.5 × 0.375 = 0.48W
```

**Reverse Recovery Loss** (Schottky has minimal reverse recovery):
```
P_rr ≈ 0.1W
```

**Total Diode Loss**:
```
P_diode_total ≈ 0.6W
```

**Conclusion**: Synchronous rectifier (Q2 MOSFET) has higher loss than diode in this design. However, at higher output currents (>2A), synchronous is more efficient. For 150W at Vout=96V (Iout=1.56A), diode rectifier is simpler and acceptable.

**Recommendation**: Use **MBR40250G Schottky diode** for simplicity, or **IPP60R125CP** if optimizing for higher output current applications.

## Capacitor Selection

### Input Capacitor (C_in)

**Function**: Filter PV input ripple, provide low-impedance source for inductor current.

**RMS Current**:
```
I_Cin_rms ≈ √(D × (1-D)) × IL_avg
For D = 0.625, IL_avg = 10A:
I_Cin_rms = √(0.625 × 0.375) × 10 = √0.234 × 10 = 4.84A
```

**Specifications**:
- **Capacitance**: 470µF minimum (low ESR)
- **Voltage Rating**: 63V (40V input + 50% margin)
- **ESR**: <50mΩ
- **Ripple Current**: >5A RMS @ 50kHz
- **Type**: Polymer or low-ESR electrolytic

**Recommended Parts**:

#### Primary: 470µF/63V Polymer Capacitor
- **Part**: Panasonic EEF-UE0J471R (470µF, 63V, polymer)
- **ESR**: 40mΩ @ 100kHz
- **Ripple Current**: 3.8A @ 100kHz
- **Supplier**: Mouser #667-EEF-UE0J471R
- **Quantity**: 2× in parallel (9.4A ripple rating, 20mΩ ESR)

#### Bypass: 10µF Ceramic (high frequency)
- **Part**: 10µF, 50V, X7R ceramic (1206 or larger)
- **Supplier**: Murata GRM31CR61H106KA12L (10µF, 50V, 1206)
- **Quantity**: 2× in parallel
- **Purpose**: Filter high-frequency switching noise

**Total Input Capacitance**: 940µF + 20µF ceramic

### Output Capacitor (C_out)

**Function**: Filter output voltage ripple, provide stable voltage to load.

**RMS Current**:
```
I_Cout_rms ≈ Iout × √(D / (1-D))
For D = 0.625, Iout = 1.56A:
I_Cout_rms = 1.56 × √(0.625 / 0.375) = 1.56 × 1.29 = 2.01A
```

**Specifications**:
- **Capacitance**: 440µF minimum (2× 220µF)
- **Voltage Rating**: 160V or 200V (129V output + margin)
- **ESR**: <100mΩ
- **Ripple Current**: >2.5A RMS @ 50kHz
- **Type**: Electrolytic or film (ceramic not practical at high voltage and capacitance)

**Recommended Parts**:

#### Primary: 220µF/160V Electrolytic Capacitor
- **Part**: Nichicon UPW1C221MPD (220µF, 160V, 105°C)
- **ESR**: 240mΩ @ 100kHz
- **Ripple Current**: 1.58A @ 100kHz
- **Size**: 12.5×25mm
- **Supplier**: Mouser #647-UPW1C221MPD
- **Quantity**: 2× in parallel (3.16A ripple rating, 120mΩ ESR)

Alternative (higher ripple current):
- **Panasonic EEU-HD1C221** (220µF, 160V, 2.33A ripple)
- **Supplier**: Mouser #667-EEU-HD1C221

#### Bypass: 4.7µF/200V Ceramic (high frequency)
- **Part**: 4.7µF, 200V, X7R ceramic (1210 or 1812)
- **Supplier**: TDK C5750X7R2A475M230KA (4.7µF, 100V, 1812) - or use 2× in series for 200V
- **Quantity**: 2× (or 4× if series-parallel configuration)
- **Purpose**: Filter high-frequency output ripple

**Total Output Capacitance**: 440µF + ~5µF ceramic

### Bootstrap Capacitor (for IR2110/IR2104)

**Function**: Provide gate charge for high-side MOSFET Q2.

**Specifications**:
- **Capacitance**: 10µF (ceramic, low ESR)
- **Voltage Rating**: 25V minimum (bootstrap voltage ≈ 12V)
- **Type**: X7R ceramic

**Recommended Part**:
- **Part**: 10µF, 25V, X7R, 1206
- **Supplier**: Murata GRM31CR61E106KA12L

## Gate Driver Circuit

See dedicated document: `MOSFET_Driver_Boost.md`

**Summary**:
- IC: IR2110 or IR2104
- Bootstrap: UF4007 diode + 10µF ceramic capacitor
- Gate Resistors: 10–22Ω series, 10kΩ pull-down

## Efficiency Calculation

### Power Loss Summary (at 150W, Vin=36V, Vout=96V, D=0.625)

| Component | Loss (W) | Notes |
|-----------|----------|-------|
| Inductor (L1) | 1.2 | DCR + core loss |
| Q1 (low-side MOSFET) | 1.2 | Conduction + switching |
| Q2 (Schottky diode) | 0.6 | Forward drop |
| Gate Driver | 0.2 | IC + gate charge |
| Input Cap ESR | 0.1 | I²_rms × ESR |
| Output Cap ESR | 0.05 | I²_rms × ESR |
| **Total Loss** | **3.35W** | |

**Efficiency**:
```
η = Pout / (Pout + Ploss) = 150 / (150 + 3.35) = 97.8%
```

**Result**: **97.8% efficiency** at rated power (exceeds 94% target).

### Efficiency vs. Operating Point

| Vin (V) | Vout (V) | Pin (W) | D | Estimated η | Notes |
|---------|----------|---------|---|-------------|-------|
| 40 | 48 | 150 | 0.17 | 96% | Low duty, high current |
| 36 | 96 | 150 | 0.63 | 97.8% | Typical MPPT operating point |
| 18 | 129 | 150 | 0.86 | 93% | High duty, stress on components |
| 24 | 96 | 100 | 0.75 | 95% | Reduced power |

**Note**: Efficiency degrades at high duty cycles (D > 0.8) due to increased RMS currents and switching losses.

## Thermal Management

### MOSFET Q1 (Hottest Component)

**Power Dissipation**: 1.2W

**Thermal Resistance**:
- Rθ(JC): 0.5°C/W (junction to case, TO-220 package)
- Rθ(CS): 0.5°C/W (case to heatsink, thermal grease)
- Rθ(SA): 10°C/W (heatsink to ambient, small heatsink)

**Junction Temperature**:
```
T_j = T_a + P × (Rθ(JC) + Rθ(CS) + Rθ(SA))
For T_a = 50°C (ambient in enclosure), P = 1.2W:
T_j = 50 + 1.2 × (0.5 + 0.5 + 10) = 50 + 13.2 = 63.2°C
```

**Result**: Q1 junction temperature is **63°C**, well below 150°C max rating. No heatsink required if PCB has adequate copper area (≥10cm²).

### Inductor L1

**Power Dissipation**: 1.2W

**Temperature Rise**:
```
ΔT ≈ 40°C for 1W dissipation in typical shielded inductor
```

**Surface Temperature**:
```
T_surf ≈ 50°C + 40°C = 90°C
```

**Result**: Inductor will be warm (90°C) but within ratings (typically 125°C max).

### PCB Copper Area Recommendations

- **Q1 Drain/Source**: ≥10cm² copper area on top layer, thermal vias to bottom layer
- **Q2 or Diode**: ≥5cm² copper area
- **Inductor L1**: ≥10cm² copper area on bottom layer under inductor
- **Shunt Resistors**: ≥2cm² each, wide traces (≥2mm)

## Layout Guidelines

### Critical Paths (Minimize Loop Area)

1. **Main Power Loop**: Q1 drain → L1 → Q2 (or diode) → C_out → Q1 source
   - Keep this loop as small as possible (high di/dt current)
   
2. **Bootstrap Loop**: C_bootstrap → Q2 gate → Q2 source → D_bootstrap → VCC_12V
   - Keep bootstrap cap close to IR2110 HB and HO pins

3. **Input Loop**: C_in → Q1 drain → Q1 source → C_in
   - Input caps close to Q1 drain and source

### Trace Widths

| Net | Current | Width | Layers | Notes |
|-----|---------|-------|--------|-------|
| PV+, PV- | 10A | ≥2mm | Top + bottom | Via stitching |
| BOOST_SW | 10A peak | ≥2mm | Top only | Minimize area! |
| OUT+, OUT- | 3A | ≥1.5mm | Top + bottom | HV clearance |
| VCC_12V | 100mA | 0.5mm | Top or inner | Gate driver supply |
| VCC_3V3 | 500mA | 0.8mm | Power plane | Logic supply |

### Ground Strategy

- **Power Ground (PGND)**: High current return (PV-, Q1 source, C_in-, C_out-)
- **Logic Ground (GND)**: Low current return (ESP32-C6, sensors, gate driver ground)
- **Connection**: Star ground at C_in- (input capacitor negative terminal)

### Component Placement

1. **C_in** → Very close to Q1 drain and source (minimize input loop)
2. **L1** → Adjacent to Q1 drain (switch node)
3. **Q2/Diode** → Adjacent to L1 and C_out
4. **C_out** → Close to Q2/Diode output
5. **IR2110** → Close to Q1 and Q2 gates (short gate traces <50mm)
6. **Bootstrap components** → Close to IR2110 HB, HO, VS pins

## Component Checklist

- [ ] Inductor: 47–100µH, Isat >12A, DCR <10mΩ (e.g., Würth 744355247)
- [ ] Q1: 60V, <20mΩ Rds(on) (e.g., IRFB4115PBF)
- [ ] Q2: 150V MOSFET (IPP60R125CP) or 250V Schottky diode (MBR40250G)
- [ ] C_in: 2× 470µF/63V polymer + 2× 10µF/50V ceramic
- [ ] C_out: 2× 220µF/160V electrolytic + ceramic bypass
- [ ] Gate driver: IR2110 or IR2104 (see MOSFET_Driver_Boost.md)
- [ ] Bootstrap: UF4007 diode + 10µF/25V ceramic
- [ ] Gate resistors: 10–22Ω series, 10kΩ pull-down
- [ ] Thermal pads: Copper areas ≥10cm² for Q1, L1

## Testing and Validation

### Bench Testing Procedure

1. **No-Load Test**: Apply 24V input, verify output voltage regulation
2. **Load Sweep**: 10% to 100% load, measure efficiency at each point
3. **Input Voltage Sweep**: 18V to 40V, verify MPPT tracking
4. **Thermal Test**: Run at 150W for 30 minutes, measure component temperatures
5. **Ripple Measurement**: Oscilloscope on input and output (AC coupling, 20MHz BW)
   - Input ripple: <1V pk-pk
   - Output ripple: <2V pk-pk
6. **EMI Pre-scan**: Near-field probe around inductor and switch node

### Acceptance Criteria

- [ ] Efficiency >94% at Vin=36V, Vout=96V, Pout=150W
- [ ] Output voltage regulation ±2% under load variations
- [ ] MOSFET temperatures <85°C at 150W continuous, 50°C ambient
- [ ] Input ripple <1V pk-pk, output ripple <2V pk-pk
- [ ] No audible noise (inductor saturation or magnetostriction)

## Design Notes

1. **Synchronous vs. Non-Synchronous**: For this power level (150W) and output current (1.5–3A), non-synchronous (Schottky diode) is simpler and nearly as efficient. Use synchronous Q2 if scaling to higher output currents (>5A).

2. **CCM vs. DCM**: Design assumes Continuous Conduction Mode (CCM) for predictable operation. At light loads (<20W), converter may enter Discontinuous Conduction Mode (DCM), which is acceptable but requires different control loop tuning.

3. **Duty Cycle Limitation**: ESP32-C6 firmware should limit duty cycle to 0.05–0.90 range to avoid extreme operating conditions and ensure safe startup.

4. **Soft-Start**: Implement firmware soft-start (ramp duty cycle 0→target over 100ms) to prevent input current surge and output voltage overshoot.

5. **MPPT Integration**: The boost converter is controlled by MPPT algorithm (P&O in `mppt_pno.c`). Ensure MPPT updates are synchronized with voltage/current sampling from INA226 sensors.

## References

- Fundamentals of Power Electronics (Erickson & Maksimovic), Chapter 6: Boost Converter
- TI Application Note SLVA372C: "Basic Calculation of a Boost Converter's Power Stage"
- IR2110 Datasheet and Application Note AN-978: "HV Floating MOS-Gate Driver IC"
- Würth Elektronik "Inductor Selection Guide for DC-DC Converters"

## Revision History

- **v1.0** (2025-12-15): Initial power stage design for 150W boost converter
