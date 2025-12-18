# Hardware Setup Guide

Complete hardware documentation for the CH32V003 7-Segment Display project.

## Table of Contents

- [Components List](#components-list)
- [CH32V003A4M6 Overview](#ch32v003a4m6-overview)
- [7-Segment Display Basics](#7-segment-display-basics)
- [Wiring Diagram](#wiring-diagram)
- [Current Limiting Resistors](#current-limiting-resistors)
- [Power Supply Requirements](#power-supply-requirements)
- [PCB Design Considerations](#pcb-design-considerations)

---

## Components List

### Required Components

| Component | Quantity | Description |
|-----------|----------|-------------|
| CH32V003A4M6 | 1 | RISC-V Microcontroller (SOP-16 package) |
| 3-Digit 7-Segment Display | 1 | Common Anode type |
| Resistors (330Ω) | 7 | Current limiting for segments |
| Capacitor (100nF) | 1 | Decoupling capacitor |
| Capacitor (10µF) | 1 | Bulk capacitor |
| WCH-Link | 1 | Programming/debugging |

### Optional Components

| Component | Quantity | Description |
|-----------|----------|-------------|
| Push Buttons | 2 | Programming mode selection |
| Pull-up Resistors (10kΩ) | 2 | For button inputs |
| LED (indicator) | 1 | Power/status indicator |
| Resistor (1kΩ) | 1 | For status LED |

---

## CH32V003A4M6 Overview

### Pinout (SOP-16 Package)

```
                 CH32V003A4M6
                  ┌────┬────┐
            PD4 ─┤1       16├─ PD5
            PD5 ─┤2       15├─ PD6
            PD6 ─┤3       14├─ PD7/NRST
            PD7 ─┤4       13├─ PA1
            PA1 ─┤5       12├─ PA2
            PA2 ─┤6       11├─ VSS (GND)
            VSS ─┤7       10├─ VDD (Power)
            PC1 ─┤8        9├─ PC2
                 └──────────┘
```

**Note**: Pin numbers may vary by package variant. Always verify with the official datasheet.

### Key Specifications

| Parameter | Value |
|-----------|-------|
| Core | 32-bit RISC-V2A |
| Max Frequency | 48 MHz |
| Operating Frequency | 24 MHz (this project) |
| Flash Memory | 16 KB |
| SRAM | 2 KB |
| Operating Voltage | 2.5V - 5.5V |
| Operating Temperature | -40°C to +85°C |
| GPIO Pins | Up to 18 (package dependent) |
| Package | SOP-16, QFN-20, SOP-8 |

### GPIO Capabilities

All GPIO pins support:
- Input modes: Floating, Pull-up, Pull-down
- Output modes: Push-Pull, Open-Drain
- Maximum output current: 8mA per pin
- 5V tolerant inputs (most pins)
- Configurable speed: 2MHz, 10MHz, 50MHz

---

## 7-Segment Display Basics

### Segment Naming Convention

```
      ════A════
     ║         ║
     F         B
     ║         ║
      ════G════
     ║         ║
     E         C
     ║         ║
      ════D════   ● DP
```

### Common Anode vs Common Cathode

| Type | Segment ON | Segment OFF | Common Pin |
|------|-----------|-------------|------------|
| Common Anode | LOW (0V) | HIGH (VDD) | Connect to VDD |
| Common Cathode | HIGH (VDD) | LOW (0V) | Connect to GND |

**This project uses Common Anode** displays.

### Display Pinout (Typical 3-Digit Common Anode)

Most 3-digit displays have 12 pins:

```
  Pin 1  ─ Segment E
  Pin 2  ─ Segment D
  Pin 3  ─ Common Digit 3 (Anode)
  Pin 4  ─ Segment C
  Pin 5  ─ Decimal Point
  Pin 6  ─ Segment B
  Pin 7  ─ Segment A
  Pin 8  ─ Common Digit 1 (Anode)
  Pin 9  ─ Segment F
  Pin 10 ─ Segment G
  Pin 11 ─ Common Digit 2 (Anode)
  Pin 12 ─ (Varies by model)
```

**Important**: Always verify your specific display's pinout with a multimeter or datasheet!

---

## Wiring Diagram

### Connection Table

| Display Pin | Function | MCU Pin | GPIO |
|-------------|----------|---------|------|
| Segment A | Top horizontal | PD1 | Port D, Pin 1 |
| Segment B | Upper right vertical | PD6 | Port D, Pin 6 |
| Segment C | Lower right vertical | PC2 | Port C, Pin 2 |
| Segment D | Bottom horizontal | PC1 | Port C, Pin 1 |
| Segment E | Lower left vertical | PC3 | Port C, Pin 3 |
| Segment F | Upper left vertical | PC6 | Port C, Pin 6 |
| Segment G | Middle horizontal | PC0 | Port C, Pin 0 |
| Digit 1 Common | Left digit anode | PC7 | Port C, Pin 7 |
| Digit 2 Common | Middle digit anode | PD5 | Port D, Pin 5 |
| Digit 3 Common | Right digit anode | PD7 | Port D, Pin 7 |

### ASCII Schematic

```
                                          ┌─────────────────────┐
                                          │  7-Segment Display  │
                                          │  (Common Anode)     │
                                          └─────────────────────┘
                                               │ │ │ │ │ │ │
                                               A B C D E F G
                                               │ │ │ │ │ │ │
                  330Ω  330Ω  330Ω  330Ω  330Ω  330Ω  330Ω
                   ┬     ┬     ┬     ┬     ┬     ┬     ┬
                   │     │     │     │     │     │     │
    ┌──────────────┼─────┼─────┼─────┼─────┼─────┼─────┼────┐
    │              │     │     │     │     │     │     │    │
    │  CH32V003    │     │     │     │     │     │     │    │
    │              │     │     │     │     │     │     │    │
    │  PD1 ────────┘     │     │     │     │     │     │    │
    │  PD6 ──────────────┘     │     │     │     │     │    │
    │  PC2 ────────────────────┘     │     │     │     │    │
    │  PC1 ──────────────────────────┘     │     │     │    │
    │  PC3 ────────────────────────────────┘     │     │    │
    │  PC6 ──────────────────────────────────────┘     │    │
    │  PC0 ────────────────────────────────────────────┘    │
    │                                                       │
    │  PC7 ─────────────── Digit 1 Common (via transistor)  │
    │  PD5 ─────────────── Digit 2 Common (via transistor)  │
    │  PD7 ─────────────── Digit 3 Common (via transistor)  │
    │                                                       │
    │  VDD ─────────────── 3.3V / 5V                        │
    │  VSS ─────────────── GND                              │
    │                                                       │
    └───────────────────────────────────────────────────────┘
```

### Optional: Transistor Drivers for Digit Control

For brighter displays or higher current requirements, use transistor drivers:

```
    MCU Pin ────┬──── 1kΩ ────┬──── Base
                │              │
               GND            │
                         ┌────┴────┐
                         │  PNP    │
                         │ BC557   │
                         └────┬────┘
                              │
                         Emitter ──── VDD
                              │
                         Collector ── Digit Common
```

---

## Current Limiting Resistors

### Calculating Resistor Values

For each segment LED:

```
R = (VDD - Vf) / If

Where:
  VDD = Supply voltage (3.3V or 5V)
  Vf  = LED forward voltage (~2V for red, ~3V for blue/white)
  If  = Desired forward current (10-20mA typical)
```

### Recommended Values

| Supply Voltage | LED Color | Resistor Value |
|----------------|-----------|----------------|
| 3.3V | Red | 100Ω - 150Ω |
| 3.3V | Green | 68Ω - 100Ω |
| 5V | Red | 220Ω - 330Ω |
| 5V | Green | 150Ω - 220Ω |
| 5V | Blue/White | 100Ω - 150Ω |

**This project uses 330Ω resistors** for safe operation across the voltage range.

### Power Dissipation

With multiplexing (duty cycle ~33%), average current per segment:

```
I_avg = I_peak × duty_cycle
I_avg = 10mA × 0.33 = 3.3mA per segment
```

Total maximum current (all segments on, one digit): ~70mA

---

## Power Supply Requirements

### Specifications

| Parameter | Minimum | Typical | Maximum |
|-----------|---------|---------|---------|
| Voltage | 2.5V | 3.3V / 5V | 5.5V |
| Current (MCU only) | - | 5mA | 15mA |
| Current (with display) | - | 50mA | 150mA |

### Decoupling Capacitors

Place capacitors as close to the MCU as possible:

```
         VDD
          │
         ─┴─
        │   │ 100nF (ceramic)
         ─┬─
          │
         ─┴─
        │   │ 10µF (electrolytic/tantalum)
         ─┬─
          │
         GND
```

### Power Supply Recommendations

1. **USB Power**: 5V from USB, add 3.3V regulator if needed
2. **Battery**: 2× AA/AAA (3V) or LiPo (3.7V nominal)
3. **Bench Supply**: Set to 3.3V or 5V with current limit at 200mA

---

## PCB Design Considerations

### Layout Guidelines

1. **Decoupling Capacitors**
   - Place 100nF ceramic within 5mm of VDD pin
   - Keep ground path short and direct

2. **Signal Routing**
   - Keep segment traces similar length if possible
   - Avoid running high-speed signals near analog pins

3. **Ground Plane**
   - Use solid ground plane on bottom layer
   - Connect all grounds with multiple vias

4. **Display Connector**
   - Use appropriate connector for your display type
   - Consider header pins for prototyping

### Recommended Trace Widths

| Signal Type | Trace Width |
|-------------|-------------|
| Power (VDD/GND) | 0.5mm - 1.0mm |
| Signal | 0.25mm - 0.3mm |
| High current (display) | 0.4mm - 0.6mm |

### ESD Protection

- Add TVS diodes on external connections
- Include series resistors (100Ω) on button inputs
- Use proper handling procedures during assembly

---

## Programming Connection

### WCH-Link Pinout

| WCH-Link Pin | MCU Pin | Function |
|--------------|---------|----------|
| SWDIO | PD1 | Debug Data |
| GND | VSS | Ground |
| 3V3 | VDD | Power (optional) |

**Note**: After programming, PD1 is reconfigured for display use.

### Programming Mode

1. Connect WCH-Link to target
2. Hold both buttons (PA1, PA2) during power-up
3. MCU enters programming mode
4. Use MounRiver Studio or WCH ISP tool

---

## Testing Procedure

### Initial Power-Up Test

1. **Before connecting display**: Verify 3.3V/5V on VDD pin
2. **Check current draw**: Should be <10mA without display
3. **Verify GPIO outputs**: Use multimeter or oscilloscope

### Display Test Sequence

1. Connect one segment (segment A recommended)
2. Verify it lights when GPIO goes LOW
3. Connect remaining segments one at a time
4. Test digit multiplexing

### Troubleshooting Hardware Issues

| Symptom | Possible Cause | Solution |
|---------|----------------|----------|
| No display | Wrong common type | Check anode/cathode |
| Dim display | High resistance | Reduce resistor values |
| Segments stuck ON | Transistor issue | Check driver circuit |
| Erratic behavior | Power issues | Add decoupling caps |
| MCU not responding | Wrong voltage | Verify power supply |

---

## Safety Notes

1. **ESD Precautions**: Handle ICs with proper grounding
2. **Polarity**: Double-check power connections before applying power
3. **Current Limits**: Never exceed 8mA per GPIO pin without drivers
4. **Heat**: Ensure adequate ventilation for power components

---

## References

- [CH32V003 Datasheet](http://www.wch.cn/products/CH32V003.html)
- [CH32V003 Reference Manual](http://www.wch.cn/downloads/CH32V003RM_PDF.html)
- [WCH-Link User Guide](http://www.wch.cn/products/WCH-Link.html)
