/*
 * Game.c
 *
 *  Created on: Dec 3, 2024
 *      Author: sammy
 */
#include "Game.h"

uint16_t blockMatrix[GRID_HEIGHT][GRID_WIDTH] = {0};

uint8_t fallingBlockMatrix[GRID_HEIGHT][GRID_WIDTH] = {0};
Block fallingBlock;

uint8_t elapsedTime = 0;
uint8_t singles = 0;
uint8_t doubles = 0;
uint8_t triples = 0;
uint8_t tetri = 0;

uint8_t currentFallingBlock = 0;
uint8_t nextBlock = 0;

static uint8_t blockBag[BAG_SIZE];
static uint8_t bagIndex = 0;
static uint8_t lastBlock = 255;

const Block tetrisBlocks[NUM_BLOCKS] = {
    { // Square block
        .shape = {
            {1, 1, 0, 0},
            {1, 1, 0, 0},
            {0, 0, 0, 0},
            {0, 0, 0, 0}
        },
        .width = 2,
        .height = 2,
        .color = LCD_COLOR_YELLOW
    },
    { // Line block
        .shape = {
            {1, 1, 1, 1},
            {0, 0, 0, 0},
            {0, 0, 0, 0},
            {0, 0, 0, 0}
        },
        .width = 4,
        .height = 1,
        .color = LCD_COLOR_CYAN
    },
    { // T block
        .shape = {
            {0, 1, 0, 0},
            {1, 1, 1, 0},
            {0, 0, 0, 0},
            {0, 0, 0, 0}
        },
        .width = 3,
        .height = 2,
        .color = LCD_COLOR_MAGENTA
    },
    { // L block
        .shape = {
            {1, 0, 0, 0},
            {1, 1, 1, 0},
            {0, 0, 0, 0},
            {0, 0, 0, 0}
        },
        .width = 3,
        .height = 2,
        .color = LCD_COLOR_ORANGE
    },
    { // Reverse L block
        .shape = {
            {0, 0, 1, 0},
            {1, 1, 1, 0},
            {0, 0, 0, 0},
            {0, 0, 0, 0}
        },
        .width = 3,
        .height = 2,
        .color = LCD_COLOR_BLUE
    },
    { // S block
        .shape = {
            {0, 1, 1, 0},
            {1, 1, 0, 0},
            {0, 0, 0, 0},
            {0, 0, 0, 0}
        },
        .width = 3,
        .height = 2,
        .color = LCD_COLOR_GREEN
    },
    { // Z block
        .shape = {
            {1, 1, 0, 0},
            {0, 1, 1, 0},
            {0, 0, 0, 0},
            {0, 0, 0, 0}
        },
        .width = 3,
        .height = 2,
        .color = LCD_COLOR_RED
    }
};

void InitializeGame(void) {
    ShuffleBag();
    SpawnRandomBlock();

    // Setup Timers
    elapsedTime = 0;
    __HAL_TIM_SET_COUNTER(&htim2, 0);
    HAL_TIM_Base_Start_IT(&htim2);
    __HAL_TIM_SET_COUNTER(&htim5, 0);
    HAL_TIM_Base_Start_IT(&htim5);
}

void RenderGameScreen(void) {
    LCD_Clear(0, LCD_COLOR_BLACK);

    uint16_t gridBlockSize = LCD_PIXEL_HEIGHT / GRID_HEIGHT;

    // Render the grid
    for (uint8_t y = 0; y < GRID_HEIGHT; y++) {
        for (uint8_t x = 0; x < GRID_WIDTH; x++) {
            // Calculate top-left corner of the block
            uint16_t startX = x * gridBlockSize;
            uint16_t startY = y * gridBlockSize;

            // Draw grid lines (white border for all blocks)
            for (uint16_t i = 0; i < gridBlockSize; i++) {
                LCD_Draw_Pixel(startX + i, startY, LCD_COLOR_WHITE);                 // Top line
                LCD_Draw_Pixel(startX + i, startY + gridBlockSize - 1, LCD_COLOR_WHITE); // Bottom line
                LCD_Draw_Pixel(startX, startY + i, LCD_COLOR_WHITE);                 // Left line
                LCD_Draw_Pixel(startX + gridBlockSize - 1, startY + i, LCD_COLOR_WHITE); // Right line
            }

            // Render the static matrix (blockMatrix) with stored colors
            if (blockMatrix[y][x] != 0x0000) {
                for (uint16_t i = 1; i < gridBlockSize - 1; i++) {
                    for (uint16_t j = 1; j < gridBlockSize - 1; j++) {
                        LCD_Draw_Pixel(startX + i, startY + j, blockMatrix[y][x]);
                    }
                }
            }

            // Render the falling block (fallingBlockMatrix) with its color
            if (fallingBlockMatrix[y][x] == 1) {
                uint32_t color = fallingBlock.color;

                for (uint16_t i = 1; i < gridBlockSize - 1; i++) {
                    for (uint16_t j = 1; j < gridBlockSize - 1; j++) {
                        LCD_Draw_Pixel(startX + i, startY + j, color);
                    }
                }
            }
        }
    }
    DisplayTimer();
    RenderNextBlock();
}

