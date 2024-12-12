/*
 * GameOver.h
 *
 *  Created on: Dec 3, 2024
 *      Author: sammy
 */

#ifndef INC_GAMEOVER_H_
#define INC_GAMEOVER_H_

#include <stdint.h>
#include "LCD_Driver.h"

#define CHAR_WIDTH 16
#define STAT_X 20
#define BUFFER_Y 40

void CheckGameOver(void);
void RenderGameOverScreen(void);

#endif /* INC_GAMEOVER_H_ */
