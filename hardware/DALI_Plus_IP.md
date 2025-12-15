# DALI+ over IP - LED Control Protocol

## Overview

This document specifies DALI+ over IP (IEC 62386-104) implementation for controlling LED drivers over TCP/IP network. The ESP32-C6 acts as a DALI+ IP client or gateway.

## DALI+ over IP Standard (IEC 62386-104)

### Protocol Specifications
- **Transport**: TCP/IP
- **Port**: 55825 (standard DALI+ IP port)
- **Discovery**: mDNS service `_dali._tcp.local`
- **Encoding**: DALI PDU (Protocol Data Unit) encapsulated in TCP packets
- **Addressing**: Device addressing via DALI short address or broadcast

### DALI Commands
- **Arc Power**: Set brightness level (0–254)
- **Direct Arc**: Immediate brightness change
- **Configuration**: Device setup, grouping, scenes
- **Query**: Status, brightness, fault condition

---

## DALI+ IP Client Implementation

### TCP Client Connection

**Firmware**:
```c
#include "lwip/sockets.h"

#define DALI_IP_PORT 55825
#define DALI_SERVER_IP "192.168.1.100"  // LED driver IP or gateway IP

int dali_socket = -1;

int dali_ip_connect(const char *server_ip) {
    struct sockaddr_in server_addr = {
        .sin_family = AF_INET,
        .sin_port = htons(DALI_IP_PORT),
    };
    inet_pton(AF_INET, server_ip, &server_addr.sin_addr);
    
    dali_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (dali_socket < 0) {
        ESP_LOGE(TAG, "Failed to create socket");
        return -1;
    }
    
    if (connect(dali_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) != 0) {
        ESP_LOGE(TAG, "Failed to connect to DALI+ server");
        close(dali_socket);
        return -1;
    }
    
    ESP_LOGI(TAG, "Connected to DALI+ server %s:%d", server_ip, DALI_IP_PORT);
    return 0;
}
```

### DALI PDU Encoding

**PDU Structure**:
```
+--------+--------+--------+--------+
| Byte 0 | Byte 1 | Byte 2 | Byte 3 |
+--------+--------+--------+--------+
| Length | OpCode | Addr   | Data   |
+--------+--------+--------+--------+
```

**Example: Set Arc Power Level**:
- OpCode: 0x00 (Direct Arc)
- Address: 0xFF (broadcast to all devices)
- Data: 0–254 (brightness level)

**Firmware**:
```c
void dali_set_brightness(uint8_t address, uint8_t level) {
    uint8_t pdu[4] = {
        0x04,     // Length (4 bytes)
        0x00,     // OpCode: Direct Arc
        address,  // DALI short address or 0xFF (broadcast)
        level     // Brightness: 0 (off) to 254 (max)
    };
    
    send(dali_socket, pdu, sizeof(pdu), 0);
    ESP_LOGI(TAG, "DALI: Set brightness to %d", level);
}
```

### mDNS Discovery

**Discover DALI+ Gateways**:
```c
#include "mdns.h"

void dali_mdns_discover(void) {
    mdns_result_t *results = NULL;
    esp_err_t err = mdns_query_ptr("_dali", "_tcp", 3000, 10, &results);
    
    if (err == ESP_OK && results != NULL) {
        mdns_result_t *r = results;
        while (r) {
            ESP_LOGI(TAG, "DALI+ service found: %s at %s:%d", 
                     r->hostname, inet_ntoa(r->addr->addr.u_addr.ip4), r->port);
            r = r->next;
        }
        mdns_query_results_free(results);
    }
}
```

---

## Integration with Matter Level Control

### Synchronization

When Matter Level Control command is received, translate to DALI+ command:

**Firmware**:
```c
void matter_to_dali_sync(uint8_t matter_level) {
    // Matter level: 0–254
    // DALI level: 0–254 (direct mapping)
    uint8_t dali_level = matter_level;
    
    dali_set_brightness(0xFF, dali_level);  // Broadcast to all DALI devices
    
    // Also update local LED PWM (0–10V)
    uint8_t percent = (dali_level * 100) / 254;
    set_led_brightness(percent);
}
```

---

## DALI+ over WiFi vs Thread

**Recommendation**: Use **WiFi** for DALI+ IP traffic (TCP requires reliable transport, not ideal over Thread's low-bandwidth mesh).

**Alternative**: If LED driver is on Thread mesh, use Matter Level Control cluster instead of DALI+ IP.

---

## BOM for DALI+ IP

No additional hardware required (uses WiFi interface on ESP32-C6).

---

## Design Checklist

- [ ] mDNS service discovery implemented
- [ ] TCP client connection to DALI+ gateway tested
- [ ] DALI PDU encoding implemented (Direct Arc, Query commands)
- [ ] Matter Level Control → DALI+ brightness synchronization
- [ ] WiFi connectivity verified for DALI+ traffic

---

## References

- IEC 62386-104: DALI Part 104 - General requirements - DALI over IP
- DALI Alliance (DiiA) Specifications
- ESP-IDF TCP Socket Examples

## Revision History

- **v1.0** (2025-12-15): Initial DALI+ over IP specification
