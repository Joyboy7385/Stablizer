/*
 * ============================================================================
 * VOLTAGE STABILIZER - FIXED FOR 5V OPERATION - PART 1 of 3
 * ============================================================================
 * CRITICAL FIX: Added Flash latency configuration for VDD > 3.6V
 * With STATE MACHINE 0 for immediate startup positioning
 * CORRECTED PIN MAPPING + 5V SUPPLY WORKING
 * Copy Part 1 + Part 2 + Part 3 into one file
 * ============================================================================
 */

#include "ch32v00x.h"
#include <stdbool.h>

// PIN DEFINITIONS - CORRECTED TO MATCH YOUR HARDWARE
#define PIN_LOWCUT_EN    GPIO_Pin_1  // PC1 - Low cut enable/disable
#define PIN_M_START      GPIO_Pin_3  // PC3 - M-START button
#define PIN_BUTTON       GPIO_Pin_4  // PC4 - Setting button
#define PIN_MAIN_LED     GPIO_Pin_7  // PC7 - Main LED
#define PIN_R1           GPIO_Pin_0  // PC0 - Relay R1
#define PIN_FAULT_LED    GPIO_Pin_1  // PD1 - Fault LED
#define PIN_SETTING_LED  GPIO_Pin_4  // PD4 - Setting LED
#define PIN_R4           GPIO_Pin_5  // PD5 - Relay R4
#define PIN_R3           GPIO_Pin_6  // PD6 - Relay R3
#define PIN_R2           GPIO_Pin_7  // PD7 - Relay R2
#define PIN_R5           GPIO_Pin_1  // PA1 - Relay R5
#define PIN_ADC_SENSE    GPIO_Pin_2  // PA2 - ADC Input

// CONFIGURATION
#define DEFAULT_DELAY_TIME_SEC  180
#define MIN_DELAY_TIME_SEC      3
#define MAX_DELAY_TIME_SEC      180
#define ADC_SAMPLES_COUNT       16
#define ADC_DISCARD_SAMPLES     4
#define ADC_SETTLE_DELAY_US     100
#define ADC_CAPTURE_COUNT       5
#define DEBOUNCE_TIME_MS        10
#define BUTTON_PRESS_TIME_MS    1000
#define BLINK_FAST_MS           100
#define BLINK_SLOW_MS           500
#define BLINK_SETTING_MS        1000
#define HICUT_DETECT_TIME_MS    500
#define HICUT_RESUME_TIME_MS    200
#define LOCUT_DETECT_TIME_MS    500
#define LOCUT_RESUME_TIME_MS    200
#define HICUT_THRESHOLD         256.0f
#define HICUT_RESUME            249.0f
#define LOCUT_THRESHOLD         181.0f
#define LOCUT_RESUME            189.0f
#define CALIBRATION_VOLTAGE     244.0f
#define FLASH_SETTINGS_ADDR     0x08001F80
#define SETTINGS_MAGIC          0xA5C3F0E1
#define INITIAL_TAP_RATIO       0.472414f  // 137V/290V (all relays OFF)

// DATA STRUCTURES
typedef struct {
    bool r1, r2, r3, r4;
    uint16_t threshold_up, threshold_down;
    float tap_ratio;
} RelayStep_t;

typedef struct {
    uint16_t adc_captured_a;
    uint32_t delay_time_ms, magic, checksum;
} Settings_t;

typedef enum { STATE_NORMAL, STATE_SETTING, STATE_FAULT } SystemState_t;
typedef enum { SETTING_IDLE, SETTING_WAITING_DELAY, SETTING_WAITING_ADC } SettingState_t;
typedef enum { R5_NORMAL, R5_HICUT_DETECTING, R5_HICUT_ACTIVE, R5_HICUT_RESUMING,
               R5_LOCUT_DETECTING, R5_LOCUT_ACTIVE, R5_LOCUT_RESUMING, R5_DELAY_ACTIVE } R5State_t;

// RELAY STEP TABLE
const RelayStep_t relaySteps[8] = {
    {0,0,0,0,  0,  0,0.472414f}, {0,0,0,1,115,111,0.570833f},
    {0,0,1,0,139,135,0.689655f}, {0,0,1,1,168,163,0.833333f},
    {0,1,1,0,203,196,1.000000f}, {0,1,1,1,244,236,1.208333f},
    {1,1,1,0,295,282,1.441379f}, {1,1,1,1,352,340,1.741667f}
};

