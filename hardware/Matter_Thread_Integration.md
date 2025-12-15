# Matter and Thread Integration

## Overview

This document specifies the integration of Matter over Thread, WiFi 6, and Bluetooth Mesh protocols for smart home connectivity using the ESP32-C6 microcontroller.

## Smart Home Protocol Stack

### Protocols Supported
1. **Matter over Thread**: Primary protocol for battery, solar panel, and LED driver endpoints
2. **Thread Mesh**: IEEE 802.15.4 @ 2.4 GHz for low-power mesh networking
3. **WiFi 6**: Telemetry, cloud connectivity, OTA firmware updates
4. **Bluetooth Mesh**: Provisioning and local control

### ESP32-C6 Capabilities
- **CPU**: RISC-V single-core, 160MHz
- **WiFi**: 802.11ax (WiFi 6), 2.4GHz only
- **Bluetooth**: BLE 5.3
- **Thread**: IEEE 802.15.4 MAC/PHY, 2.4GHz

---

## Matter over Thread

### Matter Device Types

The MPPT controller implements a **multi-endpoint Matter device**:

#### Endpoint 0: Root Device
- **Device Type**: Root Node
- **Clusters**: Descriptor, Basic Information, Network Commissioning

#### Endpoint 1: Solar Panel
- **Device Type**: Power Source (0x0011)
- **Clusters**:
  - Power Source (0x002F): Battery status, wired source status
  - Electrical Measurement (0x0B04): Voltage, current, power from INA226
  - Temperature Measurement (0x0402): PV panel temperature (optional)

#### Endpoint 2: Battery
- **Device Type**: Power Source (0x0011)
- **Clusters**:
  - Power Source (0x002F): Battery level, charge state, capacity
  - Electrical Measurement (0x0B04): Voltage, current, SOC from BMS

#### Endpoint 3: LED Driver
- **Device Type**: Dimmable Light (0x0101)
- **Clusters**:
  - On/Off (0x0006): LED on/off control
  - Level Control (0x0008): Dimming (0–100%)
  - Electrical Measurement (0x0B04): LED power consumption

### Matter Clusters Implementation

#### Power Source Cluster (0x002F)
**Attributes**:
- BatChargeLevel (enum): Unknown, OK, Warning, Critical
- BatPercentRemaining (uint8): 0–200 (0% to 100%, 0.5% resolution)
- BatVoltage (uint32): Millivolts
- BatChargeState (enum): Unknown, IsCharging, IsAtFullCharge, IsNotCharging

**Firmware Example**:
```c
void update_battery_attributes(float soc, float voltage, bool charging) {
    uint8_t percent_remaining = (uint8_t)(soc * 2.0);  // 0–100% → 0–200
    uint32_t voltage_mv = (uint32_t)(voltage * 1000.0);
    
    matter_update_attribute(ENDPOINT_BATTERY, CLUSTER_POWER_SOURCE, 
                           ATTR_BAT_PERCENT_REMAINING, &percent_remaining);
    matter_update_attribute(ENDPOINT_BATTERY, CLUSTER_POWER_SOURCE, 
                           ATTR_BAT_VOLTAGE, &voltage_mv);
    
    uint8_t charge_state = charging ? BAT_CHARGING : BAT_NOT_CHARGING;
    matter_update_attribute(ENDPOINT_BATTERY, CLUSTER_POWER_SOURCE, 
                           ATTR_BAT_CHARGE_STATE, &charge_state);
}
```

#### Electrical Measurement Cluster (0x0B04)
**Attributes**:
- RMSVoltage (uint16): Volts × 10
- RMSCurrent (uint16): Amps × 1000
- ActivePower (int16): Watts

**Firmware Example**:
```c
void update_pv_electrical_measurement(float vpv, float ipv, float ppv) {
    uint16_t rms_voltage = (uint16_t)(vpv * 10.0);
    uint16_t rms_current = (uint16_t)(ipv * 1000.0);
    int16_t active_power = (int16_t)ppv;
    
    matter_update_attribute(ENDPOINT_SOLAR, CLUSTER_ELECTRICAL_MEASUREMENT, 
                           ATTR_RMS_VOLTAGE, &rms_voltage);
    matter_update_attribute(ENDPOINT_SOLAR, CLUSTER_ELECTRICAL_MEASUREMENT, 
                           ATTR_RMS_CURRENT, &rms_current);
    matter_update_attribute(ENDPOINT_SOLAR, CLUSTER_ELECTRICAL_MEASUREMENT, 
                           ATTR_ACTIVE_POWER, &active_power);
}
```

#### Level Control Cluster (0x0008) for LED
**Attributes**:
- CurrentLevel (uint8): 0–254 (0 = off, 254 = max brightness)

**Commands**:
- MoveToLevel: Set LED brightness
- Move: Continuous dimming
- Step: Incremental dimming

