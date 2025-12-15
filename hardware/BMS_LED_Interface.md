# BMS and LED Driver Interface

## Overview

This document specifies the interface between the ESP32-C6 MPPT controller and two key subsystems:
1. **BMS (Battery Management System)**: Controls charging/discharging of 1S 304Ah LiFePO4 cell
2. **LED Driver**: PWM dimming control for LED lighting output with DALI+ IP integration

## Part 1: BMS Interface (1S 304Ah LiFePO4)

### Battery Specifications

- **Cell Type**: LiFePO4 (Lithium Iron Phosphate), single cell (1S)
- **Nominal Voltage**: 3.2V
- **Capacity**: 304Ah (973Wh nominal energy)
- **Voltage Range**: 
  - Minimum (cutoff): 2.5V
  - Nominal: 3.2V
  - Maximum (charged): 3.65V
- **Charge Current**: Max 1C (304A) typical, or 0.5C (152A) conservative
- **Discharge Current**: Max 1C (304A) continuous

### External Voltage Booster

The BMS includes an **external DC-DC boost converter** to step up the 1S cell voltage (2.5–3.65V) to the system output voltage (48–129V).

**Booster Specifications** (integrated in BMS or external module):
- **Input**: 2.5–3.65V (from LiFePO4 cell)
- **Output**: 48–129V (adjustable or fixed, controlled by ESP32-C6 or BMS)
- **Power**: Up to 1kW (304Ah × 3.2V = 973Wh storage)
- **Efficiency**: Typically 92–96%

**Note**: Clarify with BMS supplier if boost converter is integrated or requires separate module.

### BMS Communication Interface

The BMS communicates with ESP32-C6 for:
- **State of Charge (SOC)**: Battery % remaining (0–100%)
- **Cell Voltage**: Real-time voltage of LiFePO4 cell
- **Cell Temperature**: Temperature monitoring
- **Fault Status**: Overvoltage, undervoltage, overcurrent, overtemperature faults
- **Charge/Discharge Control**: Enable/disable signals from ESP32-C6

**Protocol Options** (choose based on BMS model):

#### Option 1: I²C Interface
- **Signals**: SDA (GPIO8), SCL (GPIO9) - shared with INA226 sensors
- **Address**: Unique I²C address (e.g., 0x55, avoid conflict with INA226 0x40/0x41)
- **Data Rate**: 100kHz or 400kHz
- **Registers**: SOC, voltage, temperature, fault flags (BMS-specific register map)

**Example I²C BMS**:
- Victron SmartShunt (I²C/UART)
- Texas Instruments BQ34Z100 series

#### Option 2: UART Interface
- **Signals**: TX (GPIO17), RX (GPIO16)
- **Baud Rate**: 9600, 19200, or 115200 (BMS dependent)
- **Protocol**: ASCII commands or binary packets (BMS-specific)
- **Data**: SOC, voltage, current, temperature, faults

**Example UART BMS**:
- ANT BMS (UART protocol)
- Daly BMS (UART/CAN/RS485 variants)

#### Option 3: CAN Bus Interface
- **Signals**: CAN_TX (GPIO17), CAN_RX (GPIO16) via CAN transceiver (MCP2551 or TJA1050)
- **Baud Rate**: 250kbps or 500kbps (CAN standard)
- **Protocol**: CANopen or proprietary BMS protocol
- **Advantages**: Robust, suitable for automotive/industrial environments

**CAN Transceiver Circuit**:
```
ESP32-C6 GPIO17 (TX) ──> MCP2551 TXD
ESP32-C6 GPIO16 (RX) <── MCP2551 RXD
                         MCP2551 CANH ──> BMS CAN_H
                         MCP2551 CANL ──> BMS CAN_L
                         MCP2551 VCC ──── 5V
                         MCP2551 GND ──── GND
```

**Recommended**: Start with **UART** for simplicity, or **I²C** if BMS supports it (shared bus with sensors).

### Charge/Discharge Control Signals

#### Charge Enable (GPIO18)
- **Direction**: ESP32-C6 output → BMS input
- **Logic**: Active high (3.3V = enable charging)
- **Function**: Allows MPPT to charge battery when PV power is available
- **Control**: Based on SOC, cell voltage, temperature

**Circuit**:
```
ESP32-C6 GPIO18 ──┬──[R_limit 1kΩ]──┬──> BMS_CHG_EN input
                  │                  │
                  │            [Opto-isolator or level shifter if BMS requires 5V/12V]
                  │                  │
                  └──────────────────┴──> BMS charge enable
```