void RenderNextBlock(void) {
    uint16_t nextBlockX = LCD_PIXEL_WIDTH - 80;
    uint16_t nextBlockY = 50;

    LCD_SetTextColor(LCD_COLOR_WHITE);
    LCD_SetFont(&Font16x24);
    LCD_DisplayString(nextBlockX, nextBlockY - 20, "Next:");
    LCD_Draw_Block(nextBlockX, nextBlockY, &tetrisBlocks[nextBlock]);
}

void UpdateFallingBlock(void) {
    for (int8_t y = GRID_HEIGHT - 1; y >= 0; y--) {
        for (uint8_t x = 0; x < GRID_WIDTH; x++) {
            if (fallingBlockMatrix[y][x] == 1) {
                // Check collision with bottom or static blocks
                if (y == GRID_HEIGHT - 1 || blockMatrix[y + 1][x] != 0x000000) {
                    // Determine the color of the falling block
                    uint32_t blockColor = fallingBlock.color;

                    // Merge falling block into the static block matrix
                    for (uint8_t mergeY = 0; mergeY < GRID_HEIGHT; mergeY++) {
                        for (uint8_t mergeX = 0; mergeX < GRID_WIDTH; mergeX++) {
                            if (fallingBlockMatrix[mergeY][mergeX] == 1) {
                                blockMatrix[mergeY][mergeX] = blockColor; // Store the block's color
                                fallingBlockMatrix[mergeY][mergeX] = 0;  // Clear the falling block
                            }
                        }
                    }
                    ClearFullRows();
                    if (currentState != GAME_OVER)
                        SpawnRandomBlock();
                    return;
                }
            }
        }
    }

    // Move the falling block down
    for (int8_t y = GRID_HEIGHT - 1; y >= 0; y--) {
        for (uint8_t x = 0; x < GRID_WIDTH; x++) {
            if (fallingBlockMatrix[y][x] == 1) {
                fallingBlockMatrix[y + 1][x] = 1;
                fallingBlockMatrix[y][x] = 0;
            }
        }
    }
}



void MoveFallingBlockLeft(void) {
    // Check if the block can move left
    for (uint8_t y = 0; y < GRID_HEIGHT; y++) {
        for (uint8_t x = 0; x < GRID_WIDTH; x++) {
            if (fallingBlockMatrix[y][x] == 1) {
                if (x == 0 || blockMatrix[y][x - 1] == 1) {
                    return; // Illegal move, do nothing
                }
            }
        }
    }

    // Move the block left
    for (uint8_t y = 0; y < GRID_HEIGHT; y++) {
        for (uint8_t x = 0; x < GRID_WIDTH; x++) {
            if (fallingBlockMatrix[y][x] == 1) {
                fallingBlockMatrix[y][x - 1] = 1;
                fallingBlockMatrix[y][x] = 0;
            }
        }
    }
}

void MoveFallingBlockRight(void) {
    // Check if the block can move right
    for (uint8_t y = 0; y < GRID_HEIGHT; y++) {
        for (int8_t x = GRID_WIDTH - 1; x >= 0; x--) {
            if (fallingBlockMatrix[y][x] == 1) {
                if (x == GRID_WIDTH - 1 || blockMatrix[y][x + 1] == 1) {
                    return; // Illegal move, do nothing
                }
            }
        }
    }

    // Move the block right
    for (uint8_t y = 0; y < GRID_HEIGHT; y++) {
        for (int8_t x = GRID_WIDTH - 1; x >= 0; x--) {
            if (fallingBlockMatrix[y][x] == 1) {
                fallingBlockMatrix[y][x + 1] = 1;
                fallingBlockMatrix[y][x] = 0;
            }
        }
    }
}