**Firmware Example**:
```c
void matter_led_level_command_handler(uint8_t level) {
    uint8_t percent = (level * 100) / 254;
    set_led_brightness(percent);  // Call LED dimming function
    
    // Update Matter attribute
    matter_update_attribute(ENDPOINT_LED, CLUSTER_LEVEL_CONTROL, 
                           ATTR_CURRENT_LEVEL, &level);
    
    // Sync with DALI+ if enabled
    dali_set_brightness(percent);
}
```

### Matter Commissioning

**Commissioning Flow**:
1. User scans QR code or enters setup code
2. ESP32-C6 advertises over BLE (Bluetooth Mesh provisioning)
3. Commissioner sends network credentials (Thread/WiFi)
4. Device joins Thread network and registers with Matter controller
5. Endpoints and clusters discovered by Matter controller

**QR Code Generation**:
- Vendor ID: Assigned by CSA (Connectivity Standards Alliance)
- Product ID: Assigned by vendor
- Discriminator: Unique 12-bit value
- Setup PIN: Random 8-digit code

**Firmware Setup**:
```c
#include "esp_matter.h"

void matter_init(void) {
    esp_matter_attribute_val_t val = esp_matter_invalid(NULL);
    
    // Create root node
    node_t *node = node::create(node_config_default, NULL, NULL);
    
    // Endpoint 1: Solar Panel
    endpoint_t *solar_endpoint = power_source::create(node, &solar_config, ENDPOINT_FLAG_NONE);
    cluster_t *elec_cluster = electrical_measurement::create(solar_endpoint, &elec_config, ENDPOINT_FLAG_NONE);
    
    // Endpoint 2: Battery
    endpoint_t *battery_endpoint = power_source::create(node, &battery_config, ENDPOINT_FLAG_NONE);
    
    // Endpoint 3: LED Driver
    endpoint_t *led_endpoint = dimmable_light::create(node, &led_config, ENDPOINT_FLAG_NONE);
    
    esp_matter::start(node);
    ESP_LOGI(TAG, "Matter started, waiting for commissioning");
}
```

---

## Thread Mesh Networking

### Thread Network Parameters
- **Frequency**: 2.4GHz (IEEE 802.15.4)
- **Channels**: 11–26 (configurable, typically channel 15 or 20)
- **Data Rate**: 250kbps
- **Range**: 10–30m indoors (varies with environment)
- **Topology**: Mesh (self-healing, multi-hop)

### Device Roles

**Options for ESP32-C6**:
1. **End Device**: Sleepy device (not suitable for MPPT, which needs continuous operation)
2. **Router**: Forwarding device, always on (suitable for MPPT)
3. **Border Router**: Gateway between Thread mesh and WiFi/Ethernet (requires WiFi uplink)

**Recommended**: **Router** or **Border Router** (if WiFi uplink available)

### Thread Border Router Mode

If the MPPT controller has WiFi connectivity, it can act as a **Thread Border Router**:

**Functions**:
- Routes traffic between Thread mesh and WiFi/Internet
- Provides NAT64 for IPv6-to-IPv4 translation
- mDNS gateway for service discovery

**Configuration**:
```c
#include "esp_openthread.h"
#include "openthread/border_router.h"

void thread_border_router_init(void) {
    esp_openthread_platform_config_t config = {
        .radio_config = {
            .radio_mode = RADIO_MODE_NATIVE,
        },
        .host_config = {
            .host_connection_mode = HOST_CONNECTION_MODE_CLI_UART,
        },
    };
    
    ESP_ERROR_CHECK(esp_openthread_init(&config));
    ESP_ERROR_CHECK(esp_openthread_border_router_init());
    ESP_ERROR_CHECK(esp_openthread_launch_mainloop());
    
    ESP_LOGI(TAG, "Thread Border Router started");
}
```

**Requirements**:
- WiFi interface (ESP32-C6 WiFi used for uplink)
- OpenThread Border Router (OTBR) stack
- IPv6 routing enabled

### Thread Network Formation

**Master Key**: Pre-shared or generated during commissioning

**Network Name**: User-defined (e.g., "MPPT-Thread-Net")

**PAN ID**: 16-bit identifier (auto-generated or configured)

**Firmware**:
```c
void thread_network_start(void) {
    otInstance *instance = esp_openthread_get_instance();
    
    otOperationalDataset dataset;
    otDatasetCreateNewNetwork(instance, &dataset);
    
    // Set network name
    const char *network_name = "MPPT-Thread-Net";
    otNetworkNameFromString(&dataset.mNetworkName, network_name);
    
    otDatasetSetActive(instance, &dataset);
    otIp6SetEnabled(instance, true);
    otThreadSetEnabled(instance, true);
    
    ESP_LOGI(TAG, "Thread network started");
}
```

---

## WiFi 6 Integration

