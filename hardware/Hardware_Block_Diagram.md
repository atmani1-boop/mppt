# Hardware Block Diagram - MPPT 150W Boost Converter

## System Block Diagram (ASCII)

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                        MPPT 150W Boost Converter System                      │
│                      ESP32-C6 + Matter/Thread/WiFi/BLE                       │
└─────────────────────────────────────────────────────────────────────────────┘

┌──────────────┐                                                    ┌───────────┐
│  PV Panel    │                                                    │ BMS       │
│  18-40V      │                                                    │ 1S 304Ah  │
│  Max 10A     │                                                    │ LiFePO4   │
│  (150W)      │                                                    │ 3.2V      │
└──────┬───────┘                                                    │ Boost to  │
       │ PV+/PV-                                                    │ 48-129V   │
       │                                                            └─────┬─────┘
       ▼                                                                  │
┌─────────────┐                                                          │
│ J1: MC4 or  │                                                          │
│ Phoenix     │                                                          │
│ Connector   │                                                          │
└──────┬──────┘                                                          │
       │                                                                  │
       ▼                                                                  │
┌──────────────┐                                                         │
│ F1: Fuse     │                                                         │
│ 15A (opt)    │                                                         │
└──────┬───────┘                                                         │
       │                                                                  │
       ▼                                                                  │
┌────────────────────┐                                                   │
│ Q3: Reverse        │                                                   │
│ Polarity Protection│                                                   │
│ P-MOSFET IRF4905   │                                                   │
└──────┬─────────────┘                                                   │
       │ PV_PROT+                                                        │
       ▼                                                                  │
┌────────────────────┐         ┌──────────────────────────────────┐     │
│ C_in: Input Caps   │         │      Boost Converter             │     │
│ 2×470µF + ceramics │         │   ┌─────────────────────┐        │     │
└──────┬─────────────┘         │   │  L1: Inductor       │        │     │
       │                       │   │  47-100µH, 17A      │        │     │
       ├──────────────────────>│   └─────────┬───────────┘        │     │
       │                       │             │ BOOST_SW           │     │
       │   ┌──────────────────>│   ┌─────────┴──────────┐         │     │
       │   │                   │   │ Q1: Low-Side MOSFET│         │     │
       │   │                   │   │ IRFB4115 (60V)     │         │     │
       │   │  INA226 #1 (PV)   │   └─────────┬──────────┘         │     │
       │   │  I²C 0x40         │             │                    │     │
       │   │  V: 0-40V         │   ┌─────────┴──────────┐         │     │
       │   │  I: 0-10A (1mΩ)   │   │ Q2: High-Side      │         │     │
       │   │                   │   │ MOSFET/Diode       │         │     │
       ├───┴───────────────────┤   │ IPP60R125CP or     │         │     │
       │ R_SHUNT_PV: 1mΩ      │   │ MBR40250G          │         │     │
       └───────────────────────┤   └─────────┬──────────┘         │     │
                               │             │ OUT+ (48-129V)     │     │
                               └─────────────┼────────────────────┘     │
                                             │                          │
                                             ▼                          │
                               ┌─────────────────────────┐              │
                               │ C_out: Output Caps      │              │
                               │ 2×220µF/160V + ceramics │              │
                               └─────────────┬───────────┘              │
                                             │                          │
                               ┌─────────────┴───────────┐              │
                               │ INA226 #2 (Output)      │              │
                               │ I²C 0x41                │              │
                               │ V: 48-129V (÷5 divider) │              │
                               │ I: 0-3A (10mΩ)          │              │
                               ├─────────────────────────┤              │
                               │ R_SHUNT_OUT: 10mΩ      │              │
                               └─────────────┬───────────┘              │
                                             │ OUT+                     │
                                             ├─────────────────────────>│
                                             │                      BMS │
                                             │ OUT-/GND                 │
                                             └─────────────────────────>│
                                                                         │
┌───────────────────────────────────────────────────────────────────────┤
│                                                               J2: XT60│
│                                                        High Voltage Out│
└─────────────────────────────────────────────────────────────────┬─────┘
                                                                  │
                                                                  ▼
                                                       ┌───────────────────┐
                                                       │ LED Driver        │
                                                       │ 48-129V Input     │
                                                       │ 0-10V Dimming     │
                                                       │ DALI+ IP Control  │
                                                       └───────────────────┘

┌─────────────────────────────────────────────────────────────────────────────┐
│                         Control and Communication                            │
└─────────────────────────────────────────────────────────────────────────────┘

