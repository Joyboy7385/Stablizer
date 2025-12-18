# API Reference

Complete function and data structure documentation for the CH32V003 Voltage Stabilizer project.

## Table of Contents

- [Constants and Definitions](#constants-and-definitions)
- [Data Structures](#data-structures)
- [Global Variables](#global-variables)
- [Core Functions](#core-functions)
- [ADC Functions](#adc-functions)
- [State Machine Functions](#state-machine-functions)
- [Relay Control Functions](#relay-control-functions)
- [Settings Functions](#settings-functions)
- [Utility Functions](#utility-functions)
- [Code Examples](#code-examples)

---

## Constants and Definitions

### Pin Definitions

Located in `main.c:16-27`

```c
// Port C Pins
#define PIN_R1           GPIO_Pin_0  // PC0 - Relay R1
#define PIN_LOWCUT_EN    GPIO_Pin_1  // PC1 - Low cut enable/disable
#define PIN_M_START      GPIO_Pin_3  // PC3 - M-START button
#define PIN_BUTTON       GPIO_Pin_4  // PC4 - Setting button
#define PIN_MAIN_LED     GPIO_Pin_7  // PC7 - Main LED

// Port D Pins
#define PIN_FAULT_LED    GPIO_Pin_1  // PD1 - Fault LED
#define PIN_SETTING_LED  GPIO_Pin_4  // PD4 - Setting LED
#define PIN_R4           GPIO_Pin_5  // PD5 - Relay R4
#define PIN_R3           GPIO_Pin_6  // PD6 - Relay R3
#define PIN_R2           GPIO_Pin_7  // PD7 - Relay R2

// Port A Pins
#define PIN_R5           GPIO_Pin_1  // PA1 - Relay R5 (Protection)
#define PIN_ADC_SENSE    GPIO_Pin_2  // PA2 - ADC Input
```

### Timing Configuration

Located in `main.c:30-45`

```c
#define DEFAULT_DELAY_TIME_SEC  180    // Default startup delay (seconds)
#define MIN_DELAY_TIME_SEC      3      // Minimum configurable delay
#define MAX_DELAY_TIME_SEC      180    // Maximum configurable delay
#define ADC_SAMPLES_COUNT       16     // Samples per ADC reading
#define ADC_DISCARD_SAMPLES     4      // High/low samples to discard
#define ADC_SETTLE_DELAY_US     100    // Delay between ADC samples
#define ADC_CAPTURE_COUNT       5      // Samples for calibration capture
#define DEBOUNCE_TIME_MS        10     // Relay debounce time
#define BUTTON_PRESS_TIME_MS    1000   // Long press duration
#define BLINK_FAST_MS           100    // Fast LED blink rate
#define BLINK_SLOW_MS           500    // Slow LED blink rate
#define BLINK_SETTING_MS        1000   // Setting mode blink rate
```

### Protection Thresholds

Located in `main.c:42-50`

```c
#define HICUT_DETECT_TIME_MS    500    // High-cut detection delay
#define HICUT_RESUME_TIME_MS    200    // High-cut resume delay
#define LOCUT_DETECT_TIME_MS    500    // Low-cut detection delay
#define LOCUT_RESUME_TIME_MS    200    // Low-cut resume delay
#define HICUT_THRESHOLD         256.0f // High-cut trigger voltage
#define HICUT_RESUME            249.0f // High-cut resume voltage
#define LOCUT_THRESHOLD         181.0f // Low-cut trigger voltage
#define LOCUT_RESUME            189.0f // Low-cut resume voltage
#define CALIBRATION_VOLTAGE     244.0f // Reference calibration voltage
```

### Flash Storage

Located in `main.c:51-53`

```c
#define FLASH_SETTINGS_ADDR     0x08001F80  // Settings storage address
#define SETTINGS_MAGIC          0xA5C3F0E1  // Magic number for validation
#define INITIAL_TAP_RATIO       0.472414f   // Tap ratio with all relays OFF
```

---

## Data Structures

### RelayStep_t

Located in `main.c:56-60`

Defines a single voltage regulation step.

```c
typedef struct {
    bool r1, r2, r3, r4;                    // Relay states
    uint16_t threshold_up, threshold_down;  // Voltage thresholds
    float tap_ratio;                        // Transformer tap ratio
} RelayStep_t;
```

| Field | Type | Description |
|-------|------|-------------|
| r1, r2, r3, r4 | bool | Individual relay states |
| threshold_up | uint16_t | Voltage to step up (V) |
| threshold_down | uint16_t | Voltage to step down (V) |
| tap_ratio | float | Output/Input voltage ratio |

### Settings_t

Located in `main.c:62-65`

Persistent settings stored in Flash.

```c
typedef struct {
    uint16_t adc_captured_a;    // Calibration ADC value
    uint32_t delay_time_ms;     // Startup delay (ms)
    uint32_t magic;             // Validation magic number
    uint32_t checksum;          // Data integrity checksum
} Settings_t;
```

### SystemState_t

Located in `main.c:67`

Main system operating states.

```c
typedef enum {
    STATE_NORMAL,   // Normal voltage regulation
    STATE_SETTING,  // Calibration/setting mode
    STATE_FAULT     // Fault condition (high/low cut)
} SystemState_t;
```

### SettingState_t

Located in `main.c:68`

Sub-states within setting mode.

```c
typedef enum {
    SETTING_IDLE,           // Not in setting mode
    SETTING_WAITING_DELAY,  // Waiting for delay input
    SETTING_WAITING_ADC     // Waiting for ADC calibration
} SettingState_t;
```

### R5State_t

Located in `main.c:69-70`

Protection relay (R5) state machine.

```c
typedef enum {
    R5_NORMAL,          // Normal operation, R5 engaged
    R5_HICUT_DETECTING, // Detecting high voltage condition
    R5_HICUT_ACTIVE,    // High-cut protection active
    R5_HICUT_RESUMING,  // Voltage returning to normal
    R5_LOCUT_DETECTING, // Detecting low voltage condition
    R5_LOCUT_ACTIVE,    // Low-cut protection active
    R5_LOCUT_RESUMING,  // Voltage returning to normal
    R5_DELAY_ACTIVE     // Startup delay in progress
} R5State_t;
```

### Relay Step Table

Located in `main.c:73-78`

Pre-defined voltage regulation steps.

```c
const RelayStep_t relaySteps[8] = {
    {0,0,0,0,  0,  0, 0.472414f},  // Step 0: All OFF
    {0,0,0,1,115,111,0.570833f},  // Step 1: R4 ON
    {0,0,1,0,139,135,0.689655f},  // Step 2: R3 ON
    {0,0,1,1,168,163,0.833333f},  // Step 3: R3+R4 ON
    {0,1,1,0,203,196,1.000000f},  // Step 4: R2+R3 ON (unity)
    {0,1,1,1,244,236,1.208333f},  // Step 5: R2+R3+R4 ON
    {1,1,1,0,295,282,1.441379f},  // Step 6: R1+R2+R3 ON
    {1,1,1,1,352,340,1.741667f}   // Step 7: All ON
};
```

---

## Global Variables

### System State Variables

```c
volatile uint32_t systemTick;           // 1ms system tick counter
volatile SystemState_t currentState;    // Current operating state
volatile SettingState_t settingState;   // Setting mode sub-state
volatile R5State_t r5State;             // Protection relay state
```

### Voltage Variables

```c
volatile uint16_t adcCapturedA;         // Calibration ADC value
volatile float currentOPV;              // Current output voltage
volatile float currentIPV;              // Current input voltage
```

### Relay Control Variables

```c
volatile uint8_t currentStep;           // Current relay step (0-7)
volatile uint8_t pendingStep;           // Pending step change
volatile bool r5Status;                 // R5 relay state
volatile bool stepChangePending;        // Step change in progress
volatile uint32_t relayChangeTimer;     // Debounce timer
volatile uint32_t r5Timer;              // R5 state timer
```

### ADC Filter Variables

```c
static uint32_t adcFilteredValue;       // Exponential filter state
static bool adcFilterInitialized;       // Filter initialization flag
```

---

## Core Functions

### Setup_Flash_For_5V()

Located in `main.c:132-141`

```c
void Setup_Flash_For_5V(void)
```

**Description**: Configures Flash memory latency for reliable 5V operation. **MUST be called first** before any other initialization.

**Critical**: Without this function, code execution fails above 3.7V due to Flash read errors.

**Details**:
- Clears and sets Flash latency to 1 wait state
- Required for VDD > 3.6V at 24MHz clock
- Includes stabilization delay

**Usage**:
```c
int main(void) {
    Setup_Flash_For_5V();  // MUST be first!
    // ... rest of initialization
}
```

---

### System_Init()

Located in `main.c:207-222`

```c
void System_Init(void)
```

**Description**: Initializes all system peripherals in the correct order.

**Initialization Sequence**:
1. Flash latency configuration (5V operation)
2. NVIC priority group setup
3. System clock update
4. GPIO initialization
5. ADC initialization
6. Timer initialization
7. NVIC interrupt configuration
8. Flash unlock for settings storage

---

### GPIO_Init_Custom()

Located in `main.c:224-272`

```c
void GPIO_Init_Custom(void)
```

**Description**: Configures all GPIO pins for relays, LEDs, buttons, and ADC.

**Pin Configuration**:
- Relay outputs: Push-pull, 50MHz
- LED outputs: Push-pull, 50MHz
- Button inputs: Internal pull-up
- ADC input: Analog mode

**Note**: Disables SDI to use PD1 as GPIO (Fault LED). After this, reprogramming requires power cycle.

---

### ADC_Init_Custom()

Located in `main.c:274-294`

```c
void ADC_Init_Custom(void)
```

**Description**: Configures ADC1 for voltage sensing.

**Configuration**:
- Mode: Independent, single conversion
- Clock: PCLK2/8
- Channel: 0 (PA2)
- Alignment: Right-aligned
- Includes automatic calibration

---

### TIM_Init_Custom()

Located in `main.c:296-306`

```c
void TIM_Init_Custom(void)
```

**Description**: Configures TIM2 for 1ms system tick.

**Configuration**:
- Period: 999 (1000 counts)
- Prescaler: SystemCoreClock/1000000 - 1
- Generates interrupt every 1ms

---

## ADC Functions

### ADC_ReadCount()

Located in `main.c:374-379`

```c
uint16_t ADC_ReadCount(void)
```

**Description**: Performs a single ADC conversion.

**Returns**: 10-bit ADC value (0-1023)

---

### ADC_ReadCount_Averaged()

Located in `main.c:381-406`

```c
uint16_t ADC_ReadCount_Averaged(void)
```

**Description**: Returns averaged ADC reading with outlier rejection.

**Algorithm**:
1. Collect 16 samples
2. Sort samples (bubble sort)
3. Discard 4 highest and 4 lowest
4. Average remaining 8 samples

**Returns**: Averaged 10-bit ADC value

---

### ADC_ReadCount_Filtered()

Located in `main.c:408-419`

```c
uint16_t ADC_ReadCount_Filtered(void)
```

**Description**: Returns exponentially filtered ADC reading.

**Filter**: `output = (2 * new + 8 * old) / 10`

**Returns**: Filtered ADC value (smooth, lag-compensated)

---

### ADC_Capture_Calibration()

Located in `main.c:421-438`

```c
uint16_t ADC_Capture_Calibration(void)
```

**Description**: Captures calibration ADC value using median selection.

**Algorithm**:
1. Capture 5 averaged readings
2. Sort readings
3. Return median value

**Returns**: Calibration ADC value

---

### Calculate_OPV()

Located in `main.c:440-443`

```c
float Calculate_OPV(uint16_t adc)
```

**Description**: Converts ADC reading to output voltage.

**Formula**: `OPV = (adc / adcCapturedA) * CALIBRATION_VOLTAGE`

**Parameters**:
| Parameter | Type | Description |
|-----------|------|-------------|
| adc | uint16_t | Current ADC reading |

**Returns**: Output voltage in volts (float)

---

## State Machine Functions

### StateMachine0_Initial_Startup()

Located in `main.c:188-204`

```c
void StateMachine0_Initial_Startup(void)
```

**Description**: Positions relays based on initial voltage reading at startup.

**Algorithm**:
1. Read averaged ADC value
2. Calculate output voltage
3. Estimate input voltage using initial tap ratio
4. Find appropriate relay step
5. Apply relay configuration

---

### StateMachine1_Calculate_Voltages()

Located in `main.c:446-450`

```c
void StateMachine1_Calculate_Voltages(void)
```

**Description**: Continuously updates voltage measurements.

**Updates**:
- `currentOPV`: Output voltage (from ADC)
- `currentIPV`: Input voltage (OPV * tap_ratio)

---

### StateMachine2_Control_R1_R4()

Located in `main.c:453-487`

```c
void StateMachine2_Control_R1_R4(void)
```

**Description**: Controls tap-changing relays (R1-R4) with debouncing.

**Features**:
- Hysteresis between step-up and step-down thresholds
- 10ms debounce timer prevents oscillation
- Supports multi-step jumps for rapid voltage changes

---

### StateMachine2_Control_R5()

Located in `main.c:489-576`

```c
void StateMachine2_Control_R5(void)
```

**Description**: Controls protection relay (R5) state machine.

**States**:
- Normal operation
- High-cut detection and activation
- Low-cut detection and activation
- Resume from fault conditions
- Startup delay management

---

## Relay Control Functions

### Apply_Relay_Step()

Located in `main.c:578-585`

```c
void Apply_Relay_Step(uint8_t step)
```

**Description**: Sets relay outputs to match a specific step.

**Parameters**:
| Parameter | Type | Description |
|-----------|------|-------------|
| step | uint8_t | Step index (0-7) |

**Usage**:
```c
Apply_Relay_Step(4);  // Set to unity gain (1.0x)
```

---

### Set_R5_Relay()

Located in `main.c:587-590`

```c
void Set_R5_Relay(bool state)
```

**Description**: Controls the protection relay (R5).

**Parameters**:
| Parameter | Type | Description |
|-----------|------|-------------|
| state | bool | true = engaged, false = disconnected |

---

## Settings Functions

### Load_Settings()

Located in `main.c:329-341`

```c
void Load_Settings(void)
```

**Description**: Loads settings from Flash memory with validation.

**Validation**:
- Magic number check
- Checksum verification
- Range validation for values

---

### Save_Settings()

Located in `main.c:343-354`

```c
void Save_Settings(void)
```

**Description**: Saves current settings to Flash memory.

**Storage**:
- ADC calibration value
- Delay time
- Magic number
- Checksum

---

### Clear_Settings()

Located in `main.c:356-360`

```c
void Clear_Settings(void)
```

**Description**: Clears all stored settings and erases Flash page.

---

### Calculate_Checksum()

Located in `main.c:325-327`

```c
uint32_t Calculate_Checksum(Settings_t* s)
```

**Description**: Calculates simple additive checksum for settings validation.

---

## Utility Functions

### raw_Delay_Ms()

```c
void raw_Delay_Ms(uint32_t ms)
```

**Description**: Blocking delay in milliseconds using system tick.

---

### raw_Delay_Us()

```c
void raw_Delay_Us(uint32_t us)
```

**Description**: Blocking delay in microseconds using NOP loop.

---

### LED_Set()

```c
void LED_Set(GPIO_TypeDef* port, uint16_t pin, bool state)
```

**Description**: Sets an LED to a specific state.

**Parameters**:
| Parameter | Type | Description |
|-----------|------|-------------|
| port | GPIO_TypeDef* | GPIO port (GPIOC, GPIOD) |
| pin | uint16_t | GPIO pin mask |
| state | bool | true = ON, false = OFF |

---

### LED_Handle_Blinking()

```c
void LED_Handle_Blinking(void)
```

**Description**: Handles LED blinking states for fault indication.

---

## Code Examples

### Example 1: Basic Initialization

```c
int main(void) {
    // CRITICAL: Flash latency first for 5V
    Setup_Flash_For_5V();

    // Initialize peripherals
    System_Init();

    // Load calibration from Flash
    Load_Settings();

    // Initial relay positioning
    if(adcCapturedA > 0) {
        StateMachine0_Initial_Startup();
    }

    // Main loop
    while(1) {
        StateMachine1_Calculate_Voltages();
        StateMachine2_Control_R1_R4();
        StateMachine2_Control_R5();
        raw_Delay_Ms(10);
    }
}
```

### Example 2: Manual Relay Control

```c
// Set specific relay step
Apply_Relay_Step(4);  // Unity gain

// Control protection relay
Set_R5_Relay(true);   // Engage
Set_R5_Relay(false);  // Disconnect
```

### Example 3: Voltage Reading

```c
// Single reading
uint16_t raw = ADC_ReadCount();

// Averaged reading (noise-filtered)
uint16_t avg = ADC_ReadCount_Averaged();

// Smoothed reading (exponential filter)
uint16_t smooth = ADC_ReadCount_Filtered();

// Convert to voltage
float voltage = Calculate_OPV(smooth);
```

### Example 4: Checking Protection Status

```c
// Check high-cut condition
if(currentOPV > HICUT_THRESHOLD) {
    // Over-voltage detected
    Set_R5_Relay(false);  // Disconnect load
    currentState = STATE_FAULT;
}

// Check low-cut condition (if enabled)
bool lowcut_enabled = !GPIO_ReadInputDataBit(GPIOC, PIN_LOWCUT_EN);
if(lowcut_enabled && currentOPV < LOCUT_THRESHOLD) {
    // Under-voltage detected
    Set_R5_Relay(false);
    currentState = STATE_FAULT;
}
```

---

## Interrupt Handlers

### TIM2_IRQHandler()

Located in `main.c:318-323`

```c
void TIM2_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
```

**Description**: Timer 2 interrupt handler, increments `systemTick` every 1ms.

**Note**: Uses WCH fast interrupt attribute for minimal latency.

---

## Memory Map

### Flash Usage

| Region | Address | Size | Purpose |
|--------|---------|------|---------|
| Code | 0x08000000 | ~4 KB | Application firmware |
| Settings | 0x08001F80 | 16 bytes | Persistent settings |
| Free | - | ~12 KB | Unused |

### RAM Usage

| Variable | Size | Purpose |
|----------|------|---------|
| systemTick | 4 bytes | Tick counter |
| State variables | ~20 bytes | Operating states |
| ADC filter | 8 bytes | Filter state |
| Stack | ~256 bytes | Function calls |
| **Total** | ~300 bytes | |

---

## Performance Notes

### Timing Analysis

| Operation | Duration | Notes |
|-----------|----------|-------|
| ADC_ReadCount() | ~250 us | Single conversion |
| ADC_ReadCount_Averaged() | ~6 ms | 16 samples + sorting |
| ADC_ReadCount_Filtered() | ~6 ms | Averaged + filter |
| Apply_Relay_Step() | ~5 us | 4 GPIO writes |
| Main loop iteration | ~10 ms | Configurable delay |

### ADC Resolution

- 10-bit ADC: 0-1023 counts
- Effective resolution: ~0.24V per count (at 244V calibration)
- Noise floor: ~2-3 counts (with averaging)