### WiFi Usage
- **Telemetry**: Send MPPT data to cloud (MQTT, HTTP)
- **OTA Updates**: Firmware upgrade over WiFi
- **Border Router Uplink**: Route Thread traffic to Internet

### WiFi Configuration

**SSID and Password**: Provisioned via Matter commissioning or Bluetooth

**Firmware**:
```c
#include "esp_wifi.h"

void wifi_init(const char *ssid, const char *password) {
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = "",
            .password = "",
        },
    };
    strcpy((char *)wifi_config.sta.ssid, ssid);
    strcpy((char *)wifi_config.sta.password, password);
    
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
    ESP_ERROR_CHECK(esp_wifi_connect());
    
    ESP_LOGI(TAG, "WiFi connecting to %s", ssid);
}
```

### mDNS Service Discovery

**Services Advertised**:
- `_matter._tcp.local`: Matter device discovery
- `_dali._tcp.local`: DALI+ over IP service (TCP port 55825)

**Firmware**:
```c
#include "mdns.h"

void mdns_init_services(void) {
    mdns_init();
    mdns_hostname_set("mppt-controller");
    mdns_instance_name_set("MPPT 150W Boost");
    
    // Advertise Matter service
    mdns_service_add(NULL, "_matter", "_tcp", 5540, NULL, 0);
    
    // Advertise DALI+ service
    mdns_service_add(NULL, "_dali", "_tcp", 55825, NULL, 0);
    
    ESP_LOGI(TAG, "mDNS services advertised");
}
```

---

## Bluetooth Mesh

### BLE Usage
- **Provisioning**: Initial device setup (Matter commissioning)
- **Local Control**: Direct control without WiFi/Thread (optional)

### BLE Advertising

**During Commissioning**:
- Device advertises as "MPPT-XXXX" (where XXXX is last 4 digits of MAC address)
- Includes Matter commissioning data

**Firmware**:
```c
#include "esp_gap_ble_api.h"

void ble_advertising_start(void) {
    esp_ble_adv_data_t adv_data = {
        .set_scan_rsp = false,
        .include_name = true,
        .manufacturer_len = 4,
        .p_manufacturer_data = (uint8_t[]){0xFF, 0xFF, 0x01, 0x02},  // Vendor-specific
    };
    
    esp_ble_gap_config_adv_data(&adv_data);
    esp_ble_gap_start_advertising(&adv_params);
    
    ESP_LOGI(TAG, "BLE advertising started");
}
```

---

## OTA Firmware Updates

### Matter OTA

**OTA Provider**: Matter controller (e.g., Apple Home, Google Home, Amazon Alexa)

**Process**:
1. Matter controller notifies device of available update
2. Device requests firmware image via Matter OTA cluster
3. Image downloaded and verified (signature check)
4. Device reboots into new firmware

**Firmware**:
```c
#include "esp_ota_ops.h"

void matter_ota_update_callback(const uint8_t *data, size_t len) {
    static esp_ota_handle_t ota_handle;
    static const esp_partition_t *update_partition;
    
    if (ota_handle == 0) {
        update_partition = esp_ota_get_next_update_partition(NULL);
        esp_ota_begin(update_partition, OTA_SIZE_UNKNOWN, &ota_handle);
    }
    
    esp_ota_write(ota_handle, data, len);
    
    // On completion:
    esp_ota_end(ota_handle);
    esp_ota_set_boot_partition(update_partition);
    esp_restart();
}
```

---

## Antenna Design

### 2.4GHz Antenna (Thread/WiFi/BLE)

**Options**:
1. **PCB Trace Antenna** (meandered or inverted-F)
2. **External Chip Antenna** (ceramic, via U.FL connector)

**Recommended**: External chip antenna for ease of design and tuning

**Matching Network**: 50Ω impedance matching (π-network or L-network)

See `Pinout_and_NetMap.md` for antenna connection details.

---

## Design Checklist

- [ ] Matter SDK (esp-matter) integrated in firmware
- [ ] OpenThread stack enabled for Thread mesh
- [ ] WiFi 6 configured for telemetry and OTA
- [ ] Bluetooth Mesh enabled for provisioning
- [ ] Multi-endpoint device created (Solar, Battery, LED)
- [ ] Matter clusters implemented (Power Source, Electrical Measurement, Level Control)
- [ ] Thread Border Router mode configured (if WiFi uplink available)
- [ ] mDNS services advertised (Matter, DALI+)
- [ ] OTA update mechanism tested
- [ ] Antenna matching network designed and tested

---

## References

- Matter Specification (CSA Alliance)
- ESP-Matter SDK (Espressif GitHub)
- OpenThread Documentation
- ESP32-C6 WiFi 6 Documentation (ESP-IDF)
- Bluetooth Mesh Specification (Bluetooth SIG)

## Revision History

- **v1.0** (2025-12-15): Initial Matter/Thread integration specification
