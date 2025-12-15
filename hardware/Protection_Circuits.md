# Protection Circuits - MPPT 150W Boost Converter

## Overview

This document specifies the protection circuits for the MPPT boost converter to ensure safe operation under fault conditions. Multiple layers of protection prevent damage to components, battery, and connected loads.

## Protection Summary

| Protection Type | Method | Threshold | Action | Response Time |
|-----------------|--------|-----------|--------|---------------|
| Reverse Polarity | P-MOSFET + Zener | PV polarity | Block reverse current | Instant (passive) |
| Over-Voltage (OVP) | Comparator + latch | 140V | Disable PWM, alert ESP32 | <10µs (hardware) |
| Over-Current PV (OCP) | INA226 monitoring | 10A | Reduce duty or shutdown | <20ms (software) |
| Over-Current Out (OCP) | INA226 monitoring | 3A | Reduce duty or shutdown | <20ms (software) |
| Thermal Shutdown | NTC + ADC | 85°C MOSFET temp | Shutdown PWM | <100ms (software) |
| Watchdog | ESP32-C6 internal | Firmware hang | System reset | Configurable |

---

## 1. Reverse Polarity Protection (PV Input)

### Function
Protects the circuit if PV panel is connected with reversed polarity (PV+ to GND, PV− to input+).

### Topology: P-Channel MOSFET

**Advantages**:
- Low voltage drop (Rds(on) × Ipv)
- No power dissipation when OFF (reverse polarity condition)
- Better than diode (lower forward drop)

**Circuit**:
```
PV+ ────┬──────────────────┐
        │                  │
        │               [Zener D2]
        │               18V, 1W
        │                  │
        │          Gate────┴───[Rpull 10kΩ]───┐
        │            │                        │
        │         Source                      │
        │            │                        │
     [Cin_bulk] [Q3 P-MOSFET]                │
     100µF/63V      │                         │
        │         Drain                       │
        │            │                        │
        │            └────> PV_PROT+ (to boost converter input)
        │                                     │
PV− ────┴─────────────────────────────────────┴──> GND
```

### Component Selection

#### Q3: P-Channel MOSFET
- **Part**: IRF4905 or similar
- **Specifications**:
  - Vds: -55V (or higher)
  - Rds(on): <20mΩ @ Vgs = -10V
  - Id: -74A @ 25°C (huge margin for 10A PV current)
  - Package: TO-220 or D2PAK
- **Connection**:
  - Source → PV+ (input connector)
  - Drain → PV_PROT+ (protected rail to boost converter)
  - Gate → PV− (GND) via 10kΩ resistor
- **Supplier**: Mouser #942-IRF4905PBF
- **Cost**: ~$1.50

**Alternative**: FQP47P06 (60V, -47A, 17mΩ)

#### D2: Zener Diode (Gate Protection)
- **Function**: Clamp gate-source voltage to safe level if PV voltage exceeds Vgs(max)
- **Specifications**:
  - Vz: 18V (typical Vgs(max) for MOSFETs is ±20V)
  - Power: 1W (ensures margin)
  - Package: DO-41 or SMA
- **Connection**: Cathode to Gate, Anode to Source (PV+)
- **Part**: 1N4746A (18V, 1W Zener)
- **Supplier**: Mouser #512-1N4746A

#### Rpull: Pull-down Resistor
- **Value**: 10kΩ
- **Function**: Ensures gate is pulled to source when PV is disconnected (keeps Q3 ON during normal operation)
- **Power**: 0.125W (1/8W)
- **Part**: Yageo RC0603FR-0710KL

### Operation

#### Normal Polarity (PV+ to PV+, PV− to GND)
1. Gate is pulled to PV− (GND) via 10kΩ resistor
2. Source is at PV+ (18–40V)
3. Vgs = GND - PV+ = -18V to -40V (negative voltage)
4. P-MOSFET is ON (Vgs < Vgs(th) ≈ -2V)
5. Current flows: PV+ → Source → Drain → PV_PROT+ (low resistance path)

#### Reverse Polarity (PV+ to GND, PV− to input+)
1. Source is at GND (0V)
2. Gate is pulled to PV− (now at +18 to +40V)
3. Vgs = +18V to +40V (positive voltage, clamped to +18V by Zener)
4. P-MOSFET is OFF (Vgs > 0V)
5. No current flows (reverse polarity blocked)

