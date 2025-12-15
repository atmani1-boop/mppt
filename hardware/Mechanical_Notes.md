# Mechanical Notes - MPPT 150W Boost Converter

## PCB Specifications

### Dimensions
- **Size**: 100×80mm (indicative, adjust based on component placement)
- **Thickness**: 1.6mm (standard FR4)
- **Layers**: 4-layer stackup (recommended for proper grounding and power distribution)

### 4-Layer Stackup

```
┌─────────────────────────────────────┐
│ Layer 1 (Top): Signal + Components  │  ← SMD components, high voltage traces
├─────────────────────────────────────┤
│ Layer 2 (Inner): Ground Plane (GND) │  ← Solid copper pour, minimal interruptions
├─────────────────────────────────────┤
│ Layer 3 (Inner): Power Planes       │  ← VCC_3V3, VCC_5V, PV+ regions
├─────────────────────────────────────┤
│ Layer 4 (Bottom): Signal + Thermal  │  ← Additional routing, thermal pads
└─────────────────────────────────────┘
```

**Copper Weight**:
- **Top/Bottom (L1/L4)**: 2oz (70µm) for high current traces
- **Inner (L2/L3)**: 1oz (35µm) standard

**Surface Finish**: ENIG (Electroless Nickel Immersion Gold) for better solderability and shelf life

---

## Mounting Holes

### Specifications
- **Quantity**: 4 (one in each corner)
- **Size**: 3.2mm diameter (for M3 screws)
- **Clearance**: 5mm radius keep-out zone (no copper or components)
- **Reinforcement**: Plated through-holes with annular ring

### Mounting Hole Locations

```
┌─────────────────────────────────────┐
│  ●                             ●    │  ← Mounting holes (M3)
│                                     │
│                                     │
│         [PCB Components]            │
│                                     │
│                                     │
│  ●                             ●    │
└─────────────────────────────────────┘
   5mm from edge, all corners
```

**Standoff Height**: 10mm (brass or nylon M3 standoffs)

---

## Thermal Management

### High-Power Components

#### Q1: Low-Side MOSFET (1.2W dissipation)
- **Copper Area**: ≥10cm² on top layer (drain/source pads)
- **Thermal Vias**: 5×5 array of 0.3mm vias (0.6mm pitch) under TO-220 thermal pad
- **Heatsink**: Optional small TO-220 heatsink if ambient >40°C
- **Thermal Grease**: Apply between Q1 case and PCB copper

#### Q2: High-Side MOSFET or Diode (5W dissipation if MOSFET, 0.6W if diode)
- **Copper Area**: ≥5cm² on top layer
- **Thermal Vias**: 4×4 array under component

#### L1: Inductor (1.2W dissipation)
- **Copper Area**: ≥10cm² on bottom layer (under inductor)
- **Ventilation**: Keep clear of components on bottom side for air circulation

#### Q3: Reverse Polarity MOSFET (2W dissipation)
- **Copper Area**: ≥5cm² on top layer
- **Thermal Vias**: 3×3 array

### Thermal Via Specifications
- **Diameter**: 0.3mm (finished)
- **Pitch**: 0.6–0.8mm
- **Plating**: Through-hole plated
- **Fill**: Tented (via mask on top/bottom) or plugged with epoxy for better thermal performance

### Thermal Simulation (Optional)
- Use thermal simulation tools (e.g., ANSYS Icepak, Mentor FloTHERM) to verify temperatures <85°C
- Ambient temperature assumption: 50°C (enclosed, no forced air cooling)

---

## High Voltage Clearances

### IEC 60664-1 Guidelines

For **129V DC, Pollution Degree 2** (normal indoor environment):
- **Creepage Distance**: 1.5mm minimum, **3mm recommended** for safety margin
- **Clearance (air gap)**: 1.0mm minimum, **3mm recommended**

### Trace Spacing

| Net Pair | Minimum Clearance | Recommended | Notes |
|----------|-------------------|-------------|-------|
| OUT+ to GND/Logic | 3mm | 4mm | High voltage (48–129V) |
| OUT+ to OUT− | 1.5mm | 2mm | Same potential domain |
| PV+ to GND/Logic | 1.5mm | 2mm | Lower voltage (18–40V) |
| BOOST_SW to GND | 3mm | 4mm | High dV/dt, potential EMI |
| Logic traces | 0.15mm | 0.2mm | Standard spacing |

