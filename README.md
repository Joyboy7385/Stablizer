# CH32V003 Automatic Voltage Stabilizer

Firmware for an automatic voltage regulator/stabilizer using the **CH32V003A4M6** RISC-V microcontroller. Controls a multi-tap transformer via relay switching to maintain stable output voltage within safe operating limits.

## Features

- **8-Step Voltage Regulation** - Automatic tap switching for output range 0.47x to 1.74x
- **Over-Voltage Protection** - High-cut disconnection at 256V with automatic recovery
- **Under-Voltage Protection** - Optional low-cut at 181V (hardware selectable)
- **Configurable Delay Timer** - 3-180 second startup delay after fault recovery
- **Flash-Persistent Settings** - Calibration and settings survive power loss
- **5V Optimized Operation** - Critical Flash latency configuration for reliable 5V operation
- **Multi-Stage ADC Filtering** - Robust noise rejection with averaged + exponential filtering

## Hardware Requirements

| Component | Specification |
|-----------|---------------|
| Microcontroller | CH32V003A4M6 (RISC-V 32-bit) |
| Supply Voltage | 2.5V - 5.5V (optimized for 5V) |
| Clock | 24 MHz HSI (Internal) |
| Relays | 5x (R1-R4 for tap control, R5 for protection) |
| Voltage Sensor | Scaled to 0-3.3V ADC range |
| Flash Memory | 16 KB (settings stored at 0x08001F80) |
| SRAM | 2 KB |

## Pin Configuration

### Relay Outputs

| Relay | Pin | Port | Function |
|-------|-----|------|----------|
| R1 | PC0 | Port C | Tap control (LSB) |
| R2 | PD7 | Port D | Tap control |
| R3 | PD6 | Port D | Tap control |
| R4 | PD5 | Port D | Tap control (MSB) |
| R5 | PA1 | Port A | Protection relay |

### Inputs

| Function | Pin | Port | Description |
|----------|-----|------|-------------|
| ADC Sense | PA2 | Port A | Voltage sensor input (Channel 0) |
| Button | PC4 | Port C | Setting mode entry |
| M-START | PC3 | Port C | Manual start button |
| Low-Cut Enable | PC1 | Port C | Hardware low-cut enable |

### LED Indicators

| LED | Pin | Port | Function |
|-----|-----|------|----------|
| Main LED | PC7 | Port C | Normal operation |
| Fault LED | PD1 | Port D | High/low cut active |
| Setting LED | PD4 | Port D | Setting mode indicator |

## Quick Start

### 1. Clone the Repository

```bash
git clone https://github.com/Joyboy7385/Stablizer.git
cd Stablizer
```

### 2. Build the Project

**Using MounRiver Studio (Recommended):**
1. Open MounRiver Studio
2. File -> Import -> Existing Projects
3. Navigate to the cloned repository
4. Select project and import
5. Click Build

**Using Command Line:**

```bash
riscv-none-embed-gcc \
  -march=rv32ec \
  -mabi=ilp32e \
  -O2 \
  -o stabilizer.elf \
  main.c system_ch32v00x.c \
  ch32v00x_gpio.c ch32v00x_rcc.c \
  ch32v00x_adc.c ch32v00x_tim.c \
  ch32v00x_flash.c ch32v00x_misc.c
```

### 3. Flash the Firmware

```bash
# Using WCH-Link programmer
wch-isp -p stabilizer.hex
```

## Calibration

The stabilizer requires calibration before first use:

1. **Enter Setting Mode**: Hold the Button (PC4) for 1 second during power-on
2. **Setting LED Blinks**: 3 rapid blinks confirm setting mode entry
3. **Apply Reference Voltage**: Supply exactly 244V to the input
4. **Automatic Capture**: Device captures ADC reading and saves to Flash
5. **Confirm & Reboot**: Settings are stored; device reboots to normal operation

## Voltage Regulation Steps

The stabilizer uses 8 discrete tap ratios for voltage regulation:

