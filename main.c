/*===================================================================================
 * Simple Display Test - FIXED FOR 5V OPERATION v3
 * For CH32V003A4M6
 * Works from 2.5V to 5.5V
 *
 * CRITICAL HARDWARE NOTE:
 * 1. PD7 must be set to GPIO in Option Bytes (WCH-LinkUtility)
 * 2. Flash latency is configured in system_ch32v00x.c for 5V stability
 *
 * FIXES in v3:
 * - Logic bug: digit_idx > 1 changed to > 2 (digit 3 was skipped)
 * - Consistent digit control (all use SetBits=ON, ResetBits=OFF)
 *===================================================================================*/
#include "debug.h"
#include "ch32v00x_conf.h"

// Segment pins
#define SEG_A_PIN       GPIO_Pin_1      // PD1
#define SEG_B_PIN       GPIO_Pin_6      // PD6
#define SEG_C_PIN       GPIO_Pin_2      // PC2
#define SEG_D_PIN       GPIO_Pin_1      // PC1
#define SEG_E_PIN       GPIO_Pin_3      // PC3
#define SEG_F_PIN       GPIO_Pin_6      // PC6
#define SEG_G_PIN       GPIO_Pin_0      // PC0

#define DIGIT1_PIN      GPIO_Pin_7      // PC7
#define DIGIT2_PIN      GPIO_Pin_5      // PD5
#define DIGIT3_PIN      GPIO_Pin_7      // PD7

// 7-segment patterns (common anode)
static const uint8_t DIGIT_PAT[10] = {
    0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F
};

static uint8_t segPattern[3] = {0, 0, 0};

/*===================================================================================
 * CRITICAL: System Init for Full Voltage Range (2.5V - 5.5V)
 * MUST be called FIRST before any other code!
 *===================================================================================*/
void Setup_Flash_For_5V(void)
{
    // CRITICAL: At VDD > 3.6V with 24MHz HSI, Flash needs 1 wait state
    FLASH->ACTLR &= ~(FLASH_ACTLR_LATENCY);     // Clear latency bits
    FLASH->ACTLR |= FLASH_ACTLR_LATENCY_1;     // Set 1 wait state
    
    // Increased stabilization delay
    for(volatile uint32_t i=0; i<50000; i++);
}

void GPIO_Init_All(void)
{
    GPIO_InitTypeDef GPIO_InitStructure = {0};

    RCC_APB2PeriphClockCmd(
        RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD,
        ENABLE
    );

    // SDI is already disabled in main(), so don't call it again here

    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

    // Port C: segments and digit1
    GPIO_InitStructure.GPIO_Pin = SEG_C_PIN | SEG_D_PIN | SEG_E_PIN | 
                                   SEG_F_PIN | SEG_G_PIN | DIGIT1_PIN;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    // Port D: segments and digit2 (NOT digit3 yet)
    GPIO_InitStructure.GPIO_Pin = SEG_A_PIN | SEG_B_PIN | DIGIT2_PIN;
    GPIO_Init(GPIOD, &GPIO_InitStructure);

    // SPECIAL: Initialize PD7/DIGIT3 (NRST pin) separately
    GPIO_InitStructure.GPIO_Pin = DIGIT3_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOD, &GPIO_InitStructure);

    // All segments OFF (HIGH for common anode)
    GPIO_SetBits(GPIOD, SEG_A_PIN | SEG_B_PIN);
    GPIO_SetBits(GPIOC, SEG_C_PIN | SEG_D_PIN | SEG_E_PIN | SEG_F_PIN | SEG_G_PIN);

    // All digits OFF (LOW = OFF for common anode with active-high drivers)
    GPIO_ResetBits(GPIOC, DIGIT1_PIN);
    GPIO_ResetBits(GPIOD, DIGIT2_PIN);
    GPIO_ResetBits(GPIOD, DIGIT3_PIN);
}