### Solder Mask Considerations
- **Solder Mask Thickness**: 25µm typical (adds insulation)
- **Solder Mask Dam**: 0.1mm between adjacent pads
- **Keep solder mask intact** on HV traces (do not expose copper except at pads/vias)

---

## Trace Widths (Current Handling)

Based on 2oz copper (70µm), 10°C temperature rise:

| Net | Current (A) | Width (mm) | Layers | Notes |
|-----|-------------|------------|--------|-------|
| PV+, PV_PROT+ | 10 | ≥2.0 | Top + Bottom (via stitching) | High current |
| BOOST_SW | 10 (peak) | ≥2.0 | Top only | Minimize area for EMI |
| OUT+, OUT− | 3 | ≥1.5 | Top + Bottom | HV clearance |
| VCC_12V | 0.1 | 0.5 | Top or inner | Gate driver supply |
| VCC_5V | 0.5 | 0.8 | Inner plane or top | Peripherals |
| VCC_3V3 | 0.5 | 0.8 | Inner plane or top | Logic supply |
| PWM, I²C, GPIO | <0.01 | 0.2–0.3 | Top | Signal traces |

**Via Stitching**: Connect top and bottom copper pours with vias every 5–10mm on high current nets

---

## EMI Considerations

### Minimize Switch Node Area (BOOST_SW)
- **Layout**: Star connection at Q1 drain, Q2 source/anode, L1 input
- **Trace Length**: <30mm total
- **Trace Width**: 2mm (adequate for current, but keep short)
- **Ground Plane**: Solid ground on Layer 2, avoid cutouts under switch node

### Input/Output Filtering
- **Input Caps (C_in)**: Place close to Q1 source and PV+ input (within 10mm)
- **Output Caps (C_out)**: Place close to L1 output and Q2 output (within 10mm)
- **Decoupling**: 100nF ceramic caps within 5mm of each IC VCC pin

### Shielding (Optional)
- For enclosed designs, use metal enclosure as Faraday cage
- Connect enclosure to GND at single point (avoid ground loops)

---

## Ground Strategy

### Single-Point Star Ground
- **Star Point**: Input capacitor negative terminal (C_in−)
- **Power Ground (PGND)**: High current paths (PV, boost, output)
- **Logic Ground (GND)**: Low current paths (ESP32-C6, sensors, gate driver COM)
- **Connection**: PGND and GND connected at star point only

### Ground Plane (Layer 2)
- **Coverage**: ≥90% of PCB area
- **Cutouts**: Minimize cutouts; if needed, bridge with 0Ω resistors
- **Stitching Vias**: Place vias every 10–15mm to connect top/bottom grounds to Layer 2

---

## Component Placement

### Top Side (Layer 1)
```
┌─────────────────────────────────────────────────────┐
│  [J1 PV]  [Q3] [C_in] [Q1] [L1] [Q2] [C_out] [J2]  │
│                                                     │
│  [U3 INA226#1]     [U1 ESP32-C6]    [U4 INA226#2]  │
│                                                     │
│  [U2 IR2110]  [LED1]  [Protection ICs]  [J3 USB-C] │
│                                                     │
│  [J4 I2C] [J6 LED] [J7 BMS]                [J5 ANT]│
└─────────────────────────────────────────────────────┘
```

**Guidelines**:
- **Power Stage** (left side): PV input → boost converter → output
- **MCU** (center): ESP32-C6 with antenna on edge
- **Sensors** (left/right): INA226 near shunts
- **Connectors** (edges): Easy access for wiring
- **LED Status** (front): Visible when mounted

### Bottom Side (Layer 4)
- **Thermal Pads**: Copper pours under MOSFETs and inductor
- **Additional Passives**: If space limited on top
- **Connector Pins**: Through-hole connectors protrude through bottom

---

## Silkscreen

### Top Silkscreen
- **Component References**: U1, U2, Q1, Q2, L1, etc.
- **Polarity Markings**: + and − symbols on connectors, capacitors
- **Voltage Warnings**: "⚠ HIGH VOLTAGE 129V" near OUT+ traces
- **Net Names**: Label critical nets (PV+, OUT+, BOOST_SW, GND) for debugging
- **Company Logo**: Optional branding

