# ESP32 Radar Scanner

A compact desktop radar scanner built on an ESP32-WROOM, sweeping an HC-SR04 ultrasonic sensor via servo and rendering a live radar display on a 128×64 SPI OLED.

---

## Hardware

| Component | Part |
|---|---|
| Microcontroller | ESP32-WROOM Dev Module |
| Display | SSD1306 0.96" OLED (SPI, 128×64) |
| Sensor | HC-SR04 Ultrasonic |
| Servo | SG90 (from micro:bit robocar kit) |

## Wiring

| Signal | ESP32 Pin |
|---|---|
| OLED CLK (D0) | GPIO 18 |
| OLED MOSI (D1) | GPIO 23 |
| OLED CS | GPIO 5 |
| OLED DC | GPIO 2 |
| OLED RST (RES) | GPIO 4 |
| HC-SR04 TRIG | GPIO 13 |
| HC-SR04 ECHO | GPIO 12 |
| Servo signal | GPIO 14 |

All VCC → 3V3, all GND → GND.  
Servo wire colours: brown = GND, red = VCC, orange = signal.

---

## Libraries

Install via Arduino Library Manager:

- `Adafruit SSD1306`
- `Adafruit GFX`
- `ESP32Servo`

---

## How it works

The servo sweeps the HC-SR04 from 0° to 180° and back continuously. At each degree the sensor takes **6 distance readings**, bubble-sorts them, drops the highest and lowest (outlier rejection), and averages the remaining 4. This trimmed mean significantly reduces the spiking behaviour common with HC-SR04 on angled surfaces.

The OLED renders a full-screen radar arc with:
- 3 range rings (at 1/3, 2/3, and full range)
- Spoke lines every 30°
- A bold animated sweep line tracking the current servo position
- Persistent blips at detected distances, mapped radially to range
- Live angle and distance readout in the bottom corners

Detection range is set to **150 cm** by default (`MAX_DIST`).

---

## Configuration

```cpp
#define MAX_DIST  150   // detection range in cm
#define SAMPLES   6     // pings per position (min 4)
```

Sweep speed is controlled by `delay(40)` in `loop()`. Increase if the servo jitters or misses steps, decrease for faster sweep.

---

## Build notes

- The HC-SR04 is physically mounted on the servo arm — keep its wires loose enough to handle the full 180° sweep without binding
- Powered via USB or VIN (5V)
- Tested in Arduino IDE with ESP32 board package