void RotateFallingBlock(void) {
    uint8_t tempMatrix[4][4] = {0};
    uint8_t blockWidth = fallingBlock.width;
    uint8_t blockHeight = fallingBlock.height;
    uint8_t topLeftX = GRID_WIDTH;
    uint8_t topLeftY = GRID_HEIGHT;

    // Find the minimum x and y coordinates of the falling block
    for (uint8_t y = 0; y < GRID_HEIGHT; y++) {
        for (uint8_t x = 0; x < GRID_WIDTH; x++) {
            if (fallingBlockMatrix[y][x] == 1) {
                if (x < topLeftX) topLeftX = x;
                if (y < topLeftY) topLeftY = y;
            }
        }
    }

    // Extract the current block into tempMatrix
    for (uint8_t y = 0; y < BLOCK_SIZE; y++) {
        for (uint8_t x = 0; x < BLOCK_SIZE; x++) {
            tempMatrix[y][x] = fallingBlockMatrix[topLeftY + y][topLeftX + x];
        }
    }

    // Transpose the matrix
    for (uint8_t i = 0; i < BLOCK_SIZE; i++) {
        for (uint8_t j = i + 1; j < BLOCK_SIZE; j++) {
            uint8_t temp = tempMatrix[i][j];
            tempMatrix[i][j] = tempMatrix[j][i];
            tempMatrix[j][i] = temp;
        }
    }

    // Reverse each row
    for (uint8_t i = 0; i < BLOCK_SIZE; i++) {
        uint8_t start = 0, end = BLOCK_SIZE-1;
        while (start < end) {
            uint8_t temp = tempMatrix[i][start];
            tempMatrix[i][start] = tempMatrix[i][end];
            tempMatrix[i][end] = temp;
            start++;
            end--;
        }
    }

    // Shift the tempMatrix left by the calculated amount for every row
    uint8_t shiftAmount = (fallingBlock.width < fallingBlock.height) ? 1 : 2;
    for (uint8_t i = 0; i < BLOCK_SIZE; i++) {
        for (uint8_t j = shiftAmount; j < BLOCK_SIZE; j++)
            tempMatrix[i][j - shiftAmount] = tempMatrix[i][j];
        for (uint8_t j = BLOCK_SIZE - shiftAmount; j < BLOCK_SIZE; j++)
            tempMatrix[i][j] = 0;
    }

    // Check for boundary collisions or overlaps
    for (uint8_t y = 0; y < BLOCK_SIZE; y++) {
        for (uint8_t x = 0; x < BLOCK_SIZE; x++) {
            if (tempMatrix[y][x] == 1) {
                uint8_t gridX = topLeftX + x;
                uint8_t gridY = topLeftY + y;

                if (gridX >= GRID_WIDTH || gridY >= GRID_HEIGHT || blockMatrix[gridY][gridX] != 0x0000) {
                    return;
                }
            }
        }
    }

    // Clear the current falling block from the grid
    for (uint8_t y = 0; y < GRID_HEIGHT; y++) {
        for (uint8_t x = 0; x < GRID_WIDTH; x++) {
            if (fallingBlockMatrix[y][x] == 1) {
                fallingBlockMatrix[y][x] = 0;
            }
        }
    }

    // Apply rotated block back to the falling block matrix
    for (uint8_t y = 0; y < BLOCK_SIZE; y++) {
        for (uint8_t x = 0; x < BLOCK_SIZE; x++) {
            if (tempMatrix[y][x] == 1) {
                fallingBlockMatrix[topLeftY + y][topLeftX + x] = 1;
            }
        }
    }

    // Update the falling block's dimensions for multiple rotation purposes
    fallingBlock.width = blockHeight;
    fallingBlock.height = blockWidth;
}

void ShuffleBag(void) {
    // Fill the bag with block indices
    for (uint8_t i = 0; i < BAG_SIZE; i++) {
        blockBag[i] = i;
    }

    // Shuffle the bag using Fisher-Yates algorithm
    for (uint8_t i = BAG_SIZE - 1; i > 0; i--) {
        uint32_t rngValue = 0;
        HAL_RNG_GenerateRandomNumber(&hrng, &rngValue);
        uint8_t j = rngValue % (i + 1);
        uint8_t temp = blockBag[i];
        blockBag[i] = blockBag[j];
        blockBag[j] = temp;
    }
}

