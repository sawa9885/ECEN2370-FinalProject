/*
 * ApplicationCode.h
 *
 *  Created on: Dec 30, 2023
 *      Author: Xavion
 */

#include <stdio.h>
#include "stm32f4xx_hal.h"

#ifndef INC_APPLICATIONCODE_H_
#define INC_APPLICATIONCODE_H_

#define RENDER_BLOCK_SIZE 20

extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim5;
extern RNG_HandleTypeDef hrng;

typedef enum {
    MAIN_MENU,
    GAME_SCREEN,
	GAME_OVER
} AppState;

extern volatile AppState currentState;

typedef struct {
    uint8_t shape[4][4];
    uint8_t width;
    uint8_t height;
    uint32_t color;
} Block;

void ApplicationInit(void);
void MainMenu(void);
void Game(void);

#endif /* INC_APPLICATIONCODE_H_ */
