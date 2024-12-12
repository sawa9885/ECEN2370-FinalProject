/*
 * ApplicationCode.c
 *
 *  Created on: Dec 30, 2023 (updated 11/12/2024) Thanks Donavon! 
 *      Author: Xavion
 */

#include "ApplicationCode.h"
#include "MainMenu.h"
#include "Game.h"
#include "GameOver.h"

/* Static variables */


extern void initialise_monitor_handles(void); 

#if COMPILE_TOUCH_FUNCTIONS == 1
static STMPE811_TouchData StaticTouchData;
#if TOUCH_INTERRUPT_ENABLED == 1
static EXTI_HandleTypeDef LCDTouchIRQ;
void LCDTouchScreenInterruptGPIOInit(void);
#endif // TOUCH_INTERRUPT_ENABLED
#endif // COMPILE_TOUCH_FUNCTIONS

volatile AppState currentState = MAIN_MENU;

void ApplicationInit(void)
{
	initialise_monitor_handles(); // Allows printf functionality
    LTCD__Init();
    LTCD_Layer_Init(0);
    LCD_Clear(0,LCD_COLOR_WHITE);

    // Initialize the random number generator with a unique seed
    uint32_t seed;
    HAL_RNG_GenerateRandomNumber(&hrng, &seed);
    srand(seed);

    #if COMPILE_TOUCH_FUNCTIONS == 1
	InitializeLCDTouch();

	// This is the orientation for the board to be direclty up where the buttons are vertically above the screen
	// Top left would be low x value, high y value. Bottom right would be low x value, low y value.
	StaticTouchData.orientation = STMPE811_Orientation_Portrait_2;

	#if TOUCH_INTERRUPT_ENABLED == 1
	LCDTouchScreenInterruptGPIOInit();
	#endif // TOUCH_INTERRUPT_ENABLED

	#endif // COMPILE_TOUCH_FUNCTIONS
}

void MainMenu(void)
{
	DisplayMenu();
}

void Game(void)
{
	if(currentState == GAME_SCREEN)
	{
		RenderGameScreen();
		UpdateFallingBlock();
        CheckGameOver();
	}
}

// TouchScreen Interrupt
#if TOUCH_INTERRUPT_ENABLED == 1

void LCDTouchScreenInterruptGPIOInit(void)
{
	GPIO_InitTypeDef LCDConfig = {0};
    LCDConfig.Pin = GPIO_PIN_15;
    LCDConfig.Mode = GPIO_MODE_IT_RISING_FALLING;
    LCDConfig.Pull = GPIO_NOPULL;
    LCDConfig.Speed = GPIO_SPEED_FREQ_HIGH;
    
    // Clock enable
    __HAL_RCC_GPIOA_CLK_ENABLE();

    // GPIO Init      
    HAL_GPIO_Init(GPIOA, &LCDConfig);

    // Interrupt Configuration
    HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

	LCDTouchIRQ.Line = EXTI_LINE_15;

}

#define TOUCH_DETECTED_IRQ_STATUS_BIT   (1 << 0)  // Touchscreen detected bitmask

void EXTI15_10_IRQHandler()
{
    HAL_NVIC_DisableIRQ(EXTI15_10_IRQn); // Disable the IRQ to avoid re-entrancy
    bool isTouchDetected = false;

    static uint32_t count;
    count = 0;
    while (count == 0) {
        count = STMPE811_Read(STMPE811_FIFO_SIZE);
    }

    // Disable touch interrupt bit on the STMPE811
    uint8_t currentIRQEnables = ReadRegisterFromTouchModule(STMPE811_INT_EN);
    WriteDataToTouchModule(STMPE811_INT_EN, 0x00);

    // Clear the interrupt bit in the STMPE811
    uint8_t statusFlag = ReadRegisterFromTouchModule(STMPE811_INT_STA);
    uint8_t clearIRQData = (statusFlag | TOUCH_DETECTED_IRQ_STATUS_BIT); // Write one to clear bit
    WriteDataToTouchModule(STMPE811_INT_STA, clearIRQData);

    uint8_t ctrlReg = ReadRegisterFromTouchModule(STMPE811_TSC_CTRL);
    if (ctrlReg & 0x80) {
        isTouchDetected = true;
    }

    // Determine if it is pressed or unpressed
    if (isTouchDetected) { // Touch has been detected
        DetermineTouchPosition(&StaticTouchData);

        if (currentState == MAIN_MENU) {
            // Handle touch in the main menu
            currentState = GAME_SCREEN;  // Transition to the game screen
            InitializeGame();            // Initialize game state
            RenderGameScreen();          // Show the game grid
        } else if (currentState == GAME_SCREEN) {
            // Handle touch during the game
            uint16_t screenMidpoint = LCD_PIXEL_WIDTH / 2;

            //Flipped because i flipped the pixels 180deg
            if (StaticTouchData.x < screenMidpoint) {
                // Left half of the screen
                MoveFallingBlockRight();
            } else {
                // Right half of the screen
                MoveFallingBlockLeft();
            }
        }
    }

    // Reset FIFO
    STMPE811_Write(STMPE811_FIFO_STA, 0x01);
    STMPE811_Write(STMPE811_FIFO_STA, 0x00);

    // Re-enable IRQs
    WriteDataToTouchModule(STMPE811_INT_EN, currentIRQEnables);
    HAL_EXTI_ClearPending(&LCDTouchIRQ, EXTI_TRIGGER_RISING_FALLING);

    HAL_NVIC_ClearPendingIRQ(EXTI15_10_IRQn);
    HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

    // Clear IRQ bit again in case of errata
    WriteDataToTouchModule(STMPE811_INT_STA, clearIRQData);
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
    if (GPIO_Pin == GPIO_PIN_0) {
        RotateFallingBlock();
    }
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
    if (htim->Instance == TIM2) { // Check if TIM2 triggered the callback
        __HAL_TIM_DISABLE_IT(htim, TIM_IT_UPDATE); // Disable TIM2 interrupt
        elapsedTime++; // Increment elapsed time counter
        __HAL_TIM_ENABLE_IT(htim, TIM_IT_UPDATE); // Reenable TIM2 interrupt
    }
    if (htim->Instance == TIM5) { // Check if TIM2 triggered the callback
        __HAL_TIM_DISABLE_IT(htim, TIM_IT_UPDATE); // Disable TIM5 interrupt
        Game();
        __HAL_TIM_ENABLE_IT(htim, TIM_IT_UPDATE); // Reenable TIM5 interrupt
    }
}


#endif // TOUCH_INTERRUPT_ENABLED
