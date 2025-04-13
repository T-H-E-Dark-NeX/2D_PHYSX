#include "raylib.h"
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include "b24cm1070_b24me1067_b24ch1004_b24me1049_music.h"

#define GRID_SIZE 20
#define CELL_SIZE 40
#define MAZE_WIDTH 19
#define MAZE_HEIGHT 19
#define PLAYER_SPEED 4
int ENEMY_SPEED = 2;

typedef struct {
    int x, y;
} Node;


// Maze layout (false = wall, true = path)
bool maze[MAZE_HEIGHT][MAZE_WIDTH];

// Directions for maze generation
static int dir[] = {0, 1, 2, 3};
static int dx[] = {0, 0, -2, 2};
static int dy[] = {-2, 2, 0, 0};

// Function to check if a cell is valid for maze generation
static int isvalid(int x, int y) {
    return (x > 0 && x < MAZE_WIDTH - 1 && y > 0 && y < MAZE_HEIGHT - 1 && maze[y][x] == false);
}

// Initialize the maze to all walls
static void initialize() {
    for (int i = 0; i < MAZE_HEIGHT; i++) {
        for (int j = 0; j < MAZE_WIDTH; j++) {
            maze[i][j] = false;
        }
    }
}

// Shuffle directions for randomness
static void randomdirection(int arr[], int size) {
    for (int i = 0; i < size; ++i) {
        int j = rand() % size;
        int temp = arr[i];
        arr[i] = arr[j];
        arr[j] = temp;
    }
}

// Maze generation algorithm
static void SearchAndDestroy(int startX, int startY) {
    int x = startX, y = startY;

    while (1) {
        randomdirection(dir, 4);
        int found = 0;

        for (int i = 0; i < 4; i++) {
            int nx = x + dx[dir[i]];
            int ny = y + dy[dir[i]];

            if (isvalid(nx, ny)) {
                maze[(y + ny) / 2][(x + nx) / 2] = true; // Remove wall
                maze[ny][nx] = true;                    // Mark path
                x = nx;
                y = ny;
                found = 1;
                break;
            }
        }

        if (!found) {
            int newX = -1, newY = -1;

            for (int i = 1; i < MAZE_HEIGHT; i += 2) {
                for (int j = 1; j < MAZE_WIDTH; j += 2) {
                    if (maze[i][j] == true) {
                        for (int s = 0; s < 4; s++) {
                            int nx = j + dx[s];
                            int ny = i + dy[s];

                            if (isvalid(nx, ny)) {
                                newX = j;
                                newY = i;
                                break;
                            }
                        }
                    }
                    if (newX != -1) break;
                }
                if (newX != -1) break;
            }

            if (newX == -1) return; // Maze generation complete
            x = newX;
            y = newY;
        }
    }
}

// Generate the maze
void generateMaze() {
    srand(time(NULL));
    initialize();
    int startX = (rand() % ((MAZE_WIDTH - 1) / 2)) * 2 + 1;
    int startY = (rand() % ((MAZE_HEIGHT - 1) / 2)) * 2 + 1;

    maze[startY][startX] = true; // Start point
    maze[10][19] = false; // End point
    SearchAndDestroy(startX, startY);
}

// Player and enemy positions
Vector2 player = {60, 60};
Vector2 enemy = {CELL_SIZE * (MAZE_WIDTH - 2)- 20, CELL_SIZE * (2)- 20};
bool gameOver = false;
bool gameWon = false;
bool gamestarted = false;
bool wongame = false;
bool showAboutPage = false;
bool showsettingpage = false;
bool showstartpage = false;
bool hard = false;
bool easy = false;
bool medium = false;

// Check if the player can move to a new position
bool CanMove(float newX, float newY) {
    float radius = (CELL_SIZE / 2) - 2;
    return (maze[(int)(newY / CELL_SIZE)][(int)((newX + radius) / CELL_SIZE)] == true &&
            maze[(int)(newY / CELL_SIZE)][(int)((newX - radius) / CELL_SIZE)] == true &&
            maze[(int)((newY + radius) / CELL_SIZE)][(int)(newX / CELL_SIZE)] == true &&
            maze[(int)((newY - radius) / CELL_SIZE)][(int)(newX / CELL_SIZE)] == true);
}

// Move the player based on input
void MovePlayer() {
    float newX = player.x, newY = player.y;
    if (IsKeyDown(KEY_RIGHT) && CanMove(newX + PLAYER_SPEED, newY)) newX += PLAYER_SPEED;
    if (IsKeyDown(KEY_LEFT) && CanMove(newX - PLAYER_SPEED, newY)) newX -= PLAYER_SPEED;
    if (IsKeyDown(KEY_DOWN) && CanMove(newX, newY + PLAYER_SPEED)) newY += PLAYER_SPEED;
    if (IsKeyDown(KEY_UP) && CanMove(newX, newY - PLAYER_SPEED)) newY -= PLAYER_SPEED;
    player.x = newX;
    player.y = newY;
}