#### Power Dissipation (Normal Operation)
```
Vdrop = Ipv × Rds(on)
For Ipv = 10A, Rds(on) = 20mΩ:
Vdrop = 10 × 0.020 = 0.2V

P_dissipation = Ipv² × Rds(on)
P_dissipation = 10² × 0.020 = 2W
```

**Thermal Check**:
- Rθ(JC) = 0.75°C/W (TO-220)
- Rθ(CA) = 30°C/W (no heatsink, PCB copper area)
- ΔT = P × (Rθ(JC) + Rθ(CA)) = 2 × (0.75 + 30) = 61.5°C
- Tj = 50°C (ambient) + 61.5°C = 111.5°C (acceptable, <150°C max)

**Note**: Add copper area (≥5cm²) on PCB under Q3 for better heat dissipation.

---

## 2. Over-Voltage Protection (OVP)

### Function
Prevents output voltage from exceeding 140V, protecting BMS, LED driver, and output capacitors (rated 160V).

### Topology: Comparator + Latch

**Method**:
- Voltage divider from OUT+ to sense output voltage
- Comparator (LM393) compares divided voltage to reference
- When Vout > 140V, comparator output goes high
- Latch signal sent to ESP32-C6 GPIO
- Hardware AND gate disables PWM immediately (optional, for fast response)

**Circuit**:
```
OUT+ (48–140V) ───┬─── R_OVP1 (470kΩ, 1%, 0.5W) ───┬─── R_OVP2 (4.7kΩ, 1%) ───┬─── GND
                  │                                │                          │
               [C_OVP_F]                           │                          │
               10nF, 1kV                        V_sense                        │
                  │                             (1.4V @ 140V)                 │
                  │                                │                          │
                  │                                │                          │
                  │                     LM393 ─────┤                          │
                  │                     Comparator │ (+)                      │
                  │                                │                          │
                  │                            [R_ref1]                       │
                  │                            10kΩ                           │
                  │                                │                          │
                  │                             V_ref ────[R_ref2]─────────────┘
                  │                            (1.25V)   10kΩ + TL431 option
                  │
                  │                     LM393 output (OVP_LATCH)
                  │                          │
                  └──────────────────────────┼────> ESP32-C6 GPIO10 (interrupt)
                                             │
                                          [R_PU]
                                          10kΩ to 3.3V
```

### Component Selection

#### U6: LM393 Dual Comparator
- **Function**: Compare output voltage (via divider) to reference voltage
- **Specifications**:
  - Supply: 3.3V (single supply operation)
  - Input range: 0–3.3V (within supply rails)
  - Output: Open-collector (requires pull-up)
  - Propagation delay: ~300ns
- **Package**: SOIC-8 or DIP-8
- **Supplier**: Mouser #926-LM393DR (SOIC-8)

#### Voltage Divider (R_OVP1, R_OVP2)
**Function**: Scale 140V down to ~1.4V for comparator input

**Calculation**:
```
Ratio = R_OVP2 / (R_OVP1 + R_OVP2)
For 140V → 1.4V: Ratio = 1.4 / 140 = 0.01 (1%)

Select: R_OVP1 = 470kΩ, R_OVP2 = 4.7kΩ
Ratio = 4.7kΩ / (470kΩ + 4.7kΩ) = 4.7 / 474.7 = 0.0099 ≈ 0.01
V_sense = Vout × 0.01

At Vout = 140V: V_sense = 140 × 0.01 = 1.4V
```

**Resistor Specs**:
- **R_OVP1**: 470kΩ, 1%, 0.5W (high voltage)
  - Use 2× 220kΩ in series for higher voltage rating if needed
  - Part: Vishay CRCW0805470KFKEA
- **R_OVP2**: 4.7kΩ, 1%, 0.125W
  - Part: Vishay CRCW08054K70FKEA

**Current Draw**:
```
I_div = Vout / (R_OVP1 + R_OVP2)
At Vout = 140V: I_div = 140 / 474.7k = 0.295mA (low power)
```

#### Reference Voltage (V_ref)

**Option 1: Resistive Divider from 3.3V**
```
V_ref = 3.3V × (R_ref2 / (R_ref1 + R_ref2))
For V_ref = 1.25V: Ratio = 1.25 / 3.3 = 0.379

Select: R_ref1 = 10kΩ, R_ref2 = 6.2kΩ
V_ref = 3.3 × (6.2 / 16.2) = 1.26V ≈ 1.25V
```

**Option 2: TL431 Shunt Regulator** (more accurate)
- TL431 provides 2.5V reference
- Use voltage divider to get 1.25V from 2.5V
- Better stability than resistive divider from 3.3V