**Firmware Logic**:
```c
if (soc < 95 && cell_voltage < 3.60 && cell_temp < 45) {
    gpio_set_level(GPIO_BMS_CHG_EN, 1);  // Enable charging
} else {
    gpio_set_level(GPIO_BMS_CHG_EN, 0);  // Disable charging
}
```

#### Discharge Enable (GPIO19)
- **Direction**: ESP32-C6 output → BMS input
- **Logic**: Active high (3.3V = enable discharging)
- **Function**: Allows battery to power LED driver or other loads
- **Control**: Based on SOC, cell voltage, load demand

**Circuit**: Similar to charge enable

**Firmware Logic**:
```c
if (soc > 10 && cell_voltage > 2.8) {
    gpio_set_level(GPIO_BMS_DISCHG_EN, 1);  // Enable discharging
} else {
    gpio_set_level(GPIO_BMS_DISCHG_EN, 0);  // Disable discharging
}
```

### Fault Detection (GPIO7)

- **Direction**: BMS output → ESP32-C6 input (GPIO7)
- **Logic**: Active low (0V = fault detected, 3.3V = normal)
- **Function**: BMS signals fault conditions (overvoltage, undervoltage, overtemperature, etc.)
- **Response**: ESP32-C6 disables charging/discharging, logs fault, updates status LED

**Circuit**:
```
BMS_FAULT_OUT ──┬──[R_pullup 10kΩ to 3.3V]──┬──> ESP32-C6 GPIO7 (input)
                │                            │
          [Opto-isolator if isolation needed]
                │                            │
                └────────────────────────────┘
```

**Firmware**:
```c
void bms_fault_isr(void *arg) {
    ESP_LOGE(TAG, "BMS fault detected!");
    gpio_set_level(GPIO_BMS_CHG_EN, 0);   // Stop charging
    gpio_set_level(GPIO_BMS_DISCHG_EN, 0); // Stop discharging
    duty = 0.0;  // Shutdown MPPT
    system_fault |= FAULT_BMS;
    // Update Thread FDTRGB LED to red blink
}

// In app_main():
gpio_config_t io_conf = {
    .pin_bit_mask = (1ULL << GPIO_BMS_FAULT),
    .mode = GPIO_MODE_INPUT,
    .pull_up_en = GPIO_PULLUP_ENABLE,
    .intr_type = GPIO_INTR_NEGEDGE,  // Trigger on falling edge (fault)
};
gpio_config(&io_conf);
gpio_isr_handler_add(GPIO_BMS_FAULT, bms_fault_isr, NULL);
```

### Temperature Sensing

**Option 1: BMS Internal Temperature Sensor** (read via I²C/UART/CAN)
- Most BMS modules include temperature sensor(s) on cell or BMS board
- Read temperature via communication protocol