// GLOBAL VARIABLES
volatile uint32_t systemTick=0;
volatile SystemState_t currentState=STATE_NORMAL;
volatile SettingState_t settingState=SETTING_IDLE;
volatile R5State_t r5State=R5_NORMAL;
volatile uint16_t adcCapturedA=0;
volatile float currentOPV=0.0f, currentIPV=0.0f;
volatile uint8_t currentStep=0, pendingStep=0;
volatile bool r5Status=false, stepChangePending=false;
volatile uint32_t relayChangeTimer=0, r5Timer=0;
volatile uint32_t delayTimeMs=DEFAULT_DELAY_TIME_SEC*1000;
volatile uint32_t delayCountStart=0, settingBlinkTimer=0;
volatile bool settingLedState=false;
volatile uint32_t buttonPressStart=0, mstartPressStart=0;
volatile bool buttonWasPressed=false, mstartWasPressed=false;
volatile uint32_t ledBlinkTimer=0;
volatile bool ledBlinkState=false;
static uint32_t adcFilteredValue=0;
static bool adcFilterInitialized=false;

// FUNCTION PROTOTYPES
void Setup_Flash_For_5V(void);
void System_Init(void);
void GPIO_Init_Custom(void);
void ADC_Init_Custom(void);
void TIM_Init_Custom(void);
void NVIC_Init_Custom(void);
void Load_Settings(void);
void Save_Settings(void);
void Clear_Settings(void);
uint32_t Calculate_Checksum(Settings_t* s);
uint16_t ADC_ReadCount(void);
uint16_t ADC_ReadCount_Averaged(void);
uint16_t ADC_ReadCount_Filtered(void);
uint16_t ADC_Capture_Calibration(void);
float Calculate_OPV(uint16_t adc);
void StateMachine0_Initial_Startup(void);
void StateMachine1_Calculate_Voltages(void);
void StateMachine2_Control_R1_R4(void);
void StateMachine2_Control_R5(void);
void Apply_Relay_Step(uint8_t step);
void Set_R5_Relay(bool state);
void Enter_Setting_Mode(void);
void Handle_Setting_Mode(void);
bool Check_Button_Pressed(void);
bool Check_MStart_Pressed(void);
void LED_Set(GPIO_TypeDef* port, uint16_t pin, bool state);
void LED_Handle_Blinking(void);
void raw_Delay_Ms(uint32_t ms);
void raw_Delay_Us(uint32_t us);

// CRITICAL: Setup Flash Latency for 5V Operation
void Setup_Flash_For_5V(void) {
    // At VDD > 3.6V with 24MHz clock, Flash needs 1 wait state
    // Without this, code execution fails above 3.7V!
    
    FLASH->ACTLR &= ~(FLASH_ACTLR_LATENCY);     // Clear latency bits
    FLASH->ACTLR |= FLASH_ACTLR_LATENCY_1;     // Set 1 wait state
    
    // Small delay for Flash controller to apply settings
    for(volatile uint32_t i=0; i<10000; i++);
}

// MAIN FUNCTION
int main(void) {
    System_Init();
    Load_Settings();
    raw_Delay_Ms(10);
    
    if(!GPIO_ReadInputDataBit(GPIOC,PIN_BUTTON)) {
        raw_Delay_Ms(990);
        if(!GPIO_ReadInputDataBit(GPIOC,PIN_BUTTON)) Enter_Setting_Mode();
    }
    
    if(currentState==STATE_NORMAL && adcCapturedA>0) {
        StateMachine0_Initial_Startup();
        r5State=R5_DELAY_ACTIVE;
        r5Timer=systemTick;
        LED_Set(GPIOC,PIN_MAIN_LED,false);
    } else if(adcCapturedA==0) {
        LED_Set(GPIOD,PIN_SETTING_LED,true);
    }
    
    while(1) {
        if(adcCapturedA>0) StateMachine1_Calculate_Voltages();
        
        switch(currentState) {
            case STATE_NORMAL:
                if(r5State!=R5_DELAY_ACTIVE) LED_Set(GPIOC,PIN_MAIN_LED,true);
                LED_Set(GPIOD,PIN_SETTING_LED,false);
                if(adcCapturedA>0) {
                    StateMachine2_Control_R1_R4();
                    StateMachine2_Control_R5();
                }
                LED_Handle_Blinking();
                break;
            case STATE_SETTING:
                Handle_Setting_Mode();
                break;
            case STATE_FAULT:
                LED_Handle_Blinking();
                break;
        }
        raw_Delay_Ms(10);
    }
}

