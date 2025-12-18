# API Reference

Complete function and data structure documentation for the CH32V003 7-Segment Display project.

## Table of Contents

- [Constants and Definitions](#constants-and-definitions)
- [Global Variables](#global-variables)
- [Core Functions](#core-functions)
- [Display Functions](#display-functions)
- [Utility Functions](#utility-functions)
- [WCH Library Functions Used](#wch-library-functions-used)
- [Code Examples](#code-examples)

---

## Constants and Definitions

### Segment Pin Definitions

Located in `main.c:12-18`

```c
#define SEG_A_PIN       GPIO_Pin_1      // PD1
#define SEG_B_PIN       GPIO_Pin_6      // PD6
#define SEG_C_PIN       GPIO_Pin_2      // PC2
#define SEG_D_PIN       GPIO_Pin_1      // PC1
#define SEG_E_PIN       GPIO_Pin_3      // PC3
#define SEG_F_PIN       GPIO_Pin_6      // PC6
#define SEG_G_PIN       GPIO_Pin_0      // PC0
```

### Digit Control Pin Definitions

Located in `main.c:20-22`

```c
#define DIGIT1_PIN      GPIO_Pin_7      // PC7
#define DIGIT2_PIN      GPIO_Pin_5      // PD5
#define DIGIT3_PIN      GPIO_Pin_7      // PD7
```

### Digit Patterns

Located in `main.c:25-27`

```c
static const uint8_t DIGIT_PAT[10] = {
    0x3F,  // 0: segments A,B,C,D,E,F
    0x06,  // 1: segments B,C
    0x5B,  // 2: segments A,B,D,E,G
    0x4F,  // 3: segments A,B,C,D,G
    0x66,  // 4: segments B,C,F,G
    0x6D,  // 5: segments A,C,D,F,G
    0x7D,  // 6: segments A,C,D,E,F,G
    0x07,  // 7: segments A,B,C
    0x7F,  // 8: segments A,B,C,D,E,F,G
    0x6F   // 9: segments A,B,C,D,F,G
};
```

**Bit Mapping:**

| Bit | Segment |
|-----|---------|
| 0 | A |
| 1 | B |
| 2 | C |
| 3 | D |
| 4 | E |
| 5 | F |
| 6 | G |
| 7 | DP (unused) |

---

## Global Variables

### segPattern

Located in `main.c:29`

```c
static uint8_t segPattern[3] = {0, 0, 0};
```

**Description:** Array holding the current segment pattern for each digit.

| Index | Digit Position |
|-------|----------------|
| 0 | Left (Digit 1) |
| 1 | Middle (Digit 2) |
| 2 | Right (Digit 3) |

**Usage:**
```c
// Display "123"
segPattern[0] = DIGIT_PAT[1];  // Left digit shows 1
segPattern[1] = DIGIT_PAT[2];  // Middle digit shows 2
segPattern[2] = DIGIT_PAT[3];  // Right digit shows 3
```

---

## Core Functions

### Setup_Flash_For_5V()

Located in `main.c:35-43`

```c
void Setup_Flash_For_5V(void)
```

**Description:** Configures Flash memory latency for reliable operation at 5V. Must be called first before any other initialization.

**Parameters:** None

**Returns:** None

**Details:**
- Sets Flash wait state to 1 (required for VDD > 3.6V at 24MHz)
- Includes 50,000 cycle stabilization delay
- Critical for preventing read errors at higher voltages

**Usage:**
```c
int main(void) {
    Setup_Flash_For_5V();  // Must be first!
    // ... rest of initialization
}
```

---

### GPIO_Init_All()

Located in `main.c:45-92`

```c
void GPIO_Init_All(void)
```

**Description:** Initializes all GPIO pins for display control.

**Parameters:** None

**Returns:** None

**Details:**
1. Enables clocks for GPIOC and GPIOD
2. Configures all segment pins as push-pull outputs at 50MHz
3. Configures all digit control pins as push-pull outputs
4. Special handling for PD7 (NRST pin):
   - Separate initialization
   - "Wake-up" pulse sequence
   - Additional stabilization delay
5. Sets initial state:
   - All segments OFF (HIGH for common anode)
   - All digits OFF (LOW for common anode)

**Usage:**
```c
// After SDI disable and Delay_Init()
GPIO_Init_All();
```

---

## Display Functions

### SetSegment()

Located in `main.c:94-104`

```c
void SetSegment(uint8_t pattern)
```

**Description:** Sets the segment pins to display a given pattern.

**Parameters:**
| Parameter | Type | Description |
|-----------|------|-------------|
| pattern | uint8_t | 7-bit segment pattern (see DIGIT_PAT) |

**Returns:** None

**Details:**
- Applies pattern to all 7 segment pins
- Handles common anode logic inversion (LOW = ON)
- Pattern bit 0 = Segment A, bit 1 = Segment B, etc.

**Usage:**
```c
SetSegment(DIGIT_PAT[5]);  // Display digit 5
SetSegment(0x7F);          // All segments ON (shows 8)
SetSegment(0x00);          // All segments OFF (blank)
```

---

### Display_Refresh()

Located in `main.c:106-137`

```c
void Display_Refresh(void)
```

**Description:** Performs one iteration of display multiplexing. Must be called repeatedly to maintain visible display.

**Parameters:** None

**Returns:** None

**Timing:**
| Phase | Duration |
|-------|----------|
| Blanking | 30 µs |
| Display | 1500 µs |
| **Total per digit** | ~1530 µs |
| **Full cycle (3 digits)** | ~5 ms |

**Algorithm:**
1. Turn off all digits
2. Wait 30µs (blanking to prevent ghosting)
3. Set segment pattern for current digit
4. Turn on current digit
5. Hold for 1500µs
6. Increment to next digit (0 → 1 → 2 → 0...)

**Note:** Current implementation cycles through digits 0 and 1 only (bug in line 136).

**Usage:**
```c
// Continuous display loop
while(1) {
    Display_Refresh();
}
```

---

### ShowFor()

Located in `main.c:139-147`

```c
void ShowFor(uint16_t ms)
```

**Description:** Displays the current segment patterns for a specified duration.

**Parameters:**
| Parameter | Type | Description |
|-----------|------|-------------|
| ms | uint16_t | Display duration in milliseconds |

**Returns:** None

**Details:**
- Calculates refresh cycles needed for specified duration
- Calls Display_Refresh() 3 times per cycle (for 3 digits)
- Approximate timing (due to overhead): ±5%

**Usage:**
```c
// Set up pattern first
segPattern[0] = DIGIT_PAT[1];
segPattern[1] = DIGIT_PAT[2];
segPattern[2] = DIGIT_PAT[3];

// Show "123" for 2 seconds
ShowFor(2000);
```

---

## Utility Functions

### WCH Library Delay Functions

These functions are provided by `debug.h`:

#### Delay_Init()

```c
void Delay_Init(void)
```

Initializes the SysTick timer for delay functions. Must be called after SDI disable.

#### Delay_Us()

```c
void Delay_Us(uint32_t n)
```

Blocking delay in microseconds.

| Parameter | Type | Description |
|-----------|------|-------------|
| n | uint32_t | Delay in microseconds |

#### Delay_Ms()

```c
void Delay_Ms(uint32_t n)
```

Blocking delay in milliseconds.

| Parameter | Type | Description |
|-----------|------|-------------|
| n | uint32_t | Delay in milliseconds |

---

## WCH Library Functions Used

### GPIO Functions

From `ch32v00x_gpio.h`:

#### GPIO_Init()

```c
void GPIO_Init(GPIO_TypeDef* GPIOx, GPIO_InitTypeDef* GPIO_InitStruct)
```

Initializes GPIO pins according to the specified parameters.

#### GPIO_SetBits()

```c
void GPIO_SetBits(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin)
```

Sets the selected pins HIGH.

#### GPIO_ResetBits()

```c
void GPIO_ResetBits(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin)
```

Sets the selected pins LOW.

#### GPIO_WriteBit()

```c
void GPIO_WriteBit(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, BitAction BitVal)
```

Writes a specific value to a pin.

#### GPIO_ReadInputDataBit()

```c
uint8_t GPIO_ReadInputDataBit(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin)
```

Reads a specific input pin.

#### GPIO_PinRemapConfig()

```c
void GPIO_PinRemapConfig(uint32_t GPIO_Remap, FunctionalState NewState)
```

Configures pin remapping (used for SDI disable).

### Clock Functions

From `ch32v00x_rcc.h`:

#### RCC_APB2PeriphClockCmd()

```c
void RCC_APB2PeriphClockCmd(uint32_t RCC_APB2Periph, FunctionalState NewState)
```

Enables or disables APB2 peripheral clocks.

---

## Code Examples

### Example 1: Display a Static Number

```c
// Display "42" (left-aligned)
void display_42(void) {
    segPattern[0] = DIGIT_PAT[4];
    segPattern[1] = DIGIT_PAT[2];
    segPattern[2] = 0x00;  // Blank right digit

    // Display for 5 seconds
    ShowFor(5000);
}
```

### Example 2: Counting Display

```c
// Count from 0 to 999
void counting_demo(void) {
    for(uint16_t num = 0; num < 1000; num++) {
        // Extract digits
        segPattern[0] = DIGIT_PAT[num / 100];        // Hundreds
        segPattern[1] = DIGIT_PAT[(num / 10) % 10];  // Tens
        segPattern[2] = DIGIT_PAT[num % 10];         // Ones

        // Display for 100ms
        ShowFor(100);
    }
}
```

### Example 3: Blinking Display

```c
// Blink "888" on and off
void blink_demo(void) {
    while(1) {
        // Display "888"
        segPattern[0] = DIGIT_PAT[8];
        segPattern[1] = DIGIT_PAT[8];
        segPattern[2] = DIGIT_PAT[8];
        ShowFor(500);

        // Blank display
        segPattern[0] = 0x00;
        segPattern[1] = 0x00;
        segPattern[2] = 0x00;
        ShowFor(500);
    }
}
```

### Example 4: Custom Segment Patterns

```c
// Create custom characters
#define CHAR_H  0x76  // H: segments B,C,E,F,G
#define CHAR_E  0x79  // E: segments A,D,E,F,G
#define CHAR_L  0x38  // L: segments D,E,F
#define CHAR_P  0x73  // P: segments A,B,E,F,G

// Display "HEL" then "P" (rotating)
void hello_demo(void) {
    // Show "HEL"
    segPattern[0] = CHAR_H;
    segPattern[1] = CHAR_E;
    segPattern[2] = CHAR_L;
    ShowFor(1000);

    // Show "ELP"
    segPattern[0] = CHAR_E;
    segPattern[1] = CHAR_L;
    segPattern[2] = CHAR_P;
    ShowFor(1000);
}
```

### Example 5: Initialization Template

```c
int main(void) {
    // ========== INITIALIZATION SEQUENCE ==========
    // Order is critical for 5V stability!

    // 1. Flash configuration (MUST be first)
    Setup_Flash_For_5V();

    // 2. Enable AFIO clock
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);

    // 3. Disable SDI (BEFORE Delay_Init)
    GPIO_PinRemapConfig(GPIO_Remap_SDI_Disable, ENABLE);
    for(volatile uint32_t i=0; i<50000; i++);  // Stabilization

    // 4. Initialize delay functions
    Delay_Init();

    // 5. Initialize display GPIO
    GPIO_Init_All();

    // ========== MAIN LOOP ==========
    while(1) {
        // Your display code here
        Display_Refresh();
    }

    return 0;
}
```

---

## Error Handling

### Common Issues and Solutions

| Issue | Cause | Solution |
|-------|-------|----------|
| No display at 5V | Missing Flash latency | Call `Setup_Flash_For_5V()` first |
| Random behavior | SDI not disabled | Disable before `Delay_Init()` |
| Digit 3 doesn't work | PD7/NRST conflict | Use special initialization sequence |
| Ghosting | Insufficient blanking | Increase blanking delay in `Display_Refresh()` |
| Dim display | Low refresh rate | Reduce display hold time or optimize code |

---

## Performance Notes

### Timing Analysis

| Operation | Duration | Notes |
|-----------|----------|-------|
| `SetSegment()` | ~5 µs | 7 GPIO writes |
| `Display_Refresh()` | ~1.5 ms | Per digit |
| Full refresh cycle | ~5 ms | All 3 digits |
| Refresh rate | ~200 Hz | Above flicker threshold |

### Memory Usage

| Resource | Usage |
|----------|-------|
| Flash | ~2 KB (with WCH library) |
| SRAM | ~100 bytes |
| Stack | ~256 bytes (recommended) |

---

## Version History

| Version | Date | Changes |
|---------|------|---------|
| v2 | Current | Fixed 5V stability, SDI timing |
| v1 | Initial | Basic display functionality |