void FormatTimerString(char *timerString, uint32_t minutes, uint32_t seconds) {
    timerString[0] = '0' + (minutes / 10);  // First digit of minutes
    timerString[1] = '0' + (minutes % 10);  // Second digit of minutes
    timerString[2] = ':';                   // Colon separator
    timerString[3] = '0' + (seconds / 10);  // First digit of seconds
    timerString[4] = '0' + (seconds % 10);  // Second digit of seconds
    timerString[5] = '\0';                  // Null terminator
}

// Function to display the timer on the screen
void DisplayTimer(void) {
    char timerString[TIMER_NUMCHARS];

    // Format elapsedTime into a string (e.g., "00:00")
    uint32_t minutes = elapsedTime / 60;
    uint32_t seconds = elapsedTime % 60;
    FormatTimerString(timerString, minutes, seconds);

    // Calculate position to render the timer (top-right)
    uint16_t timerX = LCD_PIXEL_WIDTH - (CHAR_WIDTH * (TIMER_NUMCHARS-1)); // Adjust based on character width (5 chars)
    uint16_t timerY = Y_OFF;

    // Display the timer text in white
    LCD_SetTextColor(LCD_COLOR_WHITE);
    LCD_SetFont(&Font16x24);
    uint16_t currentX = timerX; // Track position for each character
    for (const char *p = timerString; *p != '\0'; p++) {
        LCD_DisplayChar(currentX, timerY, *p);
        currentX += CHAR_WIDTH; // Advance by character width
    }
}

void ClearFullRows(void) {
    uint8_t clearedRows = 0;

    for (int8_t y = GRID_HEIGHT - 1; y >= 0; y--) {
        bool isFullRow = true;

        // Check if the row is full
        for (uint8_t x = 0; x < GRID_WIDTH; x++) {
            if (blockMatrix[y][x] == 0x0000) {
                isFullRow = false;
                break;
            }
        }

        if (isFullRow) {
            clearedRows++;

            // Clear the row
            for (uint8_t x = 0; x < GRID_WIDTH; x++) {
                blockMatrix[y][x] = 0x0000;
            }

            // Shift rows above down
            for (int8_t row = y; row > 0; row--) {
                for (uint8_t col = 0; col < GRID_WIDTH; col++) {
                    blockMatrix[row][col] = blockMatrix[row - 1][col];
                }
            }

            // Clear the top row (after shift)
            for (uint8_t x = 0; x < GRID_WIDTH; x++) {
                blockMatrix[0][x] = 0x0000;
            }

            // Adjust row index to recheck the current row after shifting
            y++;
        }
    }

    // Update scoring based on the number of cleared rows
    switch (clearedRows) {
        case 1:
            singles++;
            break;
        case 2:
            doubles++;
            break;
        case 3:
            triples++;
            break;
        case 4:
            tetri++;
            break;
        default:
            break;
    }
}


void SpawnFallingBlock(uint8_t blockIndex) {
    // Clear the current falling block matrix
    for (uint8_t y = 0; y < GRID_HEIGHT; y++) {
        for (uint8_t x = 0; x < GRID_WIDTH; x++) {
            fallingBlockMatrix[y][x] = 0;
        }
    }

    fallingBlock = tetrisBlocks[blockIndex];

    // Place the block at the top center of the grid
    uint8_t startX = (GRID_WIDTH - fallingBlock.width) / 2;
    for (uint8_t y = 0; y < fallingBlock.height; y++) {
        for (uint8_t x = 0; x < fallingBlock.width; x++) {
            if (fallingBlock.shape[y][x]) {
                fallingBlockMatrix[y][startX + x] = fallingBlock.shape[y][x];
            }
        }
    }
}

void SpawnRandomBlock(void) {
    if (bagIndex >= BAG_SIZE) {
        ShuffleBag();
        bagIndex = 0;
    }

    currentFallingBlock = nextBlock;
    nextBlock = blockBag[bagIndex++];

    if (nextBlock == lastBlock && BAG_SIZE > 1) {
        uint8_t swapIndex = (bagIndex < BAG_SIZE) ? bagIndex : 0;
        uint8_t temp = nextBlock;
        nextBlock = blockBag[swapIndex];
        blockBag[swapIndex] = temp;
    }

    lastBlock = nextBlock;
    SpawnFallingBlock(currentFallingBlock);
}
