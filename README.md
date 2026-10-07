# Synchronization-Based Smart Home Automation

An embedded systems and Operating System (OS) concept implementation on a bare-metal **Arduino Uno R3**. This project demonstrates concurrent task management, hardware interrupts, event-driven processing, and mutual exclusion (Mutex) locks without using a full Real-Time Operating System (RTOS).

---

## Key Features

- **Interrupt-Driven Event Processing:** PIR motion sensor uses hardware interrupts (INT0) for zero-latency response.
- **Non-Blocking Multitasking:** Replaces delay functions with cooperative timestamping (`millis()`).
- **Mutual Exclusion (Mutex) Simulation:** Software-level flag locking protects shared peripherals (OLED display) from race conditions.
- **Microclimate Monitoring:** Continuous temperature and humidity tracking using the DHT11 sensor with auto-actuation thresholds.
- **Wireless Override:** Asynchronous Bluetooth commands via the HC-05 module allow remote user overrides.

---

## Operating System Concepts Implemented

1. **Hardware Interrupt Service Routines (ISRs):** Immediate execution for high-priority inputs without CPU polling loops.
2. **Cooperative Time-Slicing Scheduler:** Scheduled intervals for periodic tasks (DHT reading, OLED refresh) run in parallel without blocking execution.
3. **Synchronization & Race Condition Avoidance:** Shared flags are marked with the `volatile` keyword to ensure thread-safe variable updates between ISRs and the main execution loop.
4. **Mutex-Protected Shared Resource:** Display routines use a lock flag to ensure only one process updates screen memory at a time.

---

## Hardware Architecture & Connections

### Components Used
- **Controller:** Arduino Uno Rev3 (ATmega328P MCU)
- **Display:** 0.96" 4-Pin I2C OLED Display (128x64)
- **Sensors:** PIR Motion Sensor, DHT11 Temperature & Humidity Sensor
- **Actuators:** Room Light LED, Cooling Fan LED (via 220Ω Current-Limiting Resistors)
- **Communication:** HC-05 Bluetooth Module

---

### Pin Mapping Table

| Component | Pin Label | Arduino Pin | Function |
|---|---|---|---|
| **PIR Motion Sensor** | OUT | Digital Pin 2 (INT0) | Hardware Interrupt Input |
| **DHT11 Sensor** | DATA | Digital Pin 3 | Temperature / Humidity Signal |
| **Room Light LED** | Anode (+) | Digital Pin 7 | Simulated Light Output |
| **Cooling Fan LED** | Anode (+) | Digital Pin 8 | Simulated Fan Output |
| **0.96" OLED Display** | SDA | Analog Pin A4 | Shared I2C Serial Data Bus |
| **0.96" OLED Display** | SCL | Analog Pin A5 | Shared I2C Serial Clock Bus |
| **HC-05 Bluetooth** | TXD | Digital Pin 10 | Software Serial RX |
| **HC-05 Bluetooth** | RXD | Digital Pin 11 | Software Serial TX |

---

## Software Dependencies

Install the following libraries via the Arduino IDE Library Manager before compilation:
- `Adafruit GFX Library`
- `Adafruit SSD1306`
- `DHT sensor library` (by Adafruit)

---

## Bluetooth Control Commands

| Serial Command | Action |
|---|---|
| `1` | Force Room Light LED **ON** |
| `0` | Force Room Light LED **OFF** |
| `F` | Force Fan Indicator LED **ON** |
| `f` | Force Fan Indicator LED **OFF** |

---

## Authors & Team

- **L Poojitha**
- **B. Sharvika Varshini**  
*Department of Computer Science & Engineering (Cyber Security)*