**Recommended**: Use resistive divider (simpler) for ±5% accuracy, or TL431 for ±1% accuracy.

#### Pull-Up Resistor (R_PU)
- **Value**: 10kΩ to 3.3V
- **Function**: Pull comparator output high when Vout < 140V (comparator output is open-collector)
- **Part**: Yageo RC0603FR-0710KL

#### Filter Capacitor (C_OVP_F)
- **Value**: 10nF, 1kV ceramic
- **Function**: Filter high-frequency noise on OUT+ line
- **Placement**: Close to voltage divider top (R_OVP1)

### Operation

#### Normal Operation (Vout < 140V)
- V_sense < 1.4V
- Comparator output: HIGH (via pull-up)
- OVP_LATCH = HIGH (no fault)
- ESP32-C6 GPIO10 reads HIGH (no interrupt)

#### Over-Voltage Fault (Vout ≥ 140V)
- V_sense ≥ 1.4V
- Comparator output: LOW (pulls to GND)
- OVP_LATCH = LOW (fault detected)
- ESP32-C6 GPIO10 reads LOW (interrupt triggered)
- Firmware action: Disable PWM, reduce duty cycle, log fault

### Hardware PWM Disable (Optional)

For immediate hardware shutdown (without waiting for firmware response):

**Circuit**:
```
PWM_BOOST (from ESP32-C6) ──┬─── AND gate (74HC08) ───> PWM_TO_DRIVER (to IR2110)
                            │           │
                            │       [OVP_LATCH_INV]
                            │       (inverted OVP signal)
                            │
                        [R_pullup]
                         10kΩ
```

When OVP_LATCH goes LOW, AND gate blocks PWM signal, immediately stopping boost converter.

**Note**: This adds complexity. Firmware-based shutdown is usually sufficient (<20ms response time).

---

## 3. Over-Current Protection (OCP)

### Method: Software Monitoring via INA226

**Advantages**:
- No additional hardware (uses existing INA226 sensors)
- Programmable thresholds
- Can implement soft limits (reduce duty) or hard limits (shutdown)

**Implementation**:

```c
// In MPPT control loop (20ms period)
float vpv, ipv, ppv;
read_pv_measurements(&vpv, &ipv, &ppv);

if (ipv > 10.0) {  // PV side over-current (10A limit)
    ESP_LOGW(TAG, "OCP: PV current %.2fA exceeds 10A limit", ipv);
    duty = duty * 0.9;  // Reduce duty by 10%
    ocp_fault_count++;
    if (ocp_fault_count > 5) {
        ESP_LOGE(TAG, "OCP: Persistent over-current, shutting down");
        duty = 0.0;  // Shutdown
        system_fault |= FAULT_OCP_PV;
    }
}

float vout, iout, pout;
read_output_measurements(&vout, &iout, &pout);

if (iout > 3.0) {  // Output side over-current (3A limit)
    ESP_LOGW(TAG, "OCP: Output current %.2fA exceeds 3A limit", iout);
    duty = duty * 0.95;  // Reduce duty by 5%
    ocp_fault_count++;
    if (ocp_fault_count > 5) {
        ESP_LOGE(TAG, "OCP: Persistent over-current, shutting down");
        duty = 0.0;  // Shutdown
        system_fault |= FAULT_OCP_OUT;
    }
}
```

### Hardware Fast OCP (Optional)

For faster response (<1µs), add comparator on shunt voltage:

**Circuit**:
```
SHUNT_PV_P ────┬───[R_shunt 1mΩ]───┬──── SHUNT_PV_N
               │                    │
               │                    │
            (INA226 V+)          (INA226 V-)
               │                    │
               └────> LM393 (+)     │
                      Comparator    │
                          │         │
                      V_ref_OCP ────┘
                      (10mV = 10A for 1mΩ shunt)
                          │
                    LM393 output ──> OCP_LATCH ──> ESP32-C6 or hardware PWM disable
```

**Threshold**:
- For 10A limit on 1mΩ shunt: V_shunt = 10mV
- Set V_ref_OCP = 10mV using precision voltage reference or TL431 + divider

**Note**: Hardware OCP adds complexity and may cause false triggers due to current ripple. Software OCP via INA226 is recommended for this design.

---

## 4. Thermal Protection

### Method: NTC Thermistor + ESP32-C6 ADC

**Sensor**: NTC 10kΩ @ 25°C (B-constant: 3950K)

