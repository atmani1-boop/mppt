# Thread FDTRGB Status LED

## Overview

The Thread FDTRGB LED is an addressable RGB LED (WS2812B) that provides visual status indication using a 6-state color coding system:

- **F (Fault)**: Red blinking
- **D (Disconnect)**: Orange solid
- **T (Thread)**: Green (connected) / Blue (joining)
- **R (Running)**: Green pulsing
- **G (Good)**: Solid green
- **B (Battery)**: Color gradient by SOC

## LED Component

### WS2812B Addressable RGB LED

**Specifications**:
- **Type**: 5050 SMD RGB LED with integrated WS2811 driver
- **Supply Voltage**: 5V ±0.5V
- **Current**: 60mA max (20mA per color at full brightness)
- **Data Rate**: 800kHz (1.25µs per bit)
- **Protocol**: Serial data (GRB format, 24-bit color)
- **Package**: 5050 SMD (5×5mm)

**Pinout**:
| Pin | Name | Function |
|-----|------|----------|
| 1 | VDD | 5V power |
| 2 | DOUT | Data output (daisy-chain) |
| 3 | GND | Ground |
| 4 | DIN | Data input |

**Supplier**: Adafruit #1655, Sparkfun COM-12986, or generic WS2812B

### Alternative: APA102 (If SPI Preferred)

**Advantages**: Separate clock line (less timing-critical than WS2812B)

**Pinout**: VDD, GND, DATA, CLOCK

---

## Connection to ESP32-C6

**Circuit**:
```
ESP32-C6 GPIO5 (3.3V logic) ──[Level Shifter 74HCT125]──> WS2812B DIN (5V logic)
                                                     
VCC_5V ──────────────────────────────────────────────> WS2812B VDD
GND ──────────────────────────────────────────────────> WS2812B GND
```

### Level Shifter (Optional but Recommended)

**IC**: 74HCT125 (quad buffer, 3.3V input tolerant, 5V output)

**Connection**:
```
ESP32-C6 GPIO5 ──> 74HCT125 input (pin 2)
74HCT125 output (pin 3) ──> WS2812B DIN
74HCT125 VCC ──> 5V
74HCT125 GND ──> GND
74HCT125 OE (pin 1) ──> GND (enable output)
```

**Alternative**: Some WS2812B variants accept 3.3V logic when powered by 5V. Test your specific LED model.

---

## Color Coding Scheme

### State 1: F (Fault)
**Color**: Red (255, 0, 0)
**Pattern**: Blinking 500ms ON, 500ms OFF
**Conditions**: OVP, OCP, thermal fault, BMS fault, communication error

**Firmware**:
```c
if (system_fault != FAULT_NONE) {
    uint32_t now = esp_timer_get_time() / 1000;  // ms
    if ((now / 500) % 2 == 0) {
        set_led_color(255, 0, 0);  // Red
    } else {
        set_led_color(0, 0, 0);    // Off
    }
}
```

### State 2: D (Disconnect)
**Color**: Orange (255, 165, 0)
**Pattern**: Solid
**Conditions**: PV voltage < 10V (no sunlight or panel disconnected)

**Firmware**:
```c
if (vpv < 10.0) {
    set_led_color(255, 165, 0);  // Orange
}
```

### State 3: T (Thread Network Status)
**Color**: Green (connected) or Blue (joining)
**Pattern**: Solid
**Conditions**: 
- Green: Connected to Thread network
- Blue: Joining or commissioning

**Firmware**:
```c
if (thread_state == THREAD_JOINING) {
    set_led_color(0, 0, 255);  // Blue
} else if (thread_state == THREAD_CONNECTED) {
    set_led_color(0, 255, 0);  // Green
}
```

### State 4: R (Running MPPT)
**Color**: Green (0, 255, 0)
**Pattern**: Pulsing (breathe effect, 1-second cycle)
**Conditions**: MPPT actively tracking power point

**Firmware**:
```c
if (mppt_running && vpv > 10.0) {
    uint32_t now = esp_timer_get_time() / 1000;  // ms
    float brightness = (sin(now * 2 * M_PI / 1000.0) + 1.0) / 2.0;  // 0.0–1.0
    uint8_t green = (uint8_t)(255 * brightness);
    set_led_color(0, green, 0);  // Green pulse
}
```

### State 5: G (Good / Charging)
**Color**: Green (0, 255, 0)
**Pattern**: Solid
**Conditions**: Charging BMS, no faults, stable operation

**Firmware**:
```c
if (charging && system_fault == FAULT_NONE) {
    set_led_color(0, 255, 0);  // Solid green
}
```

### State 6: B (Battery SOC)
**Color**: Gradient based on battery state of charge
**Pattern**: Solid
**Conditions**: Display battery level when not actively charging

**Color Map**:
- SOC < 20%: Red (255, 0, 0)
- SOC 20–40%: Orange (255, 165, 0)
- SOC 40–60%: Yellow (255, 255, 0)
- SOC 60–80%: Yellow-Green (128, 255, 0)
- SOC > 80%: Green (0, 255, 0)