### Bottom Silkscreen
- **PCB Version**: "MPPT v1.0", revision date
- **Serial Number**: Space for hand-written or laser-etched serial number
- **Mounting Instructions**: M3 screw positions

---

## Conformal Coating (Optional)

For outdoor or humid environments:

- **Type**: Acrylic or polyurethane conformal coating
- **Thickness**: 25–75µm
- **Coverage**: All components except:
  - Connectors (J1–J7)
  - USB-C (J3)
  - Antenna connector (J5)
  - Programming/debug headers
- **Application**: Spray or brush-on (after assembly and testing)

**Benefits**: Moisture protection, dust resistance, improved creepage distance

---

## Enclosure (If Applicable)

### Dimensions
- **Internal**: 110×90×40mm (to fit 100×80mm PCB with clearance)
- **Material**: ABS plastic or aluminum (for EMI shielding)
- **Mounting**: DIN rail clips or wall-mount brackets

### Ventilation
- **Vents**: 10mm diameter holes on sides (if passive cooling)
- **Fan**: Optional 40mm fan for active cooling (if ambient >50°C)

### Connector Cutouts
- **J1 (PV)**: 10×15mm slot
- **J2 (HV Out)**: 15×15mm slot
- **J3 (USB-C)**: 10×5mm slot
- **J5 (Antenna)**: 8mm diameter hole for U.FL cable passthrough

### IP Rating (Optional)
- **IP20**: Indoor use, basic protection
- **IP65**: Outdoor use, dust-tight and water-resistant (requires sealed connectors and conformal coating)

---

## PCB Fabrication Notes

### Design Files
- **Gerber Files**: RS-274X format (standard)
- **Drill Files**: Excellon format
- **BOM**: CSV or Excel
- **Assembly Drawings**: PDF with component placement and orientation

### Panelization
- **Single PCB**: 100×80mm
- **Panel**: 2×2 or 3×2 if ordering multiple units
- **V-Scoring or Tab Routing**: For easy depanelization

### Quality Checks
- **DRC (Design Rule Check)**: Verify trace widths, clearances, via sizes
- **ERC (Electrical Rule Check)**: Verify net connectivity, power flags
- **Visual Inspection**: Check component footprints, pad sizes

---

## Testing Access

### Test Points
- **TP1**: PV+ (test PV voltage)
- **TP2**: PV_PROT+ (post reverse-polarity protection)
- **TP3**: BOOST_SW (switch node, for oscilloscope)
- **TP4**: OUT+ (output voltage)
- **TP5**: VCC_3V3 (logic supply)
- **TP6**: GND (common ground)

**Test Point Type**: 1.0mm diameter pad with exposed copper (no solder mask), or through-hole pin

### Debug Headers
- **J4 (I²C)**: 4-pin header for logic analyzer
- **Optional**: JTAG/SWD header if external debugging needed (ESP32-C6 has internal USB-JTAG)

---

## Mechanical Checklist

- [ ] PCB dimensions finalized (100×80mm or as required)
- [ ] 4 mounting holes placed (M3, 5mm from edges)
- [ ] High voltage clearances verified (≥3mm for 129V)
- [ ] Thermal management designed (copper pours, thermal vias)
- [ ] Trace widths adequate for current (2mm for 10A, 1.5mm for 3A)
- [ ] Ground plane continuous on Layer 2 (≥90% coverage)
- [ ] Component placement optimized for assembly and airflow
- [ ] Silkscreen labels clear and readable (1mm min height)
- [ ] Conformal coating plan defined (if outdoor use)
- [ ] Enclosure dimensions and cutouts specified (if applicable)
- [ ] Test points and debug headers accessible
- [ ] DRC/ERC checks passed with no errors

---

## References

- IEC 60664-1: Insulation coordination for low-voltage equipment
- IPC-2221: Generic Standard on Printed Board Design
- IPC-7351: Generic Requirements for Surface Mount Design and Land Pattern Standard
- PCB Thermal Design Guidelines (Texas Instruments, Infineon)

## Revision History

- **v1.0** (2025-12-15): Initial mechanical and PCB layout notes for MPPT 150W boost converter
