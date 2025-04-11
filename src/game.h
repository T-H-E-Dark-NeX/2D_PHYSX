#ifndef GAME_H
#define GAME_H

#include "raylib.h"

// Difficulty levels
typedef enum {
    VERY_EASY,
    EASY,
    MEDIUM,
    HARD,
    EXPERT
} Difficulty;

// Start the game with specified difficulty
void startGame(Difficulty difficulty, float musicVolume);

#endif // GAME_H 