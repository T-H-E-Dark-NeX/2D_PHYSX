#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>
#include "game.h"

#define SCREEN_WIDTH 1024
#define SCREEN_HEIGHT 768
#define BUTTON_WIDTH 200
#define BUTTON_HEIGHT 60
#define BUTTON_SPACING 20

typedef enum {
    MENU_STATE,
    GAME_STATE
} MenuState;

typedef struct {
    Rectangle rect;
    const char* text;
    Color color;
    Color hoverColor;
    bool isHovered;
} Button;

// Music control variables
Music menuMusic;
float musicVolume = 0.5f;  // 50% volume

Button createButton(float x, float y, const char* text) {
    Button button = {
        .rect = {x, y, BUTTON_WIDTH, BUTTON_HEIGHT},
        .text = text,
        .color = WHITE,
        .hoverColor = (Color){200, 200, 200, 255},
        .isHovered = false
    };
    return button;
}

void drawButton(Button* button) {
    Color currentColor = button->isHovered ? button->hoverColor : button->color;
    
    // Draw button background
    DrawRectangleRec(button->rect, currentColor);
    
    // Draw button border
    DrawRectangleLinesEx(button->rect, 2, BLACK);
    
    // Calculate text position to center it
    int textWidth = MeasureText(button->text, 30);
    int textX = button->rect.x + (button->rect.width - textWidth) / 2;
    int textY = button->rect.y + (button->rect.height - 30) / 2;
    
    // Draw text
    DrawText(button->text, textX, textY, 30, BLACK);
}

bool isButtonHovered(Button* button, Vector2 mousePos) {
    return CheckCollisionPointRec(mousePos, button->rect);
}

void drawTitle() {
    const char* title = "MAZE GAME";
    int titleWidth = MeasureText(title, 60);
    int titleX = (SCREEN_WIDTH - titleWidth) / 2;
    int titleY = 100;
    
    // Draw title shadow
    DrawText(title, titleX + 2, titleY + 2, 60, (Color){40, 40, 40, 255});
    // Draw main title
    DrawText(title, titleX, titleY, 60, WHITE);
}

void drawMusicControls() {
    // Draw volume control
    const char* volumeText = "Volume:";
    int volumeX = 20;
    int volumeY = SCREEN_HEIGHT - 40;
    
    DrawText(volumeText, volumeX, volumeY, 20, WHITE);
    
    // Draw volume bar
    Rectangle volumeBar = {volumeX + 80, volumeY + 5, 100, 20};
    DrawRectangleRec(volumeBar, (Color){40, 40, 40, 255});
    DrawRectangle(volumeX + 80, volumeY + 5, musicVolume * 100, 20, WHITE);
    DrawRectangleLinesEx(volumeBar, 2, WHITE);
    
    // Draw volume percentage
    char volumeStr[10];
    sprintf(volumeStr, "%d%%", (int)(musicVolume * 100));
    DrawText(volumeStr, volumeX + 190, volumeY, 20, WHITE);
}