**Option 2: External NTC Thermistor** (if BMS doesn't provide temperature)
- **Sensor**: 10kΩ NTC mounted on cell or BMS case
- **Circuit**: Voltage divider → ESP32-C6 ADC (GPIO11 or separate ADC channel)
- See Protection_Circuits.md for NTC circuit details

### BMS Connector

**Connector Type**: Phoenix Contact or XT60/Anderson Powerpole for high voltage output

**Pinout Example** (if BMS has separate connector):
| Pin | Signal | Description |
|-----|--------|-------------|
| 1 | OUT+ | High voltage output (48–129V) |
| 2 | OUT- | High voltage return (GND) |
| 3 | CHG_EN | Charge enable input (3.3V or 5V logic) |
| 4 | DISCHG_EN | Discharge enable input |
| 5 | FAULT | Fault output (active low) |
| 6 | COMM_TX | UART TX or CAN_H or SDA |
| 7 | COMM_RX | UART RX or CAN_L or SCL |
| 8 | GND | Signal ground |
| 9 | TEMP | Temperature sensor output (optional) |

---

## Part 2: LED Driver Interface

### LED Driver Output

**Output Voltage**: 48–129V (from BMS boost converter or boost output)

**LED Driver Type**: External constant-current LED driver module (e.g., Mean Well HLG series, Inventronics)

**Power**: Up to 150W (limited by MPPT capacity)

### Dimming Control (0–10V Analog or PWM)

Most LED drivers accept **0–10V dimming input**:
- 0V = minimum brightness (off or 10% depending on driver)
- 10V = maximum brightness (100%)

**ESP32-C6 Output**: GPIO6 provides PWM signal (0–3.3V)

**Conversion Circuit** (PWM → 0–10V):

#### Option 1: PWM + RC Filter + Op-Amp Buffer + Voltage Scaler

**Circuit**:
```
ESP32-C6 GPIO6 (PWM) ──┬──[R1 1kΩ]──┬──[C1 10µF]──┬──> Op-Amp (LM358) (+) input
                       │             │            │
                       │             └─[R2 10kΩ]──┴──> GND
                       │                           │
                       │                       Op-Amp output ──┬──[R3 20kΩ]──┬──> 0–10V out to LED driver
                       │                                       │             │
                       │                                       └──[R4 10kΩ]──┴──> GND
                       │                                                      │
                       └───────────────────────────────────────────────────────┴──> Power: 12V to Op-Amp VCC
```

**Components**:
- R1, C1: RC low-pass filter (PWM → analog voltage)
- R2: Pull-down for stability
- LM358: Dual op-amp (one channel used as buffer/amplifier)
- R3, R4: Voltage divider for scaling (3.3V → 10V requires gain of ~3)

**Alternative Gain Stage**: Non-inverting amplifier configuration:
```
Gain = 1 + (R_feedback / R_input)
For 3.3V → 10V: Gain = 10/3.3 = 3.03
Select R_input = 10kΩ, R_feedback = 20kΩ → Gain = 1 + 20/10 = 3
```

**Op-Amp Supply**: 12V from VCC_12V rail (same as gate driver supply)

#### Option 2: I²C DAC (MCP4725) + Op-Amp Scaler

**Circuit**:
```
ESP32-C6 I²C (SDA/SCL) ──> MCP4725 DAC ──> 0–3.3V analog ──> Op-Amp scaler ──> 0–10V out
```

**Advantages**:
- True analog output (no PWM ripple)
- Easier firmware control (I²C write to set voltage)

**Disadvantages**:
- Requires I²C communication (shared bus with sensors)
- Additional component cost

**MCP4725 Configuration**:
```c
// Set LED brightness (0–100%)
void set_led_brightness(uint8_t percent) {
    uint16_t dac_value = (percent * 4095) / 100;  // 12-bit DAC
    i2c_master_write_to_device(I2C_NUM_0, MCP4725_ADDR, 
                               &dac_value, 2, 1000 / portTICK_PERIOD_MS);
}
```

**Recommended**: Use **Option 1** (PWM + op-amp) for simplicity, or **Option 2** (I²C DAC) for higher precision.

### DALI+ over IP Integration

See dedicated document: `DALI_Plus_IP.md`

**Summary**:
- DALI+ control commands received via TCP port 55825 over WiFi/Thread
- ESP32-C6 translates DALI commands to 0–10V dimming voltage
- Synchronizes with Matter Level Control cluster

**Example**:
```
DALI Command: "SET LEVEL 128" (50% brightness)
ESP32-C6 Action: Set GPIO6 PWM to 50% duty cycle → 1.65V → scaled to 5V for LED driver
```

### LED Driver Connector

**Connector Type**: Phoenix Contact 2-pin (5.08mm pitch) or dedicated LED driver connector

**Pinout**:
| Pin | Signal | Description |
|-----|--------|-------------|
| 1 | DIM+ | 0–10V dimming input (positive) |
| 2 | DIM- | Dimming ground (LED driver GND) |

**Note**: Some LED drivers have separate connectors for:
- **Power Input**: High voltage (48–129V)
- **Dimming Control**: 0–10V or PWM
- **LED Output**: Constant current to LED array

Refer to specific LED driver datasheet (e.g., Mean Well HLG-150H-48A with dimming).

### Isolation (Optional)

If LED driver is on a separate high voltage rail or requires isolation:

**Opto-Isolator Circuit**:
```
ESP32-C6 GPIO6 (PWM) ──[R_LED 220Ω]──┬──> Opto (PC817) LED anode
                                     │
                                  LED cathode ──> GND
                                     
                            Opto transistor collector ──[R_pullup 10kΩ to 12V]──> 12V
                            Opto transistor emitter ──> GND (isolated)
                                     │
                                     └──> Isolated PWM signal to 0–10V converter on LED driver side
```

**Advantages**: Protects ESP32-C6 from LED driver faults

**Disadvantages**: Adds complexity and component count

**Recommendation**: Use isolation if LED driver is on separate power domain or if EMI/safety requires it.

---

## Firmware Integration

### BMS Communication Example (UART)

```c
#include "driver/uart.h"

#define BMS_UART_NUM UART_NUM_1
#define BMS_TX_PIN GPIO_NUM_17
#define BMS_RX_PIN GPIO_NUM_16

void bms_uart_init(void) {
    uart_config_t uart_config = {
        .baud_rate = 9600,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
    };
    uart_param_config(BMS_UART_NUM, &uart_config);
    uart_set_pin(BMS_UART_NUM, BMS_TX_PIN, BMS_RX_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    uart_driver_install(BMS_UART_NUM, 1024, 1024, 0, NULL, 0);
}

float bms_read_soc(void) {
    uint8_t cmd[] = {0xA5, 0x40, 0x90, 0x08, 0x77};  // Example BMS command for SOC
    uart_write_bytes(BMS_UART_NUM, (const char *)cmd, sizeof(cmd));
    
    uint8_t response[13];
    int len = uart_read_bytes(BMS_UART_NUM, response, sizeof(response), 100 / portTICK_PERIOD_MS);
    
    if (len > 0 && response[0] == 0xA5) {
        uint16_t soc_raw = (response[4] << 8) | response[5];
        return (float)soc_raw / 10.0;  // SOC in percentage
    }
    return -1.0;  // Error
}
```

### LED Dimming Example (PWM)

```c
#include "driver/ledc.h"

#define LED_DIM_GPIO GPIO_NUM_6
#define LED_DIM_CHANNEL LEDC_CHANNEL_1
#define LED_DIM_FREQ 1000  // 1kHz PWM for 0–10V converter

void led_dimming_init(void) {
    ledc_timer_config_t timer = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .duty_resolution = LEDC_TIMER_10_BIT,  // 1024 steps
        .timer_num = LEDC_TIMER_1,
        .freq_hz = LED_DIM_FREQ,
        .clk_cfg = LEDC_AUTO_CLK
    };
    ledc_timer_config(&timer);
    
    ledc_channel_config_t channel = {
        .gpio_num = LED_DIM_GPIO,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = LED_DIM_CHANNEL,
        .timer_sel = LEDC_TIMER_1,
        .duty = 0,
        .hpoint = 0
    };
    ledc_channel_config(&channel);
}

void set_led_brightness(uint8_t percent) {
    uint32_t duty = (percent * 1023) / 100;  // 0–100% → 0–1023
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LED_DIM_CHANNEL, duty);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LED_DIM_CHANNEL);
}
```

---

## BOM for BMS and LED Interface

| Ref Des | Part | Value | Package | Qty | Supplier Part # | Notes |
|---------|------|-------|---------|-----|-----------------|-------|
| U7 | Op-Amp | LM358 | SOIC-8 | 1 | TI LM358DR | For 0–10V conversion |
| R_DIM1 | Resistor | 1kΩ | 0603 | 1 | Yageo RC0603FR-071KL | PWM filter |
| R_DIM2 | Resistor | 10kΩ | 0603 | 1 | Yageo RC0603FR-0710KL | PWM filter |
| R_DIM3 | Resistor | 20kΩ | 0603 | 1 | Yageo RC0603FR-0720KL | Op-amp gain |
| R_DIM4 | Resistor | 10kΩ | 0603 | 1 | Yageo RC0603FR-0710KL | Op-amp gain |
| C_DIM | Capacitor | 10µF, X7R | 0805 | 1 | Murata GRM21BR71A106KA73L | PWM filter |
| U_CAN | CAN Transceiver | MCP2551 | SOIC-8 | 1 | Microchip MCP2551-I/SN | If CAN BMS |
| J_BMS | Connector | Phoenix 8-pin | 5.08mm | 1 | Phoenix MSTB 2,5/8-ST-5,08 | BMS interface |
| J_LED | Connector | Phoenix 2-pin | 5.08mm | 1 | Phoenix MSTB 2,5/2-ST-5,08 | LED dimming |

---

## Design Checklist

- [ ] BMS communication protocol selected (I²C, UART, or CAN)
- [ ] Charge/discharge enable signals routed from ESP32-C6 to BMS
- [ ] BMS fault signal routed to ESP32-C6 with interrupt configuration
- [ ] LED dimming circuit designed (PWM → 0–10V conversion)
- [ ] Op-amp circuit tested for 0–10V output range
- [ ] BMS connector selected and pinout documented
- [ ] LED driver connector selected (Phoenix or equivalent)
- [ ] Firmware integration for BMS communication implemented
- [ ] Firmware integration for LED PWM dimming implemented
- [ ] DALI+ over IP integration planned (see DALI_Plus_IP.md)

---

## References

- LM358 Op-Amp Datasheet (Texas Instruments)
- MCP4725 I²C DAC Datasheet (Microchip)
- MCP2551 CAN Transceiver Datasheet (Microchip)
- Mean Well HLG Series LED Driver Datasheets
- ESP32-C6 UART and LEDC Documentation (ESP-IDF)
- IEC 62386-104: DALI+ over IP Specification

## Revision History

- **v1.0** (2025-12-15): Initial BMS and LED driver interface specification