// Move the enemy toward the player
void MoveEnemy1() {
    if(medium){
        ENEMY_SPEED = 1;
    }
    if(hard){
        ENEMY_SPEED = 2;
    }
    float diffX = player.x - enemy.x;
    float diffY = player.y - enemy.y;
    float distance = sqrt(diffX * diffX + diffY * diffY);

    if (distance > 0) {
        enemy.x += (diffX / distance) * ENEMY_SPEED;
        enemy.y += (diffY / distance) * ENEMY_SPEED;
    }
}

void MoveEnemy() {
    float diffX = player.x - enemy.x;
    float diffY = player.y - enemy.y;
    float distance = sqrt(diffX * diffX + diffY * diffY);
    if (distance > 0) {
        float moveX = (diffX / distance) * ENEMY_SPEED;
        float moveY = (diffY / distance) * ENEMY_SPEED;
        if (CanMove(enemy.x + moveX, enemy.y)) enemy.x += moveX;
        if (CanMove(enemy.x, enemy.y + moveY)) enemy.y += moveY;
    }
}

int main() {
    
    InitAudioDevice();

    InitWindow(MAZE_WIDTH * CELL_SIZE, MAZE_HEIGHT * CELL_SIZE, "Maze Game");
    SetTargetFPS(60);

    generateMaze(); // Generate the maze
    Music currentMusic = PlayMenuMusic();

    Image bgImage = LoadImage("BG.png");
    Texture2D background = LoadTextureFromImage(bgImage);
    UnloadImage(bgImage);

    while (!WindowShouldClose()) {
        UpdateMusicStream(currentMusic);
        BeginDrawing();
        ClearBackground(RAYWHITE);

        if (!gamestarted && !showAboutPage && !showstartpage) {
            DrawTexture(background,0,0,WHITE);
            
            Rectangle startBtn = {320, 250, 160, 50};
            Rectangle aboutBtn = {600, 670, 160, 50};
        
            float roundness = 0.3f;
            int segments = 10;
        
            DrawRectangleRounded(startBtn, roundness, segments, BLUE);
            DrawRectangleRounded(aboutBtn, roundness, segments, BLUE);
        
            DrawText("START", 350, 260, 30, BLACK);
            DrawText("ABOUT", 630, 680, 30, BLACK);
        
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                Vector2 mouse = GetMousePosition();
                if (CheckCollisionPointRec(mouse, startBtn)) {
                    showstartpage = true;
                    
                    StopMusicStream(currentMusic);
                    UnloadMusicStream(currentMusic);
                    currentMusic = PlayRandomMusic();
                }
                if (CheckCollisionPointRec(mouse, aboutBtn)) {
                    showAboutPage = true;
                }
            }
        }
        else if (showAboutPage) {
            DrawRectangle(100, 100, 600, 400, LIGHTGRAY);  // Background box
            DrawText("ABOUT THIS GAME", 260, 150, 30, DARKBLUE);
            DrawText("Created by......", 300, 200, 20, BLACK);
            DrawText("Tushar Verma", 220, 320, 20, PINK);
            DrawText("Abhishek Sonparote", 220, 280, 20, GREEN);
            DrawText("Samyag Kothari", 220, 240, 20, RED);
            DrawText("Mrigank Sharma", 220, 360, 20, BLUE);
        
            Rectangle exitBtn = {320, 500, 160, 50};
            float roundness = 0.3f;
            int segments = 10;
        
            DrawRectangleRounded(exitBtn, roundness, segments, BLUE);
            DrawText("EXIT", 360, 510, 30, BLACK);
        
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                Vector2 mouse = GetMousePosition();
                if (CheckCollisionPointRec(mouse, exitBtn)) {
                    showAboutPage = false;  // Return to main menu
                }
            }
        }

        else if (showstartpage) {
            DrawRectangle(95, 100, 600, 600, LIGHTGRAY);  // Background box
            DrawText("WHAT DO YOU WANT\n TO PLAY (MODE)", 300, 200, 30, BLACK);

            Rectangle hardBtn = {320, 300, 160, 50};
            Rectangle mediumBtn = {310, 400, 180, 50};
            Rectangle easyBtn = {320, 500, 160, 50};

            float roundness = 0.3f;
            int segments = 10;

            DrawRectangleRounded(hardBtn, roundness, segments, BLUE);
            DrawRectangleRounded(mediumBtn, roundness, segments, BLUE);
            DrawRectangleRounded(easyBtn, roundness, segments, BLUE);

            DrawText("HARD", 360, 310, 30, PINK);
            DrawText("MEDIUM", 340, 410, 30, GREEN);
            DrawText("EASY", 360, 510, 30, RED);

            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                Vector2 mouse = GetMousePosition();
                if (CheckCollisionPointRec(mouse, easyBtn)) {
                    easy = true;
                    gamestarted = true;
                    player.x = 60;
                    player.y = 60;
                    enemy.x = CELL_SIZE * (MAZE_WIDTH - 2) - 20;
                    enemy.y = CELL_SIZE * (2) - 20;
                    gameOver = false;
                    generateMaze();
                    showstartpage = false;
                }
                if (CheckCollisionPointRec(mouse, mediumBtn)) {
                    medium = true;
                    gamestarted = true;
                    player.x = 60;
                    player.y = 60;
                    enemy.x = CELL_SIZE * (MAZE_WIDTH - 2) - 20;
                    enemy.y = CELL_SIZE * (2) - 20;
                    gameOver = false;
                    generateMaze();
                    showstartpage = false;
                }
                if (CheckCollisionPointRec(mouse, hardBtn)) {
                    hard = true;
                    gamestarted = true;
                    player.x = 60;
                    player.y = 60;
                    enemy.x = CELL_SIZE * (MAZE_WIDTH - 2) - 20;
                    enemy.y = CELL_SIZE * (2) - 20;
                    gameOver = false;
                    generateMaze();
                    showstartpage = false;
                }
            }

            Rectangle exitBtn = {320, 600, 160, 50};
        
            DrawRectangleRounded(exitBtn, roundness, segments, BLUE);
            DrawText("EXIT", 360, 610, 30, BLACK);
        
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                Vector2 mouse = GetMousePosition();
                if (CheckCollisionPointRec(mouse, exitBtn)) {
                    gamestarted = false;
                    showstartpage = false;
                }
            }
        }

        else if (gamestarted){
            if (!gameOver && !gameWon) {
                MovePlayer();
                if (easy) {
                    MoveEnemy();
                }
                if (medium) {
                    MoveEnemy1();
                }
                if (hard) {
                    MoveEnemy1();
                }
            }

            // Draw the maze
            for (int y = 0; y < MAZE_HEIGHT; y++) {
                for (int x = 0; x < MAZE_WIDTH; x++) {
                    if (maze[y][x] == false) {
                        DrawRectangle(x * CELL_SIZE, y * CELL_SIZE, CELL_SIZE, CELL_SIZE, BLACK);
                    }
                }
            }

            DrawCircleV(player, CELL_SIZE / 2, BLUE);
            DrawCircleV(enemy, CELL_SIZE / 2, RED);
            DrawRectangle(700-20, 700-20, 40, 40, GREEN);

            if (player.x == (700) && player.y == (700)) wongame = true;

            if(wongame){
                player.x = 60;
                player.y = 60;
                enemy.x = CELL_SIZE * (MAZE_WIDTH - 2) - 20;
                enemy.y = CELL_SIZE * (2) - 20;
                wongame = false;

                StopMusicStream(currentMusic);
                UnloadMusicStream(currentMusic);
                currentMusic = PlayRandomMusic();
                generateMaze(); // Regenerate the maze
            }

            if (CheckCollisionCircles(player, CELL_SIZE / 2, enemy, CELL_SIZE / 2)) gameOver = true;

            if (gameWon) {
                DrawText("YOU WIN!", 350, 400, 40, GREEN);
                DrawText("Press R to Restart", 200, 450, 40, PURPLE);
                if (IsKeyPressed(KEY_R)) {
                    player.x = 60;
                    player.y = 60;
                    enemy.x = CELL_SIZE * (MAZE_WIDTH - 2) - 20;
                    enemy.y = CELL_SIZE * (2) - 20;
                    gameWon = false;
                    
                    StopMusicStream(currentMusic);
                    UnloadMusicStream(currentMusic);
                    currentMusic = PlayRandomMusic();

                    generateMaze(); // Regenerate the maze
                }
            } else if (gameOver) {
                DrawText("Game Over!", 300, 300, 40, RED);
                DrawText("Press R to Restart", 200, 450, 40, GREEN);
                if (IsKeyPressed(KEY_R)) {
                    hard = false;
                    easy = false;
                    medium = false;
                    showstartpage = true;
                    
                    StopMusicStream(currentMusic);
                    UnloadMusicStream(currentMusic);
                    currentMusic = PlayRandomMusic();
                }
            }
        }

        EndDrawing();
    }

    UnloadMusicStream(currentMusic);
    CloseAudioDevice();

    UnloadTexture(background);
    CloseWindow();
    return 0;
}