┌──────────────────────┐
│ ESP32-C6-WROOM-1     │
│ RISC-V, WiFi6, BLE5  │
│ Thread 802.15.4      │
└────────┬─────────────┘
         │
         ├──> GPIO3: PWM ───────────────┐
         │                              ▼
         │                    ┌──────────────────┐
         │                    │ U2: IR2110       │
         │                    │ Gate Driver      │
         │                    └─────┬──────┬─────┘
         │                          │      │
         │                     HO───┴──>Q2 │
         │                     LO────────>Q1
         │
         ├──> GPIO8/9: I²C ────────┬───> INA226 #1 (PV, 0x40)
         │      SDA/SCL            └───> INA226 #2 (Out, 0x41)
         │                              (+ BMS if I²C)
         │
         ├──> GPIO5: RMT ──────────────> LED1: WS2812B (Thread FDTRGB)
         │      (WS2812B data)           Status LED (6-state color)
         │
         ├──> GPIO6: PWM ──────────────> LED Dimming (0-10V circuit)
         │      (LED dimming)            J6: Phoenix connector
         │
         ├──> GPIO16/17: UART ─────────> BMS Communication
         │      (or CAN/I²C)             J7: BMS Interface
         │
         ├──> GPIO18: CHG_EN ──────────> BMS Charge Enable
         ├──> GPIO19: DISCHG_EN ───────> BMS Discharge Enable
         ├──> GPIO7: FAULT_IN ─────────< BMS Fault Signal
         │
         ├──> GPIO10: OVP_LATCH ───────< LM393 Comparator (140V threshold)
         ├──> GPIO11: ADC ─────────────< NTC Thermistor (MOSFET temp)
         │
         ├──> GPIO12/13: USB ──────────> J3: USB-C (Programming/Debug)
         │      D+/D-                    ESP32-C6 internal USB-JTAG
         │
         └──> RF: Antenna ─────────────> J5: U.FL (2.4GHz Thread/WiFi/BLE)
                                         Or PCB trace antenna

┌──────────────────────────────────────────┐
│       Protection Circuits                │
└──────────────────────────────────────────┘

OUT+ (48-129V) ─┬─> Voltage Divider ───> LM393 (+) Comparator
                │   (470kΩ + 4.7kΩ)      V_ref (1.25V) ───> LM393 (-)
                │                        └─> OVP_LATCH (GPIO10)
                │
                └─> Thermal Shutdown:
                    NTC 10kΩ on Q1 ───> Voltage Divider ───> GPIO11 (ADC)
                    
                    Over-Current Protection:
                    INA226 software monitoring (10A PV, 3A Out)

┌──────────────────────────────────────────┐
│       Power Supply (3.3V/5V/12V)         │
└──────────────────────────────────────────┘

PV_PROT+ (18-40V) ───> Buck Converter (TPS54331) ───> VCC_5V (5V @ 500mA)
                                                      │
                                                      ├──> LDO ──> VCC_3V3 (3.3V @ 500mA)
                                                      │           ESP32-C6, ICs
                                                      │
                                                      ├──> WS2812B LED (5V)
                                                      │
                                                      └──> Op-Amp LM358 (for LED dimming)
                       
                       VCC_12V (12V @ 100mA) ──> IR2110 Gate Driver
                       (from separate buck or LDO if PV ≥15V)

┌──────────────────────────────────────────┐
│       Smart Home Connectivity            │
└──────────────────────────────────────────┘

ESP32-C6 ─┬─> WiFi 6 (2.4GHz) ──────────> Cloud, OTA, Telemetry
          │
          ├─> Thread (802.15.4) ────────> Matter over Thread
          │                                Multi-endpoint device:
          │                                - Endpoint 1: Solar Panel (Power Source)
          │                                - Endpoint 2: Battery (Power Source)
          │                                - Endpoint 3: LED Driver (Dimmable Light)
          │
          ├─> Bluetooth Mesh ───────────> Provisioning, Local Control
          │
          └─> DALI+ over IP ────────────> TCP port 55825
                (via WiFi)                 LED driver control
                mDNS: _dali._tcp.local
```

## Functional Flow

### Power Flow
```
PV Panel → Reverse Protection → Boost Converter → BMS (1S 304Ah, boosted to 48-129V)
                                                 └─> LED Driver (with dimming)
```

### Data Flow
```
PV Voltage/Current (INA226 #1) ──┐
                                  ├──> ESP32-C6 ──> MPPT Algorithm (P&O)
Output Voltage/Current (INA226 #2)─┤              ──> PWM Duty Cycle
                                   │              ──> Matter Endpoints
BMS SOC/Temp/Fault ────────────────┘              ──> Thread FDTRGB LED
                                                  ──> DALI+ LED Control
```

### Control Flow
```
MPPT P&O Algorithm ──> ESP32-C6 GPIO3 PWM ──> IR2110 Driver ──> Q1/Q2 Gates ──> Boost Switching
                                                                 50kHz, adjustable duty cycle
                                                                 
Matter Level Control ──> ESP32-C6 ──> LED Dimming PWM (GPIO6) ──> 0-10V ──> LED Driver
                                   └──> DALI+ PDU ──> TCP 55825 ──> LED Driver (if DALI+)
```

## Communication Interfaces

### I²C Bus (400kHz)
```
ESP32-C6 GPIO8/9 ─── INA226 #1 (0x40) ── INA226 #2 (0x41) ── [BMS if I²C] (0x55)
                     2.2kΩ pull-ups to 3.3V
```

### UART (BMS Communication)
```
ESP32-C6 GPIO16 (RX) ←───── BMS TX
ESP32-C6 GPIO17 (TX) ─────→ BMS RX
Baud: 9600 or 115200
```

### CAN Bus (Alternative BMS)
```
ESP32-C6 GPIO16/17 ──> MCP2551 Transceiver ──> CAN_H/CAN_L ──> BMS
```

### Thread/WiFi/BLE (Shared 2.4GHz Radio)
```
ESP32-C6 RF Pin ──> Matching Network ──> Antenna (U.FL or PCB trace)
```

## Revision History

- **v1.0** (2025-12-15): Initial hardware block diagram for MPPT 150W boost converter