**Placement**: Mounted on or near MOSFET Q1 (low-side, hottest component)

**Circuit**:
```
VCC_3V3 (3.3V) ───┬─── R_NTC_PULL (10kΩ) ───┬─── NTC1 (10kΩ @ 25°C) ───┬─── GND
                  │                          │                          │
                  │                      V_NTC_sense                    │
                  │                          │                          │
                  │                          └──────> ESP32-C6 GPIO11 (ADC)
                  │
               [C_NTC_F]
               100nF
```

### Component Selection

#### NTC1: Thermistor
- **Value**: 10kΩ ±1% @ 25°C
- **B-Constant**: 3950K ±1%
- **Temperature Range**: -40°C to +125°C
- **Package**: 0603 or 0805 SMD, or epoxy bead with leads
- **Mounting**: Thermal epoxy or direct PCB contact to MOSFET Q1 drain/case
- **Part**: Murata NXRT15XH103FA1B020 (10kΩ, 3950K, 0603)
- **Supplier**: Mouser #81-NXRT15XH103FA1B020

#### R_NTC_PULL: Pull-up Resistor
- **Value**: 10kΩ (same as NTC at 25°C for mid-range voltage)
- **Tolerance**: 1%
- **Part**: Yageo RC0603FR-0710KL

#### C_NTC_F: Filter Capacitor
- **Value**: 100nF ceramic
- **Function**: Low-pass filter to remove noise from ADC reading

### Temperature Calculation (Steinhart-Hart Equation)

**Simplified B-parameter equation**:
```
1/T = 1/T0 + (1/B) × ln(R/R0)

Where:
- T = Absolute temperature (K)
- T0 = Reference temperature (298.15K for 25°C)
- R = Measured resistance (Ω)
- R0 = Reference resistance (10kΩ at 25°C)
- B = B-constant (3950K)
```

**Firmware Implementation**:

```c
#include "esp_adc/adc_oneshot.h"

#define R_PULL 10000.0  // 10kΩ pull-up
#define R0_NTC 10000.0  // NTC resistance at 25°C
#define B_NTC  3950.0   // B-constant
#define T0_K   298.15   // 25°C in Kelvin

float read_temperature_c(void) {
    uint32_t adc_raw;
    adc_oneshot_read(adc_handle, ADC_CHANNEL_0, &adc_raw);  // GPIO11 = ADC1_CH0
    
    // Convert ADC to voltage (assuming 12-bit ADC, 3.3V reference)
    float v_adc = (float)adc_raw / 4095.0 * 3.3;
    
    // Calculate NTC resistance from voltage divider
    float r_ntc = R_PULL * v_adc / (3.3 - v_adc);
    
    // Steinhart-Hart equation (simplified)
    float ln_ratio = logf(r_ntc / R0_NTC);
    float temp_k = 1.0 / (1.0/T0_K + ln_ratio/B_NTC);
    float temp_c = temp_k - 273.15;
    
    return temp_c;
}

// In MPPT control loop:
float temp_mosfet = read_temperature_c();

if (temp_mosfet > 85.0) {
    ESP_LOGW(TAG, "Thermal warning: MOSFET temp %.1f°C", temp_mosfet);
    duty = duty * 0.9;  // Reduce power by 10%
    thermal_fault_count++;
    if (thermal_fault_count > 3 || temp_mosfet > 95.0) {
        ESP_LOGE(TAG, "Thermal shutdown: MOSFET temp %.1f°C", temp_mosfet);
        duty = 0.0;  // Shutdown
        system_fault |= FAULT_THERMAL;
        // Update Thread FDTRGB LED to red blink (fault)
    }
}
```

### Thermal Thresholds

| Temperature | Action | LED Status |
|-------------|--------|------------|
| <70°C | Normal operation | Green (running/good) |
| 70–85°C | Warning (log only) | Yellow pulse |
| 85–95°C | Reduce power by 10% | Orange pulse |
| >95°C | Emergency shutdown | Red blink (fault) |

---

## 5. Input Fuse (Optional)

### Recommended: 15A Fast-Blow Fuse

**Function**: Protect against catastrophic short-circuit or component failure

**Specifications**:
- **Current Rating**: 15A (50% margin over 10A max PV current)
- **Voltage Rating**: 60V DC minimum
- **Type**: Fast-blow (F) or Time-delay (T)
- **Package**: 5×20mm glass tube or blade fuse

