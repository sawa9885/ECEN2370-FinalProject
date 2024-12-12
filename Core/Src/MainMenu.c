/*
 * MainMenu.c
 *
 *  Created on: Dec 3, 2024
 *      Author: sammy
 */

#include <stdbool.h>
#include "MainMenu.h"
#include "Game.h"

// Constants for the play button
static const uint16_t playButtonX = 120;
static const uint16_t playButtonY = 160;
static const uint16_t playButtonSize = 20;

// Constants for block placement
static const uint16_t buffer = 5; // Buffer size in pixels

bool IsOverlap(uint16_t Xpos, uint16_t Ypos, const Block *block, uint16_t positions[][2], uint8_t numPlacedBlocks) {
    uint16_t blockWidth = block->width * RENDER_BLOCK_SIZE + buffer;
    uint16_t blockHeight = block->height * RENDER_BLOCK_SIZE + buffer;

    // Check overlap with previously placed blocks
    for (uint8_t i = 0; i < numPlacedBlocks; i++) {
        uint16_t otherX = positions[i][0];
        uint16_t otherY = positions[i][1];
        uint16_t otherWidth = tetrisBlocks[i].width * RENDER_BLOCK_SIZE + buffer;
        uint16_t otherHeight = tetrisBlocks[i].height * RENDER_BLOCK_SIZE + buffer;

        if (!(Xpos + blockWidth <= otherX || // No overlap to the left
              Xpos >= otherX + otherWidth || // No overlap to the right
              Ypos + blockHeight <= otherY || // No overlap above
              Ypos >= otherY + otherHeight)) { // No overlap below
            return true; // Overlap detected
        }
    }

    // Check overlap with the play button
    uint16_t playButtonLeft = playButtonX - playButtonSize - buffer;
    uint16_t playButtonRight = playButtonX + playButtonSize + buffer;
    uint16_t playButtonTop = playButtonY - playButtonSize - buffer;
    uint16_t playButtonBottom = playButtonY + playButtonSize + buffer;

    if (!(Xpos + blockWidth <= playButtonLeft || // No overlap to the left of the button
          Xpos >= playButtonRight ||            // No overlap to the right of the button
          Ypos + blockHeight <= playButtonTop || // No overlap above the button
          Ypos >= playButtonBottom)) {           // No overlap below the button
        return true; // Overlap with play button detected
    }

    return false; // No overlap
}

void PlaceBlock(uint16_t *Xpos, uint16_t *Ypos, const Block *block, uint16_t positions[][2], uint8_t numPlacedBlocks) {
    do {
        *Xpos = rand() % (LCD_PIXEL_WIDTH - block->width * RENDER_BLOCK_SIZE);
        *Ypos = rand() % (LCD_PIXEL_HEIGHT - block->height * RENDER_BLOCK_SIZE);
    } while (IsOverlap(*Xpos, *Ypos, block, positions, numPlacedBlocks));
}

void DrawPlayButton(void) {
    for (int16_t y = -playButtonSize; y <= playButtonSize; y++) {
        for (int16_t x = -playButtonSize; x <= playButtonSize; x++) {
            if (x < 0 && abs(x) >= abs(y)) {
                LCD_Draw_Pixel(playButtonX + x, playButtonY + y, LCD_COLOR_WHITE);
            }
        }
    }
}

void DisplayMenu(void) {
    // Clear the screen with black background
    LCD_Clear(0, LCD_COLOR_BLACK);

    // Array to store positions of placed blocks
    uint16_t positions[NUM_BLOCKS][2] = {0};

    // Display all Tetris blocks scattered on the screen
    for (uint8_t i = 0; i < NUM_BLOCKS; i++) {
        uint16_t randX, randY;
        PlaceBlock(&randX, &randY, &tetrisBlocks[i], positions, i);
        positions[i][0] = randX;
        positions[i][1] = randY;
        LCD_Draw_Block(randX, randY, &tetrisBlocks[i]);
    }

    // Draw the "Play" button
    DrawPlayButton();
}
