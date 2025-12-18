# Hardware Setup Guide

Complete hardware documentation for the CH32V003 Automatic Voltage Stabilizer project.

## Table of Contents

- [System Overview](#system-overview)
- [Components List](#components-list)
- [CH32V003A4M6 Overview](#ch32v003a4m6-overview)
- [Pin Configuration](#pin-configuration)
- [Relay Circuit Design](#relay-circuit-design)
- [Voltage Sensing Circuit](#voltage-sensing-circuit)
- [Transformer Tap Configuration](#transformer-tap-configuration)
- [LED Indicators](#led-indicators)
- [Power Supply](#power-supply)
- [PCB Design Considerations](#pcb-design-considerations)
- [Safety Notes](#safety-notes)

---

## System Overview

The automatic voltage stabilizer monitors input voltage and controls a multi-tap transformer to maintain stable output voltage. The system uses:

- **5 Relays**: 4 for tap selection (R1-R4) + 1 for over/under-voltage protection (R5)
- **8 Voltage Steps**: Binary combinations of R1-R4 provide 8 discrete tap ratios
- **ADC Monitoring**: Continuous voltage sensing via scaled analog input
- **Protection**: Automatic disconnection for high (>256V) or low (<181V) voltages

```
                    ┌─────────────────────────────────────────────┐
                    │              TRANSFORMER                    │
    AC INPUT ───────┤                                             │
    (180-280V)      │  ┌─T1─┐ ┌─T2─┐ ┌─T3─┐ ┌─T4─┐               │
                    │  │ R1 │ │ R2 │ │ R3 │ │ R4 │               │
                    │  └────┘ └────┘ └────┘ └────┘               │
                    │                                             │
                    └─────────────────────┬───────────────────────┘
                                          │
                                         [R5] Protection Relay
                                          │
                                          ▼
                                     AC OUTPUT
                                     (Regulated)
```

---

## Components List

### Required Components

| Component | Quantity | Specification |
|-----------|----------|---------------|
| CH32V003A4M6 | 1 | RISC-V MCU, SOP-16 package |
| Relay (SPDT) | 5 | 5V or 12V coil, 10A contacts |
| Relay Driver | 5 | ULN2003 or individual transistors |
| Optocoupler | 5 | PC817 or equivalent (optional isolation) |
| Flyback Diodes | 5 | 1N4007 or 1N4148 |
| Voltage Sensor | 1 | Transformer + resistor divider |
| LEDs | 3 | 3mm/5mm, any color |
| LED Resistors | 3 | 330-1kΩ for 5V operation |
| Push Buttons | 2 | Momentary, normally open |
| Crystal | 1 | 24 MHz (if using external) |
| Capacitors | - | 100nF ceramic, 10µF electrolytic |
| Resistors | - | Various (see specific circuits) |
| PCB | 1 | Custom or prototyping board |
| Enclosure | 1 | Suitable for mains voltage |
| Terminal Blocks | - | For mains connections |
| Fuses | 2 | Input and output protection |

### Optional Components

| Component | Quantity | Purpose |
|-----------|----------|---------|
| WCH-Link | 1 | Programming/debugging |
| 7-Segment Display | 1 | Voltage readout (optional) |
| Buzzer | 1 | Audible fault alarm |
| Varistor (MOV) | 1 | Surge protection |
| TVS Diodes | - | ESD protection |

---

## CH32V003A4M6 Overview

### Key Specifications

| Parameter | Value |
|-----------|-------|
| Core | 32-bit RISC-V2A |
| Max Frequency | 48 MHz (24 MHz used) |
| Flash Memory | 16 KB |
| SRAM | 2 KB |
| Operating Voltage | 2.5V - 5.5V |
| GPIO Pins | 18 (package dependent) |
| ADC | 10-bit, 8 channels |
| Timers | 1x 16-bit, 1x 16-bit advanced |
| Package | SOP-16, SOP-8, QFN-20 |

### Pinout (SOP-16 Package)

```
                     CH32V003A4M6 (SOP-16)
                      ┌────────────┐
               PD4 ──┤1         16├── PD3/PA6
               PD5 ──┤2         15├── PD2
               PD6 ──┤3         14├── PD7/NRST
               PD1 ──┤4         13├── PA1
               PD0 ──┤5         12├── PA2
               VSS ──┤6         11├── VDD
               PC0 ──┤7         10├── PC7
               PC1 ──┤8          9├── PC2-PC6
                      └────────────┘
```

**Note**: Actual pin availability depends on package variant. Verify with datasheet.

---

## Pin Configuration

### Complete Pin Assignment

| Pin | Port | Function | Direction | Description |
|-----|------|----------|-----------|-------------|
| PC0 | C | R1 | Output | Relay 1 control |
| PC1 | C | LOWCUT_EN | Input | Low-cut enable (active low) |
| PC3 | C | M_START | Input | Manual start button |
| PC4 | C | BUTTON | Input | Settings button |
| PC7 | C | MAIN_LED | Output | Normal operation indicator |
| PD1 | D | FAULT_LED | Output | Fault condition indicator |
| PD4 | D | SETTING_LED | Output | Setting mode indicator |
| PD5 | D | R4 | Output | Relay 4 control |
| PD6 | D | R3 | Output | Relay 3 control |
| PD7 | D | R2 | Output | Relay 2 control |
| PA1 | A | R5 | Output | Protection relay control |
| PA2 | A | ADC_SENSE | Analog In | Voltage sensor input |
| VDD | - | Power | - | 3.3V or 5V supply |
| VSS | - | Ground | - | Ground reference |

### GPIO Configuration Summary

```c
// Output pins (Push-Pull, 50MHz)
Relays:     PC0 (R1), PD7 (R2), PD6 (R3), PD5 (R4), PA1 (R5)
LEDs:       PC7 (Main), PD1 (Fault), PD4 (Setting)

// Input pins (Internal Pull-Up)
Buttons:    PC4 (Setting), PC3 (M-START)
Enable:     PC1 (Low-cut enable)

// Analog input
ADC:        PA2 (Channel 0)
```

---

## Relay Circuit Design

### Relay Driver Circuit

The MCU GPIO cannot drive relay coils directly. Use transistor or IC drivers:

#### Option 1: ULN2003 Driver IC (Recommended)

```
                          ULN2003
    MCU GPIO ────────────┤IN1    OUT1├──────── Relay Coil (+)
    MCU GPIO ────────────┤IN2    OUT2├──────── Relay Coil (+)
    MCU GPIO ────────────┤IN3    OUT3├──────── Relay Coil (+)
    MCU GPIO ────────────┤IN4    OUT4├──────── Relay Coil (+)
    MCU GPIO ────────────┤IN5    OUT5├──────── Relay Coil (+)
                         ┤GND    COM ├──────── +12V
                          └──────────┘
                                      All relay coil (-) to GND
```

**Note**: ULN2003 has built-in flyback diodes. COM pin connects to relay supply voltage.

#### Option 2: Individual NPN Transistors

```
    MCU GPIO ─────┬──── 1kΩ ────┬──── Base (BC547/2N2222)
                  │              │
                 GND            │
                           ┌────┴────┐
                           │   NPN   │
                           │ BC547   │
                           └────┬────┘
                                │
                           Collector ── Relay Coil (-)
                                │
                           Emitter ──── GND

    +V Relay ────┬──── Relay Coil (+)
                 │
                ─┴─
               │ ▲ │ 1N4007 (Flyback)
                ─┬─
                 │
            Relay Coil (-)
```

**Component Values**:
- Base resistor: 1kΩ - 2.2kΩ
- Transistor: BC547, BC337, 2N2222 (any NPN with Ic > 100mA)
- Flyback diode: 1N4007 or 1N4148 (CRITICAL - prevents voltage spikes)

### Relay Selection

| Parameter | Requirement |
|-----------|-------------|
| Coil Voltage | 5V or 12V DC |
| Coil Current | < 80mA per relay |
| Contact Rating | ≥ 10A @ 250VAC |
| Contact Type | SPDT (Single Pole Double Throw) |
| Switching Life | > 100,000 operations |

**Recommended Relays**: SRD-05VDC-SL-C, SRD-12VDC-SL-C, or equivalent

---

## Voltage Sensing Circuit

### Isolation Transformer Method (Recommended)

Use a small transformer to step down and isolate mains voltage:

```
    AC MAINS ──────┬──── Primary (240V) ────┬──────
                   │                         │
              Transformer                    │
              (240V:9V)                       │
                   │                         │
    Isolated ──────┴──── Secondary (9V) ─────┴──────
                              │
                              ▼
                         Rectifier
                         (Bridge)
                              │
                         ┌────┴────┐
                         │         │
                        100µF     GND
                         │
                         ▼
                    DC Voltage
                    (~12V peak)
                         │
                    ┌────┴────┐
                    │   R1    │ 10kΩ
                    └────┬────┘
                         │
                         ├─────────────► To MCU ADC (PA2)
                         │
                    ┌────┴────┐
                    │   R2    │ 3.3kΩ
                    └────┬────┘
                         │
                        GND
```

### Resistor Divider Calculation

For 240V AC input (340V peak) scaled to 3.3V DC:

```
Target: 244V AC → ADC reading during calibration
Transformer ratio: 240V:9V = 26.67:1
Secondary peak: 9V × √2 ≈ 12.7V

Divider ratio: 3.3V / 12.7V ≈ 0.26

R1 + R2 = 10kΩ + 3.3kΩ = 13.3kΩ
Vout = Vin × R2/(R1+R2) = 12.7 × 3.3/13.3 ≈ 3.15V

Adjust resistor values based on actual transformer output.
```

### Protection Components

Add protection to the ADC input:

```
    From Divider ─────┬───── 1kΩ ─────┬───── To ADC (PA2)
                      │               │
                     ─┴─             ─┴─
                    │ Z │           │   │ 100nF
                     ─┬─             ─┬─
                      │               │
                     GND             GND

    Z = 3.3V Zener diode (1N4728 or similar)
```

---

## Transformer Tap Configuration

### 8-Step Tap Arrangement

The main power transformer has multiple taps controlled by relays R1-R4:

```
    Primary ════════════════════════════════════════════════
                │       │       │       │       │
               Tap0    Tap1    Tap2    Tap3    Tap4
                │       │       │       │       │
               ─┴─     ─┴─     ─┴─     ─┴─     ─┴─
              │ R4│   │ R3│   │ R2│   │ R1│   │COM│
               ─┬─     ─┬─     ─┬─     ─┬─     ─┬─
                │       │       │       │       │
                └───────┴───────┴───────┴───────┴──► Output
```

### Step Table

| Step | R1 | R2 | R3 | R4 | Tap Ratio | Output at 220V In |
|------|----|----|----|----|-----------|-------------------|
| 0 | OFF | OFF | OFF | OFF | 0.472x | 104V |
| 1 | OFF | OFF | OFF | ON | 0.571x | 126V |
| 2 | OFF | OFF | ON | OFF | 0.690x | 152V |
| 3 | OFF | OFF | ON | ON | 0.833x | 183V |
| 4 | OFF | ON | ON | OFF | 1.000x | 220V (unity) |
| 5 | OFF | ON | ON | ON | 1.208x | 266V |
| 6 | ON | ON | ON | OFF | 1.441x | 317V |
| 7 | ON | ON | ON | ON | 1.742x | 383V |

### Hysteresis Thresholds

Thresholds are set to prevent oscillation between adjacent steps:

| Transition | Up Threshold | Down Threshold | Hysteresis |
|------------|--------------|----------------|------------|
| 0 → 1 | - | - | - |
| 1 → 2 | 115V | 111V | 4V |
| 2 → 3 | 139V | 135V | 4V |
| 3 → 4 | 168V | 163V | 5V |
| 4 → 5 | 203V | 196V | 7V |
| 5 → 6 | 244V | 236V | 8V |
| 6 → 7 | 295V | 282V | 13V |
| 7 → max | 352V | 340V | 12V |

---

## LED Indicators

### LED Circuit

```
    +5V ─────────┬──────────┬──────────┬───────
                 │          │          │
                ─┴─        ─┴─        ─┴─
               │LED│      │LED│      │LED│
               │GRN│      │RED│      │YLW│
                ─┬─        ─┬─        ─┬─
                 │          │          │
                330Ω       330Ω       330Ω
                 │          │          │
                 │          │          │
    PC7 ─────────┘          │          │
    PD1 ────────────────────┘          │
    PD4 ───────────────────────────────┘
```

### LED Behavior

| LED | Pin | State | Meaning |
|-----|-----|-------|---------|
| Main (Green) | PC7 | Solid ON | Normal operation |
| Main (Green) | PC7 | OFF | Startup delay active |
| Main (Green) | PC7 | Blinking | - |
| Fault (Red) | PD1 | Solid ON | High/low cut active |
| Fault (Red) | PD1 | OFF | No fault |
| Setting (Yellow) | PD4 | Blinking | Setting mode |
| Setting (Yellow) | PD4 | Solid ON | Uncalibrated |

---

## Power Supply

### MCU Power Circuit

```
    +12V (Relay Supply) ──────┬──────────────────────
                              │
                         ┌────┴────┐
                         │  7805   │
                         │ IN  OUT │
                         └────┬────┘
                              │
                              ├───────────────► +5V to MCU (VDD)
                              │
                             ─┴─
                            │   │ 100nF
                             ─┬─
                              │
                             GND

    Alternative: Use LM1117-3.3 for 3.3V operation
```

### Decoupling Capacitors

Place capacitors close to MCU VDD/VSS pins:

```
    VDD ─────┬─────┬───── MCU VDD
             │     │
            ─┴─   ─┴─
           │   │ │   │
           │   │ │   │ 100nF + 10µF
            ─┬─   ─┬─
             │     │
    VSS ─────┴─────┴───── MCU VSS (GND)
```

### Power Requirements

| Component | Current | Notes |
|-----------|---------|-------|
| CH32V003 MCU | 5-15 mA | Depends on clock speed |
| 5 x Relay Coils | 400 mA total | ~80mA each |
| LEDs | 30 mA | ~10mA each |
| **Total** | ~450 mA | Use adequate power supply |

---

## PCB Design Considerations

### Layout Guidelines

1. **Separation**: Keep high-voltage traces far from low-voltage logic
2. **Ground Plane**: Use solid ground plane for noise immunity
3. **Relay Placement**: Group relays near edge for heat dissipation
4. **Decoupling**: Place capacitors within 5mm of MCU power pins
5. **ADC Input**: Route away from noisy relay drivers

### Trace Widths

| Signal Type | Width | Current |
|-------------|-------|---------|
| Power (VDD/GND) | 0.5-1.0 mm | >500mA |
| Relay Drivers | 0.4-0.6 mm | ~100mA |
| Signal/GPIO | 0.25-0.3 mm | <10mA |
| High Voltage | 2.0+ mm | Per regulations |

### Clearances

| Voltage Level | Minimum Clearance |
|---------------|-------------------|
| Logic (5V) | 0.25 mm |
| Relay Coil (12V) | 0.5 mm |
| Mains (240VAC) | 3.0 mm minimum |
| Mains to Logic | 6.0 mm or isolated |

---

## Safety Notes

### Electrical Safety

1. **Mains Voltage**: This project involves dangerous mains voltage (240VAC)
2. **Isolation**: Ensure proper isolation between mains and control circuitry
3. **Enclosure**: Use appropriate enclosure rated for mains voltage
4. **Fusing**: Include input and output fuses
5. **Grounding**: Connect equipment ground properly
6. **Disclaimer**: Work should be performed by qualified electricians

### Component Safety

1. **Relay Rating**: Use relays rated for mains voltage and expected current
2. **Flyback Diodes**: Always include across relay coils
3. **Fuse Selection**: Match fuse rating to load requirements
4. **Surge Protection**: Consider MOV/TVS for surge suppression

### Programming Safety

1. **Power Off**: Always disconnect mains before programming
2. **WCH-Link**: Use isolated programmer if possible
3. **SDI Disable**: After disabling SDI, reprogramming requires power cycle

### Testing Procedure

1. **Bench Test**: Test with low voltage (variac) first
2. **Current Limit**: Use current-limited supply during development
3. **Isolation Check**: Verify isolation between mains and logic
4. **Relay Test**: Verify correct relay operation before connecting load
5. **Protection Test**: Verify high/low cut functions with test voltages

---

## References

- [CH32V003 Datasheet](http://www.wch.cn/products/CH32V003.html)
- [CH32V003 Reference Manual](http://www.wch.cn/downloads/CH32V003RM_PDF.html)
- [WCH-Link User Guide](http://www.wch.cn/products/WCH-Link.html)
- IEC 61010-1: Safety requirements for electrical equipment
- Local electrical codes and regulations

---

## Disclaimer

This hardware guide is for educational purposes. Working with mains voltage is dangerous and should only be performed by qualified personnel. The authors assume no liability for any damage or injury resulting from the use of this information. Always follow local electrical codes and safety regulations.
