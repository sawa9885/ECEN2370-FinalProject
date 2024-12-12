/*
 * Game.h
 *
 *  Created on: Dec 3, 2024
 *      Author: sammy
 */

#ifndef INC_GAME_H_
#define INC_GAME_H_

#include <stdint.h>
#include "LCD_Driver.h"

#define GRID_WIDTH 10
#define GRID_HEIGHT 20

#define X_OFF 5
#define Y_OFF 5
#define CHAR_WIDTH 16
#define TIMER_NUMCHARS 6

#define NUM_BLOCKS 7
#define BLOCK_SIZE 4
#define BAG_SIZE NUM_BLOCKS

extern uint8_t elapsedTime;
extern uint8_t singles;
extern uint8_t doubles;
extern uint8_t triples;
extern uint8_t tetri;

extern uint8_t currentFallingBlock;

extern uint16_t blockMatrix[GRID_HEIGHT][GRID_WIDTH];
extern uint8_t fallingBlockMatrix[GRID_HEIGHT][GRID_WIDTH];

extern const Block tetrisBlocks[];

void InitializeGame(void);
void RenderGameScreen(void);
void RenderNextBlock(void);
void UpdateFallingBlock(void);
void MoveFallingBlockLeft(void);
void MoveFallingBlockRight(void);
void RotateFallingBlock(void);
void DisplayTimer(void);
void ClearFullRows(void);
void ShuffleBag(void);
void SpawnFallingBlock(uint8_t blockIndex);
void SpawnRandomBlock(void);

#endif /* INC_GAME_H_ */