void SetSegment(uint8_t pattern)
{
    // Common anode: LOW = segment ON
    GPIO_WriteBit(GPIOD, SEG_A_PIN, (pattern & 0x01) ? Bit_RESET : Bit_SET);
    GPIO_WriteBit(GPIOD, SEG_B_PIN, (pattern & 0x02) ? Bit_RESET : Bit_SET);
    GPIO_WriteBit(GPIOC, SEG_C_PIN, (pattern & 0x04) ? Bit_RESET : Bit_SET);
    GPIO_WriteBit(GPIOC, SEG_D_PIN, (pattern & 0x08) ? Bit_RESET : Bit_SET);
    GPIO_WriteBit(GPIOC, SEG_E_PIN, (pattern & 0x10) ? Bit_RESET : Bit_SET);
    GPIO_WriteBit(GPIOC, SEG_F_PIN, (pattern & 0x20) ? Bit_RESET : Bit_SET);
    GPIO_WriteBit(GPIOC, SEG_G_PIN, (pattern & 0x40) ? Bit_RESET : Bit_SET);
}

void Display_Refresh(void)
{
    static uint8_t digit_idx = 0;

    // 1. All digits OFF (Blanking)
    GPIO_ResetBits(GPIOC, DIGIT1_PIN);
    GPIO_ResetBits(GPIOD, DIGIT2_PIN);
    GPIO_ResetBits(GPIOD, DIGIT3_PIN);

    // Small blanking delay to prevent ghosting
    Delay_Us(30);

    // 2. Set segments for current digit
    SetSegment(segPattern[digit_idx]);

    // 3. Turn on current digit
    if(digit_idx == 0) {
        GPIO_SetBits(GPIOC, DIGIT1_PIN);
    }
    else if(digit_idx == 1) {
        GPIO_SetBits(GPIOD, DIGIT2_PIN);
    }
    else if(digit_idx == 2) {
        // PD7 - requires Option Byte NRST disabled
        GPIO_SetBits(GPIOD, DIGIT3_PIN);
    }

    // Display time
    Delay_Us(1500);

    // 4. Cycle to next digit
    digit_idx++;
    // FIX: Was "digit_idx > 1" which skipped digit 3
    if(digit_idx > 2) digit_idx = 0;
}

void ShowFor(uint16_t ms)
{
    uint16_t cycles = ms / 5;
    for(uint16_t i = 0; i < cycles; i++) {
        Display_Refresh();
        Display_Refresh();
        Display_Refresh();
    }
}

int main(void)
{
    // STEP 1: Flash latency FIRST
    Setup_Flash_For_5V();
    
    // STEP 2: Enable AFIO clock BEFORE any GPIO remapping
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
    
    // STEP 3: Disable SDI EARLY (before Delay_Init)
    GPIO_PinRemapConfig(GPIO_Remap_SDI_Disable, ENABLE);
    
    // Small delay after SDI disable
    for(volatile uint32_t i=0; i<50000; i++);
    
    // STEP 4: Now initialize delay functions
    Delay_Init();
    
    // STEP 5: Button check
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    
    GPIO_InitTypeDef GPIO_InitStructure = {0};
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    Delay_Ms(50);
    
    // Check buttons for programming mode
    if(GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_1) == 0 &&
       GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_2) == 0)
    {
        Delay_Ms(100);
    }
    else
    {
        Delay_Ms(3000);
    }
    
    // STEP 6: Initialize display GPIOs
    GPIO_Init_All();

    // Main display loop
    while(1)
    {
        // Show "1" on digit 1 only
        segPattern[0] = DIGIT_PAT[1];
        segPattern[1] = 0x00;
        segPattern[2] = 0x00;
        ShowFor(1000);

        // Show "2" on digit 2 only
        segPattern[0] = 0x00;
        segPattern[1] = DIGIT_PAT[2];
        segPattern[2] = 0x00;
        ShowFor(1000);

        // Show "3" on digit 3 only
        segPattern[0] = 0x00;
        segPattern[1] = 0x00;
        segPattern[2] = DIGIT_PAT[3];
        ShowFor(1000);

        // Show "888"
        segPattern[0] = DIGIT_PAT[8];
        segPattern[1] = DIGIT_PAT[8];
        segPattern[2] = DIGIT_PAT[8];
        ShowFor(1000);
    }

    return 0;
}
