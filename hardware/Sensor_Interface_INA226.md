# Sensor Interface - INA226 Current and Voltage Monitoring

## Overview

This document specifies the INA226-based current and voltage sensing circuits for PV input and high-voltage output monitoring. Two INA226 devices provide complete power measurement for MPPT algorithm and system monitoring.

## INA226 Overview

**Texas Instruments INA226** is a high-side current and voltage monitor with I²C interface.

### Key Features
- **Voltage Range**: 0–36V (bus voltage)
- **Current Sensing**: ±81.92mV max differential voltage across shunt
- **Resolution**: 16-bit ADC (bus voltage: 1.25mV/LSB, shunt voltage: 2.5µV/LSB)
- **I²C Interface**: Standard (100kHz), Fast (400kHz), High-Speed (2.94MHz)
- **Power Calculation**: Internal multiplier for real-time power (W)
- **Alerts**: Programmable over-current, under-voltage, over-power alerts
- **Supply**: 2.7–5.5V (use 3.3V logic rail)

### Functional Block Diagram
```
VIN+ (Vbus) ─────┬───────────> 16-bit ADC ──> Voltage Measurement
                 │
     ┌───────────┴───────────┐
     │   INA226 Die          │
     │                       │
VIN- ─────┬──> 16-bit ADC ──┼──> Shunt Voltage → Current Calculation
          │                  │
V+ (Shunt)┘                  └──> Power = V × I (internal multiplier)
                                  │
V- (Shunt)────────────────────────┘
                                  
SDA/SCL <──────> I²C Interface
```

## INA226 #1: PV Input Monitoring

### I²C Address
**0x40** (A0=GND, A1=GND)

### Connections

| INA226 Pin | Pin # | Signal | Connection | Description |
|------------|-------|--------|------------|-------------|
| VIN+ (Vbus) | 1 | PV_BUS | PV voltage after protection | 0–40V input |
| VIN- (Return) | 2 | GND | Ground reference | 0V |
| V+ (Shunt+) | 3 | SHUNT_PV_P | Positive side of shunt (Kelvin) | To PV+ side of shunt |
| V- (Shunt-) | 4 | SHUNT_PV_N | Negative side of shunt (Kelvin) | To load side of shunt |
| SDA | 5 | I2C_SDA | I²C data line | GPIO8 (ESP32-C6) |
| SCL | 6 | I2C_SCL | I²C clock line | GPIO9 (ESP32-C6) |
| ALERT | 7 | NC or GPIO | Optional over-current alert | Not connected or to GPIO for interrupt |
| GND | 8 | GND | Ground | 0V |
| VS | 9 | VCC_3V3 | Power supply | 3.3V rail |
| A1 | 10 | GND | Address bit 1 | Low for address 0x40 |
| A0 | 11 | GND | Address bit 0 | Low for address 0x40 |

### Shunt Resistor (PV Side)

**Specifications**:
- **Resistance**: 1mΩ (0.001Ω)
- **Power Rating**: 3W minimum (for 10A max current: P = I² × R = 100 × 0.001 = 0.1W, but use 3W for safety and transients)
- **Tolerance**: ±1% or better
- **Temperature Coefficient**: ±100ppm/°C or better
- **Type**: 4-terminal (Kelvin) current sense resistor

**Recommended Parts**:

#### Option 1: Vishay WSLP3921L1000FEA
- **Resistance**: 1mΩ ±1%
- **Power**: 3W
- **Size**: 3921 (10×5.3mm)
- **Kelvin**: Yes (4-terminal)
- **TCR**: ±75ppm/°C
- **Supplier**: Mouser #71-WSLP3921L1000FEA
- **Cost**: ~$1.50

#### Option 2: Ohmite LVK12R0010FE
- **Resistance**: 1mΩ ±1%
- **Power**: 3W
- **Size**: 2512 equivalent
- **Kelvin**: Yes
- **TCR**: ±50ppm/°C
- **Supplier**: Mouser #588-LVK12R0010FE

### Measurement Range (PV Side)

