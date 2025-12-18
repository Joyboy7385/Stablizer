# CH32V003 7-Segment Display Test

A demonstration project for controlling a 3-digit 7-segment LED display using the **CH32V003A4M6** RISC-V microcontroller. Optimized for stable operation across a wide voltage range (2.5V to 5.5V).

## Features

- 3-digit multiplexed 7-segment display control
- Common anode display support
- Robust 5V operation with proper Flash latency configuration
- Clean initialization sequence preventing GPIO conflicts
- Anti-ghosting blanking delays
- Minimal dependencies using WCH standard peripheral library

## Hardware Requirements

| Component | Specification |
|-----------|---------------|
| Microcontroller | CH32V003A4M6 (RISC-V 32-bit) |
| Display | 3-digit 7-segment LED (Common Anode) |
| Supply Voltage | 2.5V - 5.5V |
| Clock | 24 MHz HSI (Internal) |

## Pin Configuration

### Segment Pins

| Segment | Pin | Port |
|---------|-----|------|
| A | PD1 | Port D |
| B | PD6 | Port D |
| C | PC2 | Port C |
| D | PC1 | Port C |
| E | PC3 | Port C |
| F | PC6 | Port C |
| G | PC0 | Port C |

### Digit Control Pins

| Digit | Pin | Port |
|-------|-----|------|
| Digit 1 | PC7 | Port C |
| Digit 2 | PD5 | Port D |
| Digit 3 | PD7 | Port D |

### Button Inputs (Optional)

| Function | Pin |
|----------|-----|
| Button 1 | PA1 |
| Button 2 | PA2 |

## Quick Start

### 1. Clone the Repository

```bash
git clone https://github.com/Joyboy7385/Display-Test.git
cd Display-Test
```

### 2. Build the Project

Use MounRiver Studio or the WCH RISC-V toolchain:

```bash
# Using MounRiver Studio
# Open the project and click Build

# Or using command line with RISC-V GCC
riscv-none-embed-gcc -march=rv32ec -mabi=ilp32e -O2 -o display_test.elf main.c system_ch32v00x.c ch32v00x_gpio.c ch32v00x_rcc.c
```

### 3. Flash the Firmware

Use WCH-Link programmer:

```bash
# Using WCH ISP tool or MounRiver Studio programmer
wch-isp -p display_test.hex
```

## How It Works

### Display Multiplexing

The display uses time-division multiplexing to control all 3 digits with limited GPIO pins:

1. **Turn off all digits** - Prevents ghosting
2. **Set segment pattern** - Configure which segments to light
3. **Enable one digit** - Turn on only the target digit
4. **Hold briefly** - Display for ~1.5ms
5. **Repeat for next digit** - Cycle through all digits at ~200Hz

### 7-Segment Digit Patterns

```
    A
   ---
F |   | B
   -G-
E |   | C
   ---
    D
```

| Digit | A | B | C | D | E | F | G | Hex |
|-------|---|---|---|---|---|---|---|-----|
| 0 | 1 | 1 | 1 | 1 | 1 | 1 | 0 | 0x3F |
| 1 | 0 | 1 | 1 | 0 | 0 | 0 | 0 | 0x06 |
| 2 | 1 | 1 | 0 | 1 | 1 | 0 | 1 | 0x5B |
| 3 | 1 | 1 | 1 | 1 | 0 | 0 | 1 | 0x4F |
| 4 | 0 | 1 | 1 | 0 | 0 | 1 | 1 | 0x66 |
| 5 | 1 | 0 | 1 | 1 | 0 | 1 | 1 | 0x6D |
| 6 | 1 | 0 | 1 | 1 | 1 | 1 | 1 | 0x7D |
| 7 | 1 | 1 | 1 | 0 | 0 | 0 | 0 | 0x07 |
| 8 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 0x7F |
| 9 | 1 | 1 | 1 | 1 | 0 | 1 | 1 | 0x6F |

### Common Anode Logic

For common anode displays, the logic is inverted:
- **Segment ON**: GPIO LOW (0)
- **Segment OFF**: GPIO HIGH (1)
- **Digit ON**: GPIO HIGH (1)
- **Digit OFF**: GPIO LOW (0)

## Demo Sequence

The default firmware runs a simple demonstration:

1. Display "1" on digit 1 (1 second)
2. Display "2" on digit 2 (1 second)
3. Display "3" on digit 3 (1 second)
4. Display "888" on all digits (1 second)
5. Loop forever

## Project Structure

```
Display-Test/
├── main.c                  # Main application code
├── ch32v00x.h              # Device header file
├── ch32v00x_conf.h         # Configuration includes
├── system_ch32v00x.c       # System initialization
├── system_ch32v00x.h       # System header
├── ch32v00x_gpio.c         # GPIO driver
├── ch32v00x_gpio.h         # GPIO header
├── ch32v00x_rcc.c          # Clock control driver
├── ch32v00x_rcc.h          # Clock header
├── ch32v00x_flash.c        # Flash memory driver
├── ch32v00x_flash.h        # Flash header
├── ch32v00x_it.c           # Interrupt handlers
├── ch32v00x_it.h           # Interrupt header
├── ch32v00x_misc.c         # Miscellaneous utilities
├── ch32v00x_misc.h         # Misc header
├── README.md               # This file
├── HARDWARE.md             # Hardware setup guide
├── API.md                  # Code reference
└── CONTRIBUTING.md         # Contribution guidelines
```

## 5V Stability Features

This project includes specific optimizations for reliable 5V operation:

1. **Flash Latency Configuration**
   - Sets 1 wait state for VDD > 3.6V at 24MHz
   - Prevents read errors at higher voltages

2. **SDI Disable Timing**
   - Disables Serial Debug Interface before delay initialization
   - Prevents pin conflicts with GPIO

3. **Stabilization Delays**
   - Power supply settling time (50,000 cycles)
   - Proper sequencing for PD7 (shared with NRST)

## Troubleshooting

| Problem | Solution |
|---------|----------|
| Display not lighting | Check common anode connections, verify logic levels |
| Flickering display | Increase digit hold time, check power supply stability |
| Wrong digits shown | Verify pin assignments match your hardware |
| No response at 5V | Ensure Flash latency is configured (see `Setup_Flash_For_5V()`) |
| Random resets | Check PD7 configuration (shared with NRST pin) |

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

**Note**: This project is for educational and demonstration purposes. Ensure proper ESD protection when handling the microcontroller.