// STATE MACHINE 0 - INITIAL STARTUP
void StateMachine0_Initial_Startup(void) {
    uint16_t adc = ADC_ReadCount_Averaged();
    float opv = Calculate_OPV(adc);
    float initial_ipv = opv * INITIAL_TAP_RATIO;
    
    uint8_t target_step = 0;
    for(int step = 7; step >= 0; step--) {
        if(initial_ipv > relaySteps[step].threshold_up) {
            target_step = step;
            break;
        }
    }
    
    currentStep = target_step;
    Apply_Relay_Step(target_step);
    raw_Delay_Ms(5);
}

// INITIALIZATION - FIXED FOR 5V OPERATION
void System_Init(void) {
    // *** CRITICAL: Setup Flash latency FIRST for 5V operation ***
    Setup_Flash_For_5V();
    
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
    SystemCoreClockUpdate();
    
    GPIO_Init_Custom();
    ADC_Init_Custom();
    TIM_Init_Custom();
    NVIC_Init_Custom();
    FLASH_Unlock();
    
    // Small settling delay for peripherals
    for(volatile uint32_t i=0; i<240000; i++) __NOP();
}

void GPIO_Init_Custom(void) {
    GPIO_InitTypeDef g={0};
    
    // Enable all port clocks + AFIO for pin remapping
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOC|
                           RCC_APB2Periph_GPIOD|RCC_APB2Periph_AFIO, ENABLE);
    
    // OPTIONAL: Disable SDI to use PD1 as normal GPIO (Fault LED)
    // WARNING: After this, reprogramming requires power cycle!
    // Comment out if you want to keep programming capability
    GPIO_PinRemapConfig(GPIO_Remap_SDI_Disable, ENABLE);
    
    // Configure relay outputs on GPIOC (R1 on PC0)
    g.GPIO_Pin = PIN_R1;
    g.GPIO_Mode = GPIO_Mode_Out_PP;
    g.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOC, &g);
    
    // Configure relay outputs on GPIOD (R2, R3, R4)
    g.GPIO_Pin = PIN_R2 | PIN_R3 | PIN_R4;
    GPIO_Init(GPIOD, &g);
    
    // Configure relay output on GPIOA (R5 on PA1)
    g.GPIO_Pin = PIN_R5;
    GPIO_Init(GPIOA, &g);
    
    // Configure LED outputs on GPIOC
    g.GPIO_Pin = PIN_MAIN_LED;
    GPIO_Init(GPIOC, &g);
    
    // Configure LED outputs on GPIOD
    g.GPIO_Pin = PIN_FAULT_LED | PIN_SETTING_LED;
    GPIO_Init(GPIOD, &g);
    
    // Configure input buttons on GPIOC
    g.GPIO_Pin = PIN_LOWCUT_EN | PIN_M_START | PIN_BUTTON;
    g.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(GPIOC, &g);
    
    // Configure ADC input on GPIOA (PA2)
    g.GPIO_Pin = PIN_ADC_SENSE;
    g.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_Init(GPIOA, &g);
    
    // Reset all outputs to LOW
    GPIO_ResetBits(GPIOC, PIN_R1 | PIN_MAIN_LED);
    GPIO_ResetBits(GPIOD, PIN_R2 | PIN_R3 | PIN_R4 | PIN_FAULT_LED | PIN_SETTING_LED);
    GPIO_ResetBits(GPIOA, PIN_R5);
}