**Voltage Measurement (Vbus)**:
- **Range**: 0–40V (direct connection to PV+)
- **Resolution**: 1.25mV per LSB (16-bit ADC)
- **Accuracy**: ±0.1% (typical)

**Current Measurement (Shunt)**:
- **Shunt Voltage**: ±81.92mV max (INA226 limit)
- **Maximum Current**: 81.92mV / 1mΩ = **81.92A** (far exceeds 10A requirement)
- **Actual Range**: 0–10A (limited by PV panel and protection)
- **Shunt Voltage at 10A**: 10A × 1mΩ = **10mV**
- **Resolution**: 2.5µV / 1mΩ = **2.5mA per LSB**
- **Accuracy**: ±0.1% (typical) + shunt tolerance (±1%) = ±1.1% total

**Power Calculation**:
- **Range**: 0–400W (40V × 10A)
- **Typical**: 0–150W (MPPT operating point)
- **Resolution**: Voltage × Current resolution
- **Update Rate**: Configurable, 1ms to 8.244s per conversion

### INA226 #1 Configuration

**Calibration Register**:
The INA226 requires a calibration value to compute current and power correctly.

```
Current_LSB = Maximum Expected Current / 32768
For 10A max: Current_LSB = 10A / 32768 = 305µA

CAL = 0.00512 / (Current_LSB × R_shunt)
CAL = 0.00512 / (305e-6 × 0.001) = 16787 = 0x4193
```

**Configuration Register (0x00)**:
- **Averaging**: 16 samples (reduces noise)
- **Vbus Conversion Time**: 1.1ms
- **Vshunt Conversion Time**: 1.1ms
- **Mode**: Continuous, both shunt and bus voltage

Example: `0x4527` (AVG=16, Vbus=1.1ms, Vshunt=1.1ms, continuous)

### PCB Layout (INA226 #1)

#### Kelvin Connection (Critical!)

The shunt resistor must use Kelvin (4-terminal) connection to avoid voltage drop errors in the current measurement.

```
PV+ ──[wide 2mm trace]──┬──[Shunt 1mΩ]──┬──[wide 2mm trace]──> Boost Input
                        │                │
                        │ (power pads)   │
                        │                │
                     ┌──┘                └──┐
                     │ (sense pads)         │
                     │                      │
                  [thin 0.2mm trace]   [thin 0.2mm trace]
                     │                      │
                 SHUNT_PV_P              SHUNT_PV_N
                     │                      │
                     └────> INA226 V+       │
                            INA226 V- <─────┘
```

**Key Points**:
- Power current flows through **wide traces** (2mm) to/from shunt **power pads**.
- Sense traces (thin, 0.2mm) connect to shunt **sense pads** (separate from power pads on 4-terminal shunt).
- Sense traces go **directly** to INA226 V+ and V- pins (no other connections).
- Do NOT connect sense and power pads together except at the shunt resistor.

#### Component Placement
- INA226 #1 close to PV shunt (within 50mm)
- Decoupling: 100nF ceramic capacitor on VS pin (3.3V), close to IC
- Ground: Solid connection to ground plane
- I²C traces: Route SDA and SCL together, avoid crossing high current paths

---

## INA226 #2: High Voltage Output Monitoring

### I²C Address
**0x41** (A0=VCC, A1=GND)

### Voltage Divider (Output Voltage Sensing)

The output voltage (48–129V) exceeds the INA226 Vbus maximum (36V), so a **resistive divider** is required.

**Divider Ratio**: 5:1 (approximately)
```
Vout_max = 129V → Vbus_INA226 = 129V / 5 = 25.8V (within 36V limit)
Vout_min = 48V → Vbus_INA226 = 48V / 5 = 9.6V
```

**Resistor Values**:
- **R1** (high-side): 100kΩ, 1%, 0.25W (high voltage rated)
- **R2** (low-side): 25kΩ, 1%, 0.125W
- **Total Resistance**: 125kΩ
- **Current Draw**: 129V / 125kΩ = **1.03mA** (low power consumption)