| Step | R1 | R2 | R3 | R4 | Tap Ratio | Threshold Up | Threshold Down |
|------|----|----|----|----|-----------|--------------|----------------|
| 0 | OFF | OFF | OFF | OFF | 0.472x | - | - |
| 1 | OFF | OFF | OFF | ON | 0.571x | 115V | 111V |
| 2 | OFF | OFF | ON | OFF | 0.690x | 139V | 135V |
| 3 | OFF | OFF | ON | ON | 0.833x | 168V | 163V |
| 4 | OFF | ON | ON | OFF | 1.000x | 203V | 196V |
| 5 | OFF | ON | ON | ON | 1.208x | 244V | 236V |
| 6 | ON | ON | ON | OFF | 1.441x | 295V | 282V |
| 7 | ON | ON | ON | ON | 1.742x | 352V | 340V |

## Protection Thresholds

| Protection | Trigger | Resume |
|------------|---------|--------|
| High-Cut | > 256V | < 249V |
| Low-Cut | < 181V | > 189V |

**Note**: Low-cut protection is only active when the Low-Cut Enable input (PC1) is held LOW.

## State Machine Architecture

The firmware uses three interconnected state machines:

1. **State Machine 0 (Startup)**: Initial relay positioning based on input voltage
2. **State Machine 1 (Measurement)**: Continuous ADC reading and voltage calculation
3. **State Machine 2 (Control)**: Relay step management and R5 protection logic

## Configuration Constants

Key parameters in `main.c`:

```c
#define DEFAULT_DELAY_TIME_SEC  180   // Startup delay (seconds)
#define HICUT_THRESHOLD         256.0f // High-cut trigger (V)
#define HICUT_RESUME            249.0f // High-cut resume (V)
#define LOCUT_THRESHOLD         181.0f // Low-cut trigger (V)
#define LOCUT_RESUME            189.0f // Low-cut resume (V)
#define CALIBRATION_VOLTAGE     244.0f // Reference voltage for calibration
```

## 5V Operation Critical Notes

This firmware includes specific optimizations for 5V operation:

1. **Flash Latency**: Must set 1 wait state for VDD > 3.6V at 24MHz
2. **SDI Disable**: Disabled to free PD1 for GPIO (prevents in-circuit debugging)
3. **Stabilization Delays**: Required for proper peripheral initialization

**Warning**: After flashing with SDI disabled, reprogramming requires a power cycle.

## Project Structure

```
Stablizer/
├── main.c                  # Main application code
├── ch32v00x.h              # Device header file
├── ch32v00x_conf.h         # Configuration includes
├── system_ch32v00x.c       # System initialization
├── ch32v00x_gpio.c/h       # GPIO driver
├── ch32v00x_rcc.c/h        # Clock control driver
├── ch32v00x_adc.c/h        # ADC driver
├── ch32v00x_tim.c/h        # Timer driver
├── ch32v00x_flash.c/h      # Flash memory driver
├── ch32v00x_it.c/h         # Interrupt handlers
├── ch32v00x_misc.c/h       # Miscellaneous utilities
├── README.md               # This file
├── HARDWARE.md             # Hardware setup guide
├── API.md                  # Code reference
├── CONTRIBUTING.md         # Contribution guidelines
└── LICENSE                 # MIT License
```

## Troubleshooting

| Problem | Solution |
|---------|----------|
| No operation at 5V | Ensure `Setup_Flash_For_5V()` is called first |
| ADC reads incorrect | Check calibration, verify sensor scaling |
| Relays not switching | Verify GPIO pin mapping matches hardware |
| Setting mode won't enter | Hold button before power-on, check PC4 wiring |
| Fault LED stays on | Voltage outside thresholds, check transformer taps |
| Random resets | Add decoupling capacitors, check power supply |

## Documentation

- [Hardware Setup Guide](HARDWARE.md) - Wiring diagrams and component selection
- [API Reference](API.md) - Function documentation and code examples
- [Contributing Guide](CONTRIBUTING.md) - How to contribute to this project

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Acknowledgments

- [WCH (Nanjing Qinheng Microelectronics)](http://www.wch.cn/) - CH32V003 manufacturer
- RISC-V Foundation - Open instruction set architecture
- MounRiver Studio - Development environment

## Author

Created by [Joyboy7385](https://github.com/Joyboy7385)

---

**Note**: This firmware is for automatic voltage regulation equipment. Ensure proper safety measures when working with mains voltage. Always follow local electrical codes and regulations.