int main() {
    // Initialize window - This should be the only InitWindow call in the program
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "3D Maze Game Menu");
    SetTargetFPS(60);
    
    // Initialize audio device - This should be called only once
    InitAudioDevice();
    
    // Load and play menu music - using OGG file for better compression
    bool musicLoaded = false;
    if (FileExists("resources/menu_music.ogg")) {
        menuMusic = LoadMusicStream("resources/menu_music.ogg");
        musicLoaded = true;
        PlayMusicStream(menuMusic);
        SetMusicVolume(menuMusic, musicVolume);
    }
    else if (FileExists("resources/menu_music.wav")) {
        menuMusic = LoadMusicStream("resources/menu_music.wav");
        musicLoaded = true;
        PlayMusicStream(menuMusic);
        SetMusicVolume(menuMusic, musicVolume);
    }
    else {
        // Fallback: try MP3 if OGG and WAV don't exist
        if (FileExists("resources/menu_music.mp3")) {
            menuMusic = LoadMusicStream("resources/menu_music.mp3");
            musicLoaded = true;
            PlayMusicStream(menuMusic);
            SetMusicVolume(menuMusic, musicVolume);
        }
    }
    
    MenuState currentState = MENU_STATE;
    
    // Create buttons (removing VERY_EASY which isn't properly implemented)
    Button easyButton = createButton(
        (SCREEN_WIDTH - BUTTON_WIDTH) / 2,
        SCREEN_HEIGHT / 2 - BUTTON_HEIGHT - BUTTON_SPACING,
        "EASY"
    );
    
    Button mediumButton = createButton(
        (SCREEN_WIDTH - BUTTON_WIDTH) / 2,
        SCREEN_HEIGHT / 2,
        "MEDIUM"
    );
    
    Button hardButton = createButton(
        (SCREEN_WIDTH - BUTTON_WIDTH) / 2,
        SCREEN_HEIGHT / 2 + BUTTON_HEIGHT + BUTTON_SPACING,
        "HARD"
    );
    
    while (!WindowShouldClose()) {
        Vector2 mousePos = GetMousePosition();
        
        // Update button hover states
        easyButton.isHovered = isButtonHovered(&easyButton, mousePos);
        mediumButton.isHovered = isButtonHovered(&mediumButton, mousePos);
        hardButton.isHovered = isButtonHovered(&hardButton, mousePos);
        
        // Update music
        if (musicLoaded) {
            UpdateMusicStream(menuMusic);
        }
        
        // Handle volume control
        if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
            Rectangle volumeBar = {20 + 80, SCREEN_HEIGHT - 35, 100, 20};
            if (CheckCollisionPointRec(mousePos, volumeBar)) {
                musicVolume = (mousePos.x - volumeBar.x) / volumeBar.width;
                if (musicVolume < 0) musicVolume = 0;
                if (musicVolume > 1) musicVolume = 1;
                if (musicLoaded) {
                    SetMusicVolume(menuMusic, musicVolume);
                }
            }
        }
        
        BeginDrawing();
        ClearBackground(BLACK);
        
        if (currentState == MENU_STATE) {
            drawTitle();
            
            // Draw buttons
            drawButton(&easyButton);
            drawButton(&mediumButton);
            drawButton(&hardButton);
            
            // Draw music controls
            drawMusicControls();
            
            // Handle button clicks
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                if (easyButton.isHovered) {
                    // Stop menu music before launching the game
                    if (musicLoaded) {
                        StopMusicStream(menuMusic);
                    }
                    // Launch the game with easy difficulty
                    startGame(EASY, musicVolume);
                    
                    // When game returns to menu, restart menu music
                    if (musicLoaded) {
                        PlayMusicStream(menuMusic);
                        SetMusicVolume(menuMusic, musicVolume);
                    }
                }
                else if (mediumButton.isHovered) {
                    // Stop menu music before launching the game
                    if (musicLoaded) {
                        StopMusicStream(menuMusic);
                    }
                    // Launch the game with medium difficulty
                    startGame(MEDIUM, musicVolume);
                    
                    // When game returns to menu, restart menu music
                    if (musicLoaded) {
                        PlayMusicStream(menuMusic);
                        SetMusicVolume(menuMusic, musicVolume);
                    }
                }
                else if (hardButton.isHovered) {
                    // Stop menu music before launching the game
                    if (musicLoaded) {
                        StopMusicStream(menuMusic);
                    }
                    // Launch the game with hard difficulty
                    startGame(HARD, musicVolume);
                    
                    // When game returns to menu, restart menu music
                    if (musicLoaded) {
                        PlayMusicStream(menuMusic);
                        SetMusicVolume(menuMusic, musicVolume);
                    }
                }
            }
        }
        else if (currentState == GAME_STATE) {
            // Here you would start the maze game with the selected difficulty
            // For now, we'll just show a message and allow returning to menu
            const char* msg = "Game Started! Press ESC to return to menu";
            int msgWidth = MeasureText(msg, 30);
            DrawText(msg, 
                    (SCREEN_WIDTH - msgWidth) / 2, 
                    SCREEN_HEIGHT / 2, 
                    30, 
                    WHITE);
            
            if (IsKeyPressed(KEY_ESCAPE)) {
                currentState = MENU_STATE;
            }
        }
        
        EndDrawing();
    }
    
    // Cleanup
    if (musicLoaded) {
        UnloadMusicStream(menuMusic);
    }
    
    CloseAudioDevice();
    CloseWindow();
    return 0;
} 