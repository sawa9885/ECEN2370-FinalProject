/*
 * GameOver.c
 *
 *  Created on: Dec 3, 2024
 *      Author: sammy
 */

#include <stdio.h>
#include "GameOver.h"
#include "Game.h"

void CheckGameOver(void) {
    // Iterate through the top row of the matrix to check for any filled blocks
    for (uint8_t x = 0; x < GRID_WIDTH; x++) {
        if (blockMatrix[0][x] != 0x0000) {
            currentState = GAME_OVER;
            HAL_TIM_Base_Stop_IT(&htim2);
            RenderGameOverScreen();
            return;
        }
    }
}

void RenderGameOverScreen(void) {
    char buffer[32];

    LCD_Clear(0, LCD_COLOR_BLACK);
    LCD_SetTextColor(LCD_COLOR_WHITE);
    LCD_SetFont(&Font16x24);

    uint16_t textX = (LCD_PIXEL_WIDTH / 2) - (4 * 16);
    uint16_t textY = LCD_PIXEL_HEIGHT / 6;
    LCD_DisplayString(textX, textY, "GAME OVER");

    // Display elapsed time
    textX = STAT_X;
    textY += BUFFER_Y;
    sprintf(buffer, "Time: %02u:%02u", elapsedTime / 60, elapsedTime % 60);
    LCD_DisplayString(textX, textY, buffer);

    // Display stats: singles, doubles, triples, tetrises
    textY += BUFFER_Y;
    sprintf(buffer, "Singles: %u", singles);
    LCD_DisplayString(textX, textY, buffer);

    textY += BUFFER_Y;
    sprintf(buffer, "Doubles: %u", doubles);
    LCD_DisplayString(textX, textY, buffer);

    textY += BUFFER_Y;
    sprintf(buffer, "Triples: %u", triples);
    LCD_DisplayString(textX, textY, buffer);

    textY += BUFFER_Y;
    sprintf(buffer, "Tetrises: %u", tetri);
    LCD_DisplayString(textX, textY, buffer);
}


