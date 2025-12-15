# Hardware Questions for Designer - Checklist

## Overview

This checklist contains critical questions and clarifications that the schematic designer should address before finalizing the hardware design for the MPPT 150W boost converter.

---

## Power Stage Questions

### 1. BMS Voltage Boost Mechanism
**Question**: How is the 1S LiFePO4 cell voltage (3.2V) boosted to 48–129V output?

**Options**:
- [ ] **Integrated in BMS module**: BMS includes internal DC-DC boost converter
- [ ] **External DC-DC module**: Separate boost module connected between BMS and output
- [ ] **This MPPT boost converter outputs to BMS**: Boost converter charges BMS, BMS provides separate boosted output

**Action Required**:
- Confirm BMS model and voltage boost solution
- If external boost needed, specify part number and integration point
- Update schematic to show correct power flow

---

### 2. Output Voltage Setting
**Question**: Should the output voltage (48–129V) be fixed or adjustable?

**Options**:
- [ ] **Fixed voltage** (specify: ___ V, e.g., 48V, 96V, or 129V)
- [ ] **Adjustable via potentiometer** (add trimmer resistor in feedback divider)
- [ ] **Adjustable via firmware** (ESP32-C6 sets voltage via DAC or PWM to feedback circuit)

**Action Required**:
- Decide on voltage control method
- If adjustable, add feedback circuit with DAC/potentiometer
- Update firmware to implement voltage regulation loop

---

### 3. Synchronous vs. Non-Synchronous Rectifier
**Question**: Use synchronous MOSFET (Q2) or Schottky diode for high-side rectification?

**Options**:
- [ ] **Synchronous MOSFET** (Q2 = IPP60R125CP): Higher efficiency at high currents, requires complementary PWM control
- [ ] **Schottky Diode** (MBR40250G): Simpler design, adequate efficiency for 1.5–3A output current, no complementary PWM needed

**Action Required**:
- Select based on efficiency vs. complexity trade-off
- If synchronous: implement deadtime in firmware (300–500ns)
- If diode: tie IR2110 LIN (pin 12) to GND permanently