void ADC_Init_Custom(void) {
    ADC_InitTypeDef a={0};
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
    RCC_ADCCLKConfig(RCC_PCLK2_Div8);
    ADC_DeInit(ADC1);
    a.ADC_Mode = ADC_Mode_Independent;
    a.ADC_ScanConvMode = DISABLE;
    a.ADC_ContinuousConvMode = DISABLE;
    a.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
    a.ADC_DataAlign = ADC_DataAlign_Right;
    a.ADC_NbrOfChannel = 1;
    ADC_Init(ADC1, &a);
    ADC_Cmd(ADC1, ENABLE);
    
    ADC_ResetCalibration(ADC1);
    while(ADC_GetResetCalibrationStatus(ADC1));
    ADC_StartCalibration(ADC1);
    while(ADC_GetCalibrationStatus(ADC1));
    
    for(volatile uint32_t i=0; i<240000; i++) __NOP();
}

void TIM_Init_Custom(void) {
    TIM_TimeBaseInitTypeDef t={0};
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
    t.TIM_Period = 999;
    t.TIM_Prescaler = (SystemCoreClock/1000000)-1;
    t.TIM_ClockDivision = TIM_CKD_DIV1;
    t.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM2, &t);
    TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
    TIM_Cmd(TIM2, ENABLE);
}

void NVIC_Init_Custom(void) {
    NVIC_InitTypeDef n={0};
    n.NVIC_IRQChannel = TIM2_IRQn;
    n.NVIC_IRQChannelPreemptionPriority = 1;
    n.NVIC_IRQChannelSubPriority = 1;
    n.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&n);
}

void TIM2_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void TIM2_IRQHandler(void) {
    if(TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET) {
        systemTick++;
        TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
    }
}

uint32_t Calculate_Checksum(Settings_t* s) {
    return s->adc_captured_a + s->delay_time_ms + s->magic;
}

void Load_Settings(void) {
    Settings_t* s = (Settings_t*)FLASH_SETTINGS_ADDR;
    if(s->magic == SETTINGS_MAGIC && s->checksum == Calculate_Checksum(s)) {
        adcCapturedA = s->adc_captured_a;
        delayTimeMs = s->delay_time_ms;
        if(adcCapturedA == 0 || adcCapturedA > 1023) adcCapturedA = 0;
        if(delayTimeMs < MIN_DELAY_TIME_SEC*1000 || delayTimeMs > MAX_DELAY_TIME_SEC*1000)
            delayTimeMs = DEFAULT_DELAY_TIME_SEC*1000;
    } else {
        adcCapturedA = 0;
        delayTimeMs = DEFAULT_DELAY_TIME_SEC*1000;
    }
}

void Save_Settings(void) {
    Settings_t s;
    s.adc_captured_a = adcCapturedA;
    s.delay_time_ms = delayTimeMs;
    s.magic = SETTINGS_MAGIC;
    s.checksum = Calculate_Checksum(&s);
    FLASH_ErasePage(FLASH_SETTINGS_ADDR);
    uint32_t* src = (uint32_t*)&s;
    uint32_t addr = FLASH_SETTINGS_ADDR;
    for(uint32_t i = 0; i < sizeof(Settings_t)/4; i++, addr += 4)
        FLASH_ProgramWord(addr, src[i]);
}

void Clear_Settings(void) {
    adcCapturedA = 0;
    delayTimeMs = DEFAULT_DELAY_TIME_SEC*1000;
    FLASH_ErasePage(FLASH_SETTINGS_ADDR);
}

// END OF PART 1 - Continue with Part 2...

/*
 * ============================================================================
 * VOLTAGE STABILIZER - FIXED FOR 5V OPERATION - PART 2 of 3
 * ============================================================================
 * Append this after Part 1
 * ADC on PA2 (Channel 0) - Now works at 5V with Flash latency fix
 * ============================================================================
 */

// ADC FUNCTIONS WITH 5V COMPENSATION
uint16_t ADC_ReadCount(void) {
    ADC_RegularChannelConfig(ADC1, ADC_Channel_0, 1, ADC_SampleTime_241Cycles);
    ADC_SoftwareStartConvCmd(ADC1, ENABLE);
    while(!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC));
    return ADC_GetConversionValue(ADC1);
}