**Firmware**:
```c
void set_battery_soc_color(float soc) {
    uint8_t r, g, b;
    
    if (soc < 20) {
        r = 255; g = 0; b = 0;  // Red
    } else if (soc < 40) {
        r = 255; g = 165; b = 0;  // Orange
    } else if (soc < 60) {
        r = 255; g = 255; b = 0;  // Yellow
    } else if (soc < 80) {
        r = 128; g = 255; b = 0;  // Yellow-Green
    } else {
        r = 0; g = 255; b = 0;  // Green
    }
    
    set_led_color(r, g, b);
}
```

---

## WS2812B Driver Implementation (ESP32-C6 RMT)

### RMT Peripheral Configuration

**Timing**:
- 0-bit: 400ns high, 850ns low (total 1.25µs)
- 1-bit: 800ns high, 450ns low (total 1.25µs)
- Reset: >50µs low

**Firmware**:
```c
#include "driver/rmt_tx.h"

static rmt_channel_handle_t led_channel = NULL;
static rmt_encoder_handle_t led_encoder = NULL;

void ws2812b_init(void) {
    rmt_tx_channel_config_t tx_config = {
        .gpio_num = GPIO_NUM_5,
        .clk_src = RMT_CLK_SRC_DEFAULT,
        .resolution_hz = 10 * 1000 * 1000,  // 10MHz
        .mem_block_symbols = 64,
        .trans_queue_depth = 4,
    };
    ESP_ERROR_CHECK(rmt_new_tx_channel(&tx_config, &led_channel));
    
    // Create encoder for WS2812B (GRB format)
    led_strip_encoder_config_t encoder_config = {
        .resolution = tx_config.resolution_hz,
    };
    ESP_ERROR_CHECK(rmt_new_led_strip_encoder(&encoder_config, &led_encoder));
    
    ESP_ERROR_CHECK(rmt_enable(led_channel));
    ESP_LOGI(TAG, "WS2812B initialized on GPIO5");
}

void set_led_color(uint8_t r, uint8_t g, uint8_t b) {
    uint8_t led_data[3] = {g, r, b};  // WS2812B uses GRB format
    
    rmt_transmit_config_t tx_conf = {
        .loop_count = 0,
    };
    ESP_ERROR_CHECK(rmt_transmit(led_channel, led_encoder, led_data, sizeof(led_data), &tx_conf));
}
```

---

## State Priority

When multiple conditions are true, use this priority order (highest to lowest):

1. **Fault** (F): Any fault condition overrides all other states
2. **Disconnect** (D): PV disconnected
3. **Thread Joining** (T Blue): Commissioning in progress
4. **Running** (R): MPPT actively tracking
5. **Charging** (G): Stable charging
6. **Battery SOC** (B): Display battery level

**Firmware**:
```c
void update_status_led(void) {
    if (system_fault != FAULT_NONE) {
        // State F: Fault (red blink)
        led_blink_red();
    } else if (vpv < 10.0) {
        // State D: Disconnect (orange)
        set_led_color(255, 165, 0);
    } else if (thread_state == THREAD_JOINING) {
        // State T: Thread joining (blue)
        set_led_color(0, 0, 255);
    } else if (mppt_running) {
        // State R: MPPT running (green pulse)
        led_pulse_green();
    } else if (charging) {
        // State G: Charging (solid green)
        set_led_color(0, 255, 0);
    } else {
        // State B: Battery SOC color
        set_battery_soc_color(soc);
    }
}
```

---

## Power Consumption

**Maximum**: 60mA @ 5V = 0.3W (all colors at full brightness)

**Typical**: 20mA @ 5V = 0.1W (single color at full brightness)

**Recommendation**: Limit brightness to 50% to reduce power consumption and extend LED life.

---

## BOM for Thread FDTRGB LED

| Ref Des | Part | Value | Package | Qty | Supplier Part # | Notes |
|---------|------|-------|---------|-----|-----------------|-------|
| LED1 | WS2812B | RGB LED | 5050 SMD | 1 | Adafruit #1655 | Addressable RGB |
| U_LS | 74HCT125 | Buffer | SOIC-14 | 1 | TI SN74HCT125DR | Level shifter (optional) |
| C_LED | Capacitor | 100µF, 6.3V | Electrolytic | 1 | Panasonic EEE-FK0J101P | Power decoupling near LED |

---

## Design Checklist

- [ ] WS2812B LED selected and placed on PCB
- [ ] GPIO5 routed to LED DIN (via level shifter if needed)
- [ ] 5V power rail connected to LED VDD
- [ ] 100µF decoupling capacitor near LED
- [ ] RMT peripheral configured in firmware
- [ ] 6-state color coding implemented
- [ ] State priority logic tested
- [ ] LED brightness limited to 50% for power savings

---

## References

- WS2812B Datasheet (Worldsemi)
- ESP32-C6 RMT Peripheral Documentation (ESP-IDF)
- 74HCT125 Datasheet (Texas Instruments)

## Revision History

- **v1.0** (2025-12-15): Initial Thread FDTRGB LED specification