**Recommended Part**:
- Littelfuse 0215015.MXP (15A, 125V, fast-blow, 5×20mm)
- Supplier: Mouser #576-0215015.MXP

**Placement**: On PV+ line, before reverse polarity protection (Q3)

**Fuse Holder**:
- PCB mount 5×20mm fuse holder
- Part: Keystone 3557-2 (PCB mount, 5×20mm)
- Supplier: Mouser #534-3557-2

**Note**: Fusing is optional but recommended for safety, especially for outdoor or unattended installations.

---

## 6. Watchdog Timer

### ESP32-C6 Internal Watchdog

**Function**: Reset system if firmware hangs or infinite loop occurs

**Configuration** (ESP-IDF):

```c
#include "esp_task_wdt.h"

void app_main(void) {
    // Initialize watchdog (10 second timeout)
    esp_task_wdt_config_t wdt_config = {
        .timeout_ms = 10000,  // 10 seconds
        .idle_core_mask = 0,  // Don't monitor idle task
        .trigger_panic = true // Panic and reset on timeout
    };
    esp_task_wdt_init(&wdt_config);
    
    // Add current task to watchdog
    esp_task_wdt_add(NULL);
    
    // In main loop:
    while (1) {
        // ... MPPT algorithm ...
        esp_task_wdt_reset();  // Feed watchdog (must be called every <10s)
        vTaskDelay(pdMS_TO_TICKS(20));  // 20ms MPPT period
    }
}
```

**Timeout**: 10 seconds (adjust based on longest expected task execution time)

**Action on Timeout**: System reset (ESP32-C6 reboots)

---

## Protection Status and Fault Logging

### Fault Flags (Firmware)

```c
typedef enum {
    FAULT_NONE      = 0x00,
    FAULT_OVP       = 0x01,  // Over-voltage
    FAULT_OCP_PV    = 0x02,  // Over-current PV
    FAULT_OCP_OUT   = 0x04,  // Over-current output
    FAULT_THERMAL   = 0x08,  // Thermal shutdown
    FAULT_BMS       = 0x10,  // BMS fault signal
    FAULT_COMM      = 0x20,  // Communication error (I2C, UART)
} fault_flags_t;

static uint32_t system_fault = FAULT_NONE;
```

### Thread FDTRGB LED Status

Update LED based on fault status:

```c
void update_status_led(void) {
    if (system_fault != FAULT_NONE) {
        set_led_color(RGB_RED);      // Red
        set_led_blink(500);          // 500ms blink (fault state)
    } else if (vpv < 10.0) {
        set_led_color(RGB_ORANGE);   // Orange (disconnected, no PV)
    } else if (mppt_running) {
        set_led_color(RGB_GREEN);
        set_led_pulse(1000);         // Green pulse (MPPT tracking)
    } else {
        set_led_color(RGB_GREEN);    // Solid green (good, charging)
    }
}
```

### Matter Fault Reporting

Report faults via Matter cluster (if using Matter over Thread):

```c
// Update Matter endpoint attribute for fault status
matter_update_fault_attribute(endpoint_id, system_fault);
```

---

## BOM for Protection Circuits

| Ref Des | Part | Value | Package | Qty | Supplier Part # | Notes |
|---------|------|-------|---------|-----|-----------------|-------|
| Q3 | P-MOSFET | IRF4905 | TO-220 | 1 | Infineon IRF4905PBF | Reverse polarity protection |
| D2 | Zener | 18V, 1W | DO-41 | 1 | ON Semi 1N4746A | Gate protection for Q3 |
| U6 | Comparator | LM393 | SOIC-8 | 1 | TI LM393DR | OVP comparator |
| R_OVP1 | Resistor | 470kΩ, 1%, 0.5W | 0805 | 1 | Vishay CRCW0805470KFKEA | OVP divider |
| R_OVP2 | Resistor | 4.7kΩ, 1% | 0603 | 1 | Vishay CRCW08054K70FKEA | OVP divider |
| R_ref1 | Resistor | 10kΩ, 1% | 0603 | 1 | Yageo RC0603FR-0710KL | OVP reference |
| R_ref2 | Resistor | 6.2kΩ, 1% | 0603 | 1 | Yageo RC0603FR-076K2L | OVP reference |
| R_PU_OVP | Resistor | 10kΩ | 0603 | 1 | Yageo RC0603FR-0710KL | OVP pull-up |
| C_OVP_F | Capacitor | 10nF, 1kV, X7R | 1206 | 1 | TDK C3216X7R2E103K160AA | OVP filter |
| NTC1 | Thermistor | 10kΩ @ 25°C, B3950 | 0603 | 1 | Murata NXRT15XH103FA1B020 | Thermal sensor |
| R_NTC_PULL | Resistor | 10kΩ, 1% | 0603 | 1 | Yageo RC0603FR-0710KL | NTC pull-up |
| C_NTC_F | Capacitor | 100nF, X7R | 0603 | 1 | Murata GRM188R71H104KA93D | NTC filter |
| Rpull_Q3 | Resistor | 10kΩ | 0603 | 1 | Yageo RC0603FR-0710KL | Q3 gate pull-down |
| F1 | Fuse | 15A, 125V, fast-blow | 5×20mm | 1 | Littelfuse 0215015.MXP | Input fuse (optional) |
| Fuse Holder | Holder | 5×20mm PCB mount | - | 1 | Keystone 3557-2 | Fuse holder (optional) |