uint16_t ADC_ReadCount_Averaged(void) {
    uint16_t samples[ADC_SAMPLES_COUNT]; 
    uint32_t sum = 0;
    
    for(int i = 0; i < ADC_SAMPLES_COUNT; i++) {
        ADC_RegularChannelConfig(ADC1, ADC_Channel_0, 1, ADC_SampleTime_241Cycles);
        ADC_SoftwareStartConvCmd(ADC1, ENABLE);
        while(!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC));
        samples[i] = ADC_GetConversionValue(ADC1);
        raw_Delay_Us(ADC_SETTLE_DELAY_US);
    }
    
    for(int i = 0; i < ADC_SAMPLES_COUNT-1; i++)
        for(int j = 0; j < ADC_SAMPLES_COUNT-i-1; j++)
            if(samples[j] > samples[j+1]) {
                uint16_t t = samples[j]; 
                samples[j] = samples[j+1]; 
                samples[j+1] = t;
            }
    
    int valid = ADC_SAMPLES_COUNT - (2*ADC_DISCARD_SAMPLES);
    for(int i = ADC_DISCARD_SAMPLES; i < ADC_SAMPLES_COUNT-ADC_DISCARD_SAMPLES; i++)
        sum += samples[i];
    
    return (uint16_t)(sum/valid);
}

uint16_t ADC_ReadCount_Filtered(void) {
    uint16_t newSample = ADC_ReadCount_Averaged();
    
    if(!adcFilterInitialized) {
        adcFilteredValue = newSample; 
        adcFilterInitialized = true;
        return newSample;
    }
    
    adcFilteredValue = ((2*newSample) + (8*adcFilteredValue)) / 10;
    return (uint16_t)adcFilteredValue;
}

uint16_t ADC_Capture_Calibration(void) {
    uint16_t captures[ADC_CAPTURE_COUNT];
    
    for(int i = 0; i < ADC_CAPTURE_COUNT; i++) {
        captures[i] = ADC_ReadCount_Averaged();
        raw_Delay_Ms(50);
    }
    
    for(int i = 0; i < ADC_CAPTURE_COUNT-1; i++)
        for(int j = 0; j < ADC_CAPTURE_COUNT-i-1; j++)
            if(captures[j] > captures[j+1]) {
                uint16_t t = captures[j]; 
                captures[j] = captures[j+1]; 
                captures[j+1] = t;
            }
    
    return captures[ADC_CAPTURE_COUNT/2];
}

float Calculate_OPV(uint16_t adc) {
    if(adcCapturedA == 0) return 0.0f;
    return ((float)adc / (float)adcCapturedA) * CALIBRATION_VOLTAGE;
}

// STATE MACHINE 1 - VOLTAGE CALCULATION
void StateMachine1_Calculate_Voltages(void) {
    uint16_t adc = ADC_ReadCount_Filtered();
    currentOPV = Calculate_OPV(adc);
    currentIPV = currentOPV * relaySteps[currentStep].tap_ratio;
}

// STATE MACHINE 2 - RELAY CONTROL
void StateMachine2_Control_R1_R4(void) {
    uint8_t newStep = currentStep;
    float ipv = currentIPV;
    
    if(ipv > relaySteps[currentStep].threshold_up && currentStep < 7) {
        for(int i = currentStep+1; i < 8; i++) {
            if(ipv > relaySteps[i].threshold_up) newStep = i;
            else break;
        }
    } else if(currentStep > 0 && ipv < relaySteps[currentStep].threshold_down) {
        for(int i = currentStep-1; i >= 0; i--) {
            if(ipv < relaySteps[i+1].threshold_down) newStep = i;
            else break;
        }
    }
    
    if(newStep != currentStep) {
        if(!stepChangePending) {
            pendingStep = newStep; 
            stepChangePending = true; 
            relayChangeTimer = systemTick;
        } else if(pendingStep == newStep) {
            if((systemTick - relayChangeTimer) >= DEBOUNCE_TIME_MS) {
                currentStep = newStep; 
                Apply_Relay_Step(newStep); 
                stepChangePending = false;
            }
        } else {
            pendingStep = newStep; 
            relayChangeTimer = systemTick;
        }
    } else {
        stepChangePending = false;
    }
}