**Recommendation**: Use **Schottky diode** for simplicity (this design's output current is relatively low, 1.5–3A).

---

## Connectivity Questions

### 4. Thread Antenna: PCB Trace or External?
**Question**: How should the 2.4GHz antenna for Thread/WiFi/BLE be implemented?

**Options**:
- [ ] **PCB Trace Antenna** (meandered or inverted-F design on board edge)
- [ ] **External Chip Antenna** (U.FL connector + ceramic antenna module)

**Pros/Cons**:
| Option | Pros | Cons |
|--------|------|------|
| PCB Trace | No extra parts, lower cost | Requires RF expertise, tuning needed, performance depends on PCB layout |
| External | Better range, easier tuning, flexible placement | Extra cost (~$3–4), requires U.FL connector and cable |

**Action Required**:
- If PCB trace: Follow ESP32-C6 Hardware Design Guidelines for antenna design and matching network
- If external: Place U.FL connector (J5) on board edge, specify antenna model (e.g., Taoglas FXP.10)

**Recommendation**: Use **external chip antenna via U.FL** for prototyping (easier, more reliable). Switch to PCB trace for cost-optimized production (requires VNA tuning).

---

### 5. BMS Communication Protocol
**Question**: Which communication protocol does the BMS use?

**Options**:
- [ ] **I²C** (shared bus with INA226 sensors, address: ___)
- [ ] **UART** (GPIO16/17, baud rate: ___)
- [ ] **CAN Bus** (requires MCP2551 transceiver, baud rate: 250k or 500k)
- [ ] **RS485** (requires MAX485 transceiver)
- [ ] **No communication** (BMS is "dumb", charge/discharge control only via GPIOs)

**Action Required**:
- Confirm BMS model and protocol from BMS datasheet
- Add appropriate transceiver IC if needed (CAN, RS485)
- Update schematic and firmware to match protocol

---

## LED Driver Questions

### 6. LED Driver Integration
**Question**: Is the LED driver integrated on this PCB or external module?

**Options**:
- [ ] **External LED Driver Module** (e.g., Mean Well HLG-150H-48A with 0–10V dimming)
  - Output connector (J6) provides 0–10V dimming signal only
  - LED driver powered directly from BMS high voltage output (48–129V)
- [ ] **Integrated on PCB** (constant-current LED driver circuit on this PCB)
  - Requires additional components (LED driver IC, inductor, output capacitors)
  - Increases PCB complexity

**Action Required**:
- If external: Specify LED driver model, input voltage (48V, 96V, or adjustable), output current
- If integrated: Design LED driver circuit (e.g., using LT3755, LM3407, or similar LED driver IC)

**Recommendation**: Use **external LED driver module** for simplicity and modularity. This MPPT PCB provides 48–129V power and 0–10V dimming control.

---

### 7. LED Driver Output Voltage and Current
**Question**: What are the LED driver output specifications?

**Required Information**:
- **LED String Voltage**: ___ V (e.g., 36V for 12S LED string)
- **LED Current**: ___ mA or A (e.g., 350mA, 700mA, 1A)
- **Total LED Power**: ___ W (max 150W from MPPT)

**Action Required**:
- Confirm LED specifications from LED panel or fixture datasheet
- Select compatible LED driver module (if external)
- Verify MPPT output voltage range (48–129V) is compatible with LED driver input

---

## Safety and Certification Questions

### 8. High Voltage Output: Fixed or Adjustable?
**Question**: Should the high voltage output (48–129V) be user-adjustable or factory-set?

**Options**:
- [ ] **Fixed at __ V** (e.g., 48V, 96V, or 129V): Simpler, safer, pre-calibrated
- [ ] **Adjustable via trimmer potentiometer**: Allows field adjustment, risk of user error
- [ ] **Adjustable via firmware**: Requires USB connection to change, safer than potentiometer

**Safety Consideration**: High voltage (>60V DC) can be lethal. If adjustable, ensure:
- Clear labeling and warnings
- Over-voltage protection at 140V (already designed)
- Calibration procedure documented

**Action Required**:
- Decide on voltage setting method
- Add appropriate hardware (potentiometer, DAC, or fixed resistor values)
- Include safety warnings on silkscreen and documentation

---

### 9. Enclosure and DIN Rail Mounting
**Question**: Is an enclosure required? Should the PCB mount on DIN rail or wall?

**Options**:
- [ ] **No enclosure** (PCB only, for integration into larger system)
- [ ] **Plastic enclosure** (ABS, IP20 for indoor use)
- [ ] **Metal enclosure** (aluminum, EMI shielding, IP65 for outdoor use)
- [ ] **DIN rail mounting** (add DIN rail clips to enclosure or PCB standoffs)
- [ ] **Wall mounting** (add mounting brackets or holes)

**Action Required**:
- Specify enclosure dimensions (if needed)
- Add DIN rail clip footprints to PCB or enclosure
- Update mechanical notes with mounting instructions

---

### 10. EMI/EMC Testing and Certification
**Question**: Will the product undergo EMI/EMC testing? Which certifications are required?

**Certifications**:
- [ ] **FCC Part 15** (USA): Radiated and conducted emissions
- [ ] **CE/EN 55022** (Europe): Electromagnetic compatibility
- [ ] **Matter CSA Certification**: Required for Matter branding
- [ ] **Thread Certification**: Required for Thread logo and certification mark
- [ ] **DALI Alliance (DiiA) Certification**: Required for DALI+ logo (if marketing as DALI+ compatible)
- [ ] **IEC 62109** (Safety for PV converters): Recommended for solar applications

**Action Required**:
- Budget time and cost for certification testing ($5k–20k depending on certifications)
- Design PCB with EMI mitigation (solid ground plane, filtering, shielding)
- Plan for pre-compliance testing to catch issues early

**Recommendation**: At minimum, perform **pre-compliance EMI scan** with near-field probe before finalizing design.

---

## Design Trade-Offs

### 11. Efficiency vs. Cost vs. Complexity
**Question**: What is the priority for this design?

**Options**:
- [ ] **Maximum Efficiency** (>97% target): Use synchronous rectifier, high-quality components, optimize layout
  - Higher cost, more complex firmware (complementary PWM with deadtime)
- [ ] **Lowest Cost** (<$60 BOM): Use Schottky diode, standard components, 2-layer PCB
  - Lower efficiency (~94%), simpler design
- [ ] **Ease of Assembly** (hand-solderable): Use through-hole components, larger packages
  - Lower density, larger PCB area

**Action Required**:
- Prioritize one or two factors
- Adjust component selection and PCB design accordingly

---

### 12. Production Volume
**Question**: What is the expected production volume?

**Options**:
- [ ] **Prototype** (1–10 units): Optimize for ease of testing and modification
- [ ] **Small Batch** (10–100 units): Use readily available components, consider hand assembly or low-volume PCB assembly service
- [ ] **Production** (100–10,000 units): Optimize for cost, use automated assembly (SMT), negotiate component pricing

**Action Required**:
- Select components based on availability and lead times
- Plan assembly method (hand, low-volume PCBA service, or high-volume contract manufacturer)
- Consider second-source components for critical parts (e.g., alternative to ESP32-C6 if stock issues)

---

## Documentation Completeness

### 13. Firmware Availability
**Question**: Will firmware be developed in-house or externally?

**Existing Firmware**:
- ✅ MPPT P&O algorithm (`mppt_pno.c`)
- ✅ ADC driver (`adc.c`)
- ✅ Main application (`main/main.c`)

**Additional Firmware Needed**:
- [ ] INA226 I²C driver (voltage/current sensing)
- [ ] WS2812B RMT driver (Thread FDTRGB LED)
- [ ] BMS communication driver (I²C/UART/CAN depending on BMS model)
- [ ] Matter/Thread stack integration (esp-matter SDK)
- [ ] DALI+ over IP client (TCP socket, PDU encoding)
- [ ] LED dimming control (PWM to 0–10V)
- [ ] Protection logic (OVP, OCP, thermal shutdown)

**Action Required**:
- Assign firmware development tasks
- Estimate firmware development time (4–8 weeks for full integration)
- Plan for testing and debugging

---

### 14. Testing and Validation Plan
**Question**: What testing is planned before production?

**Test Phases**:
- [ ] **Functional Testing**: Power-on, voltage regulation, MPPT tracking
- [ ] **Efficiency Testing**: Measure efficiency at multiple operating points (25%, 50%, 75%, 100% load)
- [ ] **Thermal Testing**: Run at 150W for 30 minutes, measure component temperatures
- [ ] **Protection Testing**: Trigger OVP, OCP, thermal shutdown, reverse polarity
- [ ] **EMI Pre-Compliance**: Near-field scan, spectrum analyzer on input/output
- [ ] **Matter Commissioning**: QR code provisioning, endpoint discovery
- [ ] **DALI+ Testing**: TCP connection, dimming commands, response verification
- [ ] **Environmental Testing**: Temperature range (-20°C to +60°C), humidity (if outdoor use)

**Action Required**:
- Create detailed test plan document
- Specify pass/fail criteria for each test
- Plan for test equipment (oscilloscope, electronic load, spectrum analyzer, thermal camera)

---

## Final Design Review Checklist

Before releasing schematic for PCB layout:

- [ ] All questions in this document answered and documented
- [ ] BOM reviewed and components confirmed available (check lead times)
- [ ] Schematic reviewed by senior engineer (peer review)
- [ ] DRC (Design Rule Check) passed with no errors
- [ ] ERC (Electrical Rule Check) passed with no errors
- [ ] Power budget calculated and verified (input power ≥ output power / efficiency)
- [ ] Thermal analysis performed (all components within temperature limits)
- [ ] Safety review completed (high voltage clearances, protections, warnings)
- [ ] Regulatory requirements identified (FCC, CE, Matter, Thread, DALI+)
- [ ] Assembly method selected (hand, low-volume PCBA, or production)
- [ ] Testing plan defined with pass/fail criteria
- [ ] Firmware development plan created with timeline
- [ ] Risk analysis performed (component obsolescence, supply chain, technical risks)

---

## Contact Information

**Hardware Designer**: _______________________

**Firmware Developer**: _______________________

**Project Manager**: _______________________

**Deadline for Answers**: _______________________

**Prototype Target Date**: _______________________

---

## Notes and Clarifications

(Use this space to document answers to the above questions and any additional notes)

___________________________________________________________________________

___________________________________________________________________________

___________________________________________________________________________

___________________________________________________________________________

---

## Revision History

- **v1.0** (2025-12-15): Initial hardware questions checklist for MPPT 150W boost converter

---

**End of Hardware Questions for Designer**