---

## Design Checklist

- [ ] Reverse polarity P-MOSFET (Q3) selected and oriented correctly (source to PV+, drain to converter)
- [ ] Zener diode (D2) on Q3 gate for overvoltage protection
- [ ] OVP comparator circuit designed (voltage divider, LM393, reference)
- [ ] OVP threshold verified (140V → 1.4V sense voltage)
- [ ] OVP signal routed to ESP32-C6 GPIO10 (with pull-up)
- [ ] INA226 OCP implemented in firmware (10A PV, 3A output)
- [ ] NTC thermistor mounted on MOSFET Q1 heatsink/case
- [ ] Thermal protection thresholds configured in firmware (85°C warning, 95°C shutdown)
- [ ] Watchdog timer enabled in firmware (10s timeout)
- [ ] Input fuse selected and placed (optional, 15A fast-blow)
- [ ] Protection status reported via Thread FDTRGB LED and Matter cluster
- [ ] All protection circuits tested (OVP, OCP, thermal, reverse polarity)

---

## Testing and Validation

### 1. Reverse Polarity Test
- **Setup**: Apply reverse voltage (PV+ to GND, PV− to +24V) via current-limited supply
- **Expected**: No current flow, Q3 blocks reverse polarity
- **Verify**: No damage to downstream components, no current >1mA

### 2. OVP Test
- **Setup**: Slowly increase output voltage via external supply or reduce duty cycle to boost voltage
- **Expected**: At Vout = 140V, comparator triggers, GPIO10 goes LOW, PWM disabled
- **Verify**: Output voltage stops rising, firmware logs OVP fault

### 3. OCP Test (PV Side)
- **Setup**: Apply electronic load on PV input, increase current to >10A
- **Expected**: INA226 reads >10A, firmware reduces duty cycle or shuts down
- **Verify**: Current limited to ~10A, no component damage

### 4. Thermal Test
- **Setup**: Run converter at 150W for 30 minutes, monitor MOSFET temperature
- **Expected**: At T > 85°C, firmware reduces power; at T > 95°C, shutdown
- **Verify**: Temperature stays <95°C, or shutdown occurs if exceeded

### 5. Watchdog Test
- **Setup**: Introduce infinite loop in firmware (comment out watchdog reset)
- **Expected**: After 10 seconds, watchdog triggers system reset
- **Verify**: ESP32-C6 reboots, system recovers

---

## Common Faults and Troubleshooting

| Symptom | Possible Cause | Solution |
|---------|----------------|----------|
| No output voltage with PV connected | Reverse polarity protection triggered | Check PV polarity, verify Q3 operation |
| Output voltage exceeds 140V | OVP comparator not working | Check voltage divider, comparator power, reference voltage |
| Frequent OCP shutdowns | Current limit too low or actual overcurrent | Verify INA226 calibration, check load current |
| Thermal shutdown at low power | NTC sensor disconnected or incorrect | Check NTC connection, verify temperature calculation |
| System resets randomly | Watchdog timeout (firmware hang) | Debug firmware, increase watchdog timeout, add logging |

---

## References

- IRF4905 P-Channel MOSFET Datasheet (Infineon)
- LM393 Dual Comparator Datasheet (Texas Instruments)
- ESP32-C6 Task Watchdog Timer Documentation (ESP-IDF)
- Application Note: "Reverse Battery Protection" (Infineon AN-1542)
- IEC 62109-1: Safety of Power Converters for Photovoltaic Systems

## Revision History

- **v1.0** (2025-12-15): Initial protection circuits specification for MPPT 150W boost converter