**Divider Circuit**:
```
OUT+ (48–129V) ──┬── R1 (100kΩ) ──┬── R2 (25kΩ) ──┬── GND
                 │                 │               │
              (Optional           Vbus_INA226      │
               HV cap)            (9.6–25.8V)      │
                                  │                │
                                  └────────────────┴─> INA226 VIN+ (pin 1)
                                                        INA226 VIN- (pin 2) to GND
```

**Decoupling** (optional but recommended):
- 10nF ceramic capacitor (1kV rated) from OUT+ to GND near R1
- 100nF ceramic capacitor (50V) from Vbus_INA226 to GND (filters noise)

**Voltage Calculation in Firmware**:
```c
float vbus_ina226 = read_ina226_voltage(0x41);  // 9.6–25.8V
float vout_actual = vbus_ina226 * 5.0;         // Multiply by divider ratio
```

### Connections

| INA226 Pin | Pin # | Signal | Connection | Description |
|------------|-------|--------|------------|-------------|
| VIN+ (Vbus) | 1 | OUT_BUS_DIV | Output voltage via 5:1 divider | 9.6–25.8V |
| VIN- (Return) | 2 | GND | Ground reference | 0V |
| V+ (Shunt+) | 3 | SHUNT_OUT_P | Positive side of shunt (Kelvin) | To OUT+ side of shunt |
| V- (Shunt-) | 4 | SHUNT_OUT_N | Negative side of shunt (Kelvin) | To load side of shunt |
| SDA | 5 | I2C_SDA | I²C data line | GPIO8 (ESP32-C6) |
| SCL | 6 | I2C_SCL | I²C clock line | GPIO9 (ESP32-C6) |
| ALERT | 7 | NC or GPIO | Optional over-current alert | Not connected |
| GND | 8 | GND | Ground | 0V |
| VS | 9 | VCC_3V3 | Power supply | 3.3V rail |
| A1 | 10 | GND | Address bit 1 | Low for address 0x41 |
| A0 | 11 | VCC_3V3 | Address bit 0 | High for address 0x41 |

### Shunt Resistor (Output Side)

**Specifications**:
- **Resistance**: 10mΩ (0.01Ω)
- **Power Rating**: 1W minimum (for 3A max current: P = I² × R = 9 × 0.01 = 0.09W, but use 1W for safety)
- **Tolerance**: ±1% or better
- **Temperature Coefficient**: ±100ppm/°C or better
- **Type**: 4-terminal (Kelvin) current sense resistor

**Recommended Parts**:

#### Option 1: Vishay WSLP2512R0100FEA
- **Resistance**: 10mΩ ±1%
- **Power**: 1W
- **Size**: 2512 (6.4×3.2mm)
- **Kelvin**: Yes (4-terminal)
- **TCR**: ±75ppm/°C
- **Supplier**: Mouser #71-WSLP2512R0100FEA
- **Cost**: ~$0.80

#### Option 2: Ohmite LVK12R0100FE
- **Resistance**: 10mΩ ±1%
- **Power**: 2W
- **Size**: 2512 equivalent
- **Kelvin**: Yes
- **TCR**: ±50ppm/°C
- **Supplier**: Mouser #588-LVK12R0100FE

### Measurement Range (Output Side)

**Voltage Measurement (Vbus)**:
- **Divider Input**: 48–129V
- **INA226 Vbus**: 9.6–25.8V (after 5:1 divider)
- **Resolution**: 1.25mV per LSB × 5 = **6.25mV actual output voltage**
- **Accuracy**: ±0.1% (INA226) + ±2% (divider resistors 1% each) = ±2.1% total

**Current Measurement (Shunt)**:
- **Shunt Voltage**: ±81.92mV max (INA226 limit)
- **Maximum Current**: 81.92mV / 10mΩ = **8.19A**
- **Actual Range**: 0–3A (output current at 150W / 48V min)
- **Shunt Voltage at 3A**: 3A × 10mΩ = **30mV**
- **Resolution**: 2.5µV / 10mΩ = **0.25mA per LSB**
- **Accuracy**: ±0.1% (typical) + shunt tolerance (±1%) = ±1.1% total