void StateMachine2_Control_R5(void) {
    bool lowcut = !GPIO_ReadInputDataBit(GPIOC, PIN_LOWCUT_EN);
    float opv = currentOPV;
    
    switch(r5State) {
        case R5_NORMAL:
            if(opv > HICUT_THRESHOLD) {
                r5State = R5_HICUT_DETECTING; 
                r5Timer = systemTick;
            } else if(lowcut && opv < LOCUT_THRESHOLD) {
                r5State = R5_LOCUT_DETECTING; 
                r5Timer = systemTick;
            }
            break;
            
        case R5_HICUT_DETECTING:
            if(opv > HICUT_THRESHOLD) {
                if((systemTick - r5Timer) >= HICUT_DETECT_TIME_MS) {
                    r5State = R5_HICUT_ACTIVE; 
                    Set_R5_Relay(false); 
                    currentState = STATE_FAULT;
                }
            } else {
                r5State = R5_NORMAL;
            }
            break;
            
        case R5_HICUT_ACTIVE:
            if(opv < HICUT_RESUME) {
                r5State = R5_HICUT_RESUMING; 
                r5Timer = systemTick;
            }
            break;
            
        case R5_HICUT_RESUMING:
            if(opv < HICUT_RESUME) {
                if((systemTick - r5Timer) >= HICUT_RESUME_TIME_MS) {
                    r5State = R5_DELAY_ACTIVE; 
                    r5Timer = systemTick;
                    currentState = STATE_NORMAL; 
                    LED_Set(GPIOD, PIN_FAULT_LED, false);
                }
            } else {
                r5State = R5_HICUT_ACTIVE;
            }
            break;
            
        case R5_LOCUT_DETECTING:
            if(opv < LOCUT_THRESHOLD) {
                if((systemTick - r5Timer) >= LOCUT_DETECT_TIME_MS) {
                    r5State = R5_LOCUT_ACTIVE; 
                    Set_R5_Relay(false); 
                    currentState = STATE_FAULT;
                }
            } else {
                r5State = R5_NORMAL;
            }
            break;
            
        case R5_LOCUT_ACTIVE:
            if(opv > LOCUT_RESUME) {
                r5State = R5_LOCUT_RESUMING; 
                r5Timer = systemTick;
            }
            break;
            
        case R5_LOCUT_RESUMING:
            if(opv > LOCUT_RESUME) {
                if((systemTick - r5Timer) >= LOCUT_RESUME_TIME_MS) {
                    r5State = R5_DELAY_ACTIVE; 
                    r5Timer = systemTick;
                    currentState = STATE_NORMAL; 
                    LED_Set(GPIOD, PIN_FAULT_LED, false);
                }
            } else {
                r5State = R5_LOCUT_ACTIVE;
            }
            break;
            
        case R5_DELAY_ACTIVE:
            if((systemTick - r5Timer) >= delayTimeMs) {
                Set_R5_Relay(true); 
                r5State = R5_NORMAL; 
                LED_Set(GPIOC, PIN_MAIN_LED, true);
            }
            break;
    }
}

void Apply_Relay_Step(uint8_t step) {
    if(step >= 8) return;
    
    GPIO_WriteBit(GPIOC, PIN_R1, relaySteps[step].r1 ? Bit_SET : Bit_RESET);
    GPIO_WriteBit(GPIOD, PIN_R2, relaySteps[step].r2 ? Bit_SET : Bit_RESET);
    GPIO_WriteBit(GPIOD, PIN_R3, relaySteps[step].r3 ? Bit_SET : Bit_RESET);
    GPIO_WriteBit(GPIOD, PIN_R4, relaySteps[step].r4 ? Bit_SET : Bit_RESET);
}

void Set_R5_Relay(bool state) {
    r5Status = state;
    GPIO_WriteBit(GPIOA, PIN_R5, state ? Bit_SET : Bit_RESET);
}

// SETTING MODE
void Enter_Setting_Mode(void) {
    Clear_Settings();
    
    for(int i = 0; i < 3; i++) {
        LED_Set(GPIOD, PIN_SETTING_LED, true); 
        raw_Delay_Ms(300);
        LED_Set(GPIOD, PIN_SETTING_LED, false); 
        raw_Delay_Ms(300);
    }
    
    currentState = STATE_SETTING;
    settingState = SETTING_WAITING_DELAY
...