**Power Calculation**:
- **Range**: 0–387W (129V × 3A)
- **Typical**: 0–150W (MPPT operating point)
- **Note**: Firmware must multiply Vbus reading by 5 to get actual output voltage for power calculation

### INA226 #2 Configuration

**Calibration Register**:
```
Current_LSB = Maximum Expected Current / 32768
For 3A max: Current_LSB = 3A / 32768 = 91.5µA

CAL = 0.00512 / (Current_LSB × R_shunt)
CAL = 0.00512 / (91.5e-6 × 0.01) = 5595 = 0x15DB
```

**Configuration Register (0x00)**:
- **Averaging**: 16 samples (reduces noise)
- **Vbus Conversion Time**: 1.1ms
- **Vshunt Conversion Time**: 1.1ms
- **Mode**: Continuous, both shunt and bus voltage

Example: `0x4527` (same as INA226 #1)

### PCB Layout (INA226 #2)

#### Voltage Divider Placement
- R1, R2 close to INA226 #2 (within 30mm)
- R1 rated for high voltage (100kΩ, 0.25W, ≥250V rating or use 2× 50kΩ in series)
- Clearance: ≥3mm between R1 and low voltage traces
- Decoupling: 100nF ceramic on Vbus node (filtered divider output)

#### Kelvin Connection (same as INA226 #1)
```
OUT+ ──[wide 1.5mm trace]──┬──[Shunt 10mΩ]──┬──[wide 1.5mm trace]──> BMS/LED Output
                           │                 │
                        (power pads)      (power pads)
                           │                 │
                        ┌──┘                 └──┐
                        │ (sense pads)          │
                        │                       │
                   [thin 0.2mm trace]      [thin 0.2mm trace]
                        │                       │
                   SHUNT_OUT_P              SHUNT_OUT_N
                        │                       │
                        └────> INA226 V+        │
                               INA226 V- <──────┘
```

---

## I²C Bus Configuration

### Bus Topology

```
ESP32-C6 (GPIO8/9)
    │
    ├── SDA ──┬── 2.2kΩ pull-up to 3.3V
    │         ├── INA226 #1 (0x40)
    │         ├── INA226 #2 (0x41)
    │         └── (BMS if I²C interface, e.g., 0x55)
    │
    └── SCL ──┴── 2.2kΩ pull-up to 3.3V
```

### Pull-Up Resistors

- **Value**: 2.2kΩ (suitable for 400kHz Fast Mode I²C)
- **Connection**: SDA and SCL to VCC_3V3 (3.3V rail)
- **Location**: One set of pull-ups near ESP32-C6, not duplicated at each device

**Calculation**:
```
For 400kHz I²C, maximum bus capacitance ≈ 400pF
Rise time target: 300ns (Fast Mode spec: 300ns max)
R_min = t_rise / (0.8473 × C_bus) = 300e-9 / (0.8473 × 400e-12) = 885Ω
R_typical = 2.2kΩ (provides margin)
```

### I²C Speed

- **Standard Mode**: 100kHz (not recommended, too slow for real-time MPPT)
- **Fast Mode**: 400kHz (recommended, good balance of speed and reliability)
- **High-Speed Mode**: 2.94MHz (possible but not necessary, requires special configuration)

**Firmware Configuration** (ESP-IDF):
```c
i2c_config_t conf = {
    .mode = I2C_MODE_MASTER,
    .sda_io_num = GPIO_NUM_8,
    .scl_io_num = GPIO_NUM_9,
    .sda_pullup_en = GPIO_PULLUP_DISABLE,  // External pull-ups
    .scl_pullup_en = GPIO_PULLUP_DISABLE,  // External pull-ups
    .master.clk_speed = 400000,  // 400kHz
};
```

---

## Firmware Integration

### INA226 Register Map (Summary)

| Address | Register | R/W | Description |
|---------|----------|-----|-------------|
| 0x00 | Configuration | R/W | Averaging, conversion times, mode |
| 0x01 | Shunt Voltage | R | Signed 16-bit, 2.5µV/LSB |
| 0x02 | Bus Voltage | R | Unsigned 16-bit, 1.25mV/LSB |
| 0x03 | Power | R | Unsigned 16-bit, 25×Current_LSB (W) |
| 0x04 | Current | R | Signed 16-bit, Current_LSB (A) |
| 0x05 | Calibration | R/W | Calibration value for current/power |
| 0x06 | Mask/Enable | R/W | Alert configuration |
| 0x07 | Alert Limit | R/W | Threshold for alerts |
| 0xFE | Manufacturer ID | R | 0x5449 (TI) |
| 0xFF | Die ID | R | 0x2260 (INA226) |

### Example Firmware Code (ESP-IDF)

```c
#include "driver/i2c.h"

#define INA226_ADDR_PV    0x40
#define INA226_ADDR_OUT   0x41
#define INA226_REG_CONFIG 0x00
#define INA226_REG_SHUNT  0x01
#define INA226_REG_BUS    0x02
#define INA226_REG_POWER  0x03
#define INA226_REG_CURRENT 0x04
#define INA226_REG_CAL    0x05

void ina226_write_reg(uint8_t addr, uint8_t reg, uint16_t value) {
    uint8_t data[3] = {reg, (value >> 8) & 0xFF, value & 0xFF};
    i2c_master_write_to_device(I2C_NUM_0, addr, data, 3, 1000 / portTICK_PERIOD_MS);
}

uint16_t ina226_read_reg(uint8_t addr, uint8_t reg) {
    uint8_t data[2];
    i2c_master_write_read_device(I2C_NUM_0, addr, &reg, 1, data, 2, 1000 / portTICK_PERIOD_MS);
    return (data[0] << 8) | data[1];
}

void ina226_init(uint8_t addr, uint16_t cal_value) {
    ina226_write_reg(addr, INA226_REG_CONFIG, 0x4527);  // AVG=16, 1.1ms, continuous
    ina226_write_reg(addr, INA226_REG_CAL, cal_value);
}

float ina226_read_voltage(uint8_t addr, bool is_output) {
    uint16_t raw = ina226_read_reg(addr, INA226_REG_BUS);
    float voltage = raw * 0.00125;  // 1.25mV/LSB
    if (is_output) voltage *= 5.0;  // Apply divider ratio for output voltage
    return voltage;
}

float ina226_read_current(uint8_t addr, float current_lsb) {
    int16_t raw = (int16_t)ina226_read_reg(addr, INA226_REG_CURRENT);
    return raw * current_lsb;
}

// In main.c or adc.c:
void sensors_init(void) {
    ina226_init(INA226_ADDR_PV, 0x4193);   // CAL for 10A max, 1mΩ shunt
    ina226_init(INA226_ADDR_OUT, 0x15DB);  // CAL for 3A max, 10mΩ shunt
}

void read_pv_measurements(float *vpv, float *ipv, float *ppv) {
    *vpv = ina226_read_voltage(INA226_ADDR_PV, false);
    *ipv = ina226_read_current(INA226_ADDR_PV, 305e-6);  // 305µA/LSB
    *ppv = (*vpv) * (*ipv);
}

void read_output_measurements(float *vout, float *iout, float *pout) {
    *vout = ina226_read_voltage(INA226_ADDR_OUT, true);  // Apply 5:1 divider
    *iout = ina226_read_current(INA226_ADDR_OUT, 91.5e-6);  // 91.5µA/LSB
    *pout = (*vout) * (*iout);
}
```

### Integration with MPPT Algorithm

In `mppt_pno.c` or main loop:
```c
float vpv, ipv, ppv;
read_pv_measurements(&vpv, &ipv, &ppv);

float duty = mppt_pno_update(&pno, vpv, ipv);  // Existing P&O algorithm
set_pwm_duty_ticks(duty_to_ticks(duty));
```

---

## BOM for Sensor Interface

| Ref Des | Part | Value | Package | Qty | Supplier Part # | Notes |
|---------|------|-------|---------|-----|-----------------|-------|
| U3 | INA226 | - | VSSOP-10 | 1 | TI INA226AIDGSR | PV side, addr 0x40 |
| U4 | INA226 | - | VSSOP-10 | 1 | TI INA226AIDGSR | Output side, addr 0x41 |
| R_SHUNT_PV | Shunt | 1mΩ, 3W | 3921 | 1 | Vishay WSLP3921L1000FEA | 4-terminal |
| R_SHUNT_OUT | Shunt | 10mΩ, 1W | 2512 | 1 | Vishay WSLP2512R0100FEA | 4-terminal |
| R1 | Resistor | 100kΩ, 1% | 0805 | 1 | Vishay CRCW0805100KFKEA | HV divider |
| R2 | Resistor | 25kΩ, 1% | 0805 | 1 | Vishay CRCW080525K0FKEA | HV divider |
| R_SDA | Resistor | 2.2kΩ | 0603 | 1 | Yageo RC0603FR-072K2L | I²C pull-up |
| R_SCL | Resistor | 2.2kΩ | 0603 | 1 | Yageo RC0603FR-072K2L | I²C pull-up |
| C_VS_U3 | Capacitor | 100nF, 50V, X7R | 0603 | 1 | Murata GRM188R71H104KA93D | Decoupling |
| C_VS_U4 | Capacitor | 100nF, 50V, X7R | 0603 | 1 | Murata GRM188R71H104KA93D | Decoupling |
| C_DIV | Capacitor | 100nF, 50V, X7R | 0603 | 1 | Murata GRM188R71H104KA93D | Divider filter |

---

## Testing and Calibration

### Functional Test

1. **I²C Communication Test**:
   - Read Manufacturer ID (0xFE): Should return 0x5449
   - Read Die ID (0xFF): Should return 0x2260
   
2. **Voltage Measurement Test**:
   - Apply known voltage to PV input (e.g., 24V DC)
   - Read INA226 #1 bus voltage, verify ±1% accuracy
   - Apply known voltage to output (e.g., 96V DC via lab supply)
   - Read INA226 #2 bus voltage (should be ~19.2V), multiply by 5, verify ±2% accuracy

3. **Current Measurement Test**:
   - Apply known current through PV shunt (e.g., 5A via electronic load)
   - Read INA226 #1 current register, verify ±1.5% accuracy
   - Repeat for output shunt (e.g., 1.5A)

4. **Power Calculation Test**:
   - Apply known V and I (e.g., 24V, 5A = 120W)
   - Read power register, verify within ±2% of expected

### Calibration Procedure

If higher accuracy is required:

1. Measure actual shunt resistance with precision meter (Kelvin connection)
2. Update CAL register calculation with measured resistance
3. Measure voltage divider ratio (R1 + R2 actual values)
4. Update firmware divider multiplier for INA226 #2

---

## Design Checklist

- [ ] INA226 #1 I²C address set to 0x40 (A0=GND, A1=GND)
- [ ] INA226 #2 I²C address set to 0x41 (A0=VCC, A1=GND)
- [ ] Shunt resistors are 4-terminal Kelvin type
- [ ] Kelvin traces routed correctly (power and sense separated)
- [ ] I²C pull-ups present (2.2kΩ to 3.3V on SDA and SCL)
- [ ] Decoupling capacitors (100nF) on each INA226 VS pin
- [ ] Voltage divider for INA226 #2 (5:1 ratio, 100kΩ + 25kΩ)
- [ ] High voltage clearance (≥3mm) around R1 and output shunt
- [ ] Firmware calibration values calculated and programmed
- [ ] Voltage and current measurements tested and verified

---

## References

- INA226 Datasheet (Texas Instruments, SBOS547)
- INA226 EVM User's Guide (Texas Instruments, SBOU129)
- ESP32-C6 I²C Driver Documentation (ESP-IDF)
- Application Note: "Kelvin Sensing Improves Low-Side Current Measurements" (TI SLVA831)

## Revision History

- **v1.0** (2025-12-15): Initial sensor interface specification for INA226 dual monitoring
