#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>  // For memset
#include <time.h>
#include "math.h"
#include "game.h"

// Maze dimensions for 2D game
#define WIDTH 21
#define HEIGHT 21
#define CELL_SIZE 30  // Size of each cell in pixels for 2D rendering

// Make these global variables instead of #defines so they can be modified
int SCREEN_WIDTH = 1024;
int SCREEN_HEIGHT = 768;
#define INITIAL_SCREEN_WIDTH 1024
#define INITIAL_SCREEN_HEIGHT 768
#define MAX_SCREEN_WIDTH 1600
#define MAX_SCREEN_HEIGHT 1200

// Maximum maze size for dynamic resizing
#define MAX_WIDTH 35
#define MAX_HEIGHT 35

// Player and game state
typedef struct {
    int x, y;     // Grid position
    Color color;
} Player;

typedef struct {
    int x, y;     // Grid position
    Color color;
} EndPoint;

typedef struct {
    int x, y;     // Grid position
    Color color;
    float speed;
} Enemy;

typedef struct {
    int maze[MAX_HEIGHT][MAX_WIDTH]; // Use maximum size
    int currentWidth;  // Current maze width
    int currentHeight; // Current maze height
    Player player;
    EndPoint endPoint;
    Enemy enemy;       // Enemy that chases player
    bool gameWon;
    bool gameOver;     // Flag for game over state
    float blinkTimer;  // Timer for player blinking effect
    Sound gameSound;   // Regular game sound
    Sound hardSound;   // Hard mode sound
    Sound invertedSound; // Inverted mode sound
    float musicVolume; // Sound volume
    bool isInverted;   // Flag for inverted color mode
    int gameCount;     // Counter for number of mazes completed
    Difficulty difficulty; // Game difficulty level
    bool enemyActive;  // Whether enemy is active (HARD mode)
    float inversionTimer;  // Timer for random inversion
    float nextInversionTime; // When to trigger the next inversion
    int randomPaths;   // Controls how many random paths/openings are added
} GameState;

// Function declarations
void initializeGame(GameState* game);
void generateMaze(GameState* game);
void drawMaze2D(GameState* game);
void drawPlayer2D(GameState* game);
void drawEndPoint2D(GameState* game);
void drawEnemy2D(GameState* game);
void movePlayer(GameState* game, int dx, int dy);
bool checkWin(GameState* game);
void resetGame(GameState* game);
bool checkInvertedMusicFinished(GameState* game);
void adjustMazeForDifficulty(GameState* game);
void initializeGameWithDifficulty(GameState* game, Difficulty difficulty, float musicVolume);
void updateAndLoopSounds(GameState* game, bool soundsLoaded);
void moveEnemy(GameState* game);
void resetInversionTimer(GameState* game);
bool checkPathExists(GameState* game, int startX, int startY, int targetX, int targetY);
void playSoundWithoutOverlap(GameState* game, Sound soundToPlay, bool soundsLoaded);

// Draw the 2D maze
void drawMaze2D(GameState* game) {
    // Calculate the maze center position to center it on screen
    int mazeWidth = game->currentWidth * CELL_SIZE;
    int mazeHeight = game->currentHeight * CELL_SIZE;
    int startX = (SCREEN_WIDTH - mazeWidth) / 2;
    int startY = (SCREEN_HEIGHT - mazeHeight) / 2;
    
    // Set colors based on inverted mode - white walls and black pathways
    Color wallColor = game->isInverted ? BLACK : WHITE;
    Color pathColor = game->isInverted ? WHITE : BLACK;
    Color floorColor = game->isInverted ? WHITE : BLACK;
    Color gridColor = game->isInverted ? DARKGRAY : LIGHTGRAY;
    
    // Draw maze cells
    for (int i = 0; i < game->currentHeight; i++) {
        for (int j = 0; j < game->currentWidth; j++) {
            int cellX = startX + j * CELL_SIZE;
            int cellY = startY + i * CELL_SIZE;
            
            // Draw floor/background for all cells
            DrawRectangle(cellX, cellY, CELL_SIZE, CELL_SIZE, floorColor);
            
            // Draw walls or paths
            if (game->maze[i][j] == 0) {
                // Draw wall
                DrawRectangle(cellX, cellY, CELL_SIZE, CELL_SIZE, wallColor);
            } else {
                // Draw path
                DrawRectangle(cellX + 2, cellY + 2, CELL_SIZE - 4, CELL_SIZE - 4, pathColor);
            }
            
            // Draw grid lines
            DrawRectangleLines(cellX, cellY, CELL_SIZE, CELL_SIZE, gridColor);
        }
    }
}

void drawPlayer2D(GameState* game) {
    // Calculate position on screen
    int mazeWidth = game->currentWidth * CELL_SIZE;
    int mazeHeight = game->currentHeight * CELL_SIZE;
    int startX = (SCREEN_WIDTH - mazeWidth) / 2;
    int startY = (SCREEN_HEIGHT - mazeHeight) / 2;
    
    int playerX = startX + game->player.x * CELL_SIZE;
    int playerY = startY + game->player.y * CELL_SIZE;
    
    // Calculate blink effect (pulsating)
    float blinkValue = fabsf(sinf(game->blinkTimer * 5.0f));
    
    // Use only black or white for player with grayscale pulse
    Color playerColor;
    if (game->isInverted) {
        // White player on black background
        unsigned char grayValue = (unsigned char)(200 + 55 * blinkValue);
        playerColor = (Color){grayValue, grayValue, grayValue, 255};
    } else {
        // Black player on white background
        unsigned char grayValue = (unsigned char)(50 + 50 * blinkValue);
        playerColor = (Color){grayValue, grayValue, grayValue, 255};
    }
    
    // Draw player as a circle with pulsating size
    float radius = (CELL_SIZE / 2.5f) * (0.8f + 0.2f * blinkValue);
    DrawCircle(
        playerX + CELL_SIZE/2, 
        playerY + CELL_SIZE/2,
        radius,
        playerColor
    );
    
    // Draw player outline
    DrawCircleLines(
        playerX + CELL_SIZE/2, 
        playerY + CELL_SIZE/2,
        radius + 1,
        game->isInverted ? BLACK : WHITE
    );
}

void drawEndPoint2D(GameState* game) {
    // Calculate position on screen
    int mazeWidth = game->currentWidth * CELL_SIZE;
    int mazeHeight = game->currentHeight * CELL_SIZE;
    int startX = (SCREEN_WIDTH - mazeWidth) / 2;
    int startY = (SCREEN_HEIGHT - mazeHeight) / 2;
    
    int endX = startX + game->endPoint.x * CELL_SIZE;
    int endY = startY + game->endPoint.y * CELL_SIZE;
    
    // Endpoint color (always blue)
    Color endpointColor = BLUE;
    
    // Draw endpoint as a flashing star or different shape
    static float pulseTimer = 0.0f;
    pulseTimer += GetFrameTime() * 2.0f;
    float pulseValue = sinf(pulseTimer);
    
    // Use a pentagon shape for the endpoint
    Vector2 center = { endX + CELL_SIZE/2, endY + CELL_SIZE/2 };
    float radius = (CELL_SIZE / 2.0f) * (0.7f + 0.2f * fabsf(pulseValue));
    
    // Draw a diamond shape
    Vector2 points[4] = {
        { center.x, center.y - radius },         // Top
        { center.x + radius, center.y },         // Right
        { center.x, center.y + radius },         // Bottom
        { center.x - radius, center.y }          // Left
    };
    
    DrawTriangle(points[0], points[1], points[2], endpointColor);
    DrawTriangle(points[0], points[2], points[3], endpointColor);
    
    // Draw outline with a darker blue
    Color outlineColor = DARKBLUE;
    DrawLineV(points[0], points[1], outlineColor);
    DrawLineV(points[1], points[2], outlineColor);
    DrawLineV(points[2], points[3], outlineColor);
    DrawLineV(points[3], points[0], outlineColor);
}

void drawEnemy2D(GameState* game) {
    if (!game->enemyActive) return;
    
    // Calculate position on screen
    int mazeWidth = game->currentWidth * CELL_SIZE;
    int mazeHeight = game->currentHeight * CELL_SIZE;
    int startX = (SCREEN_WIDTH - mazeWidth) / 2;
    int startY = (SCREEN_HEIGHT - mazeHeight) / 2;
    
    int enemyX = startX + game->enemy.x * CELL_SIZE;
    int enemyY = startY + game->enemy.y * CELL_SIZE;
    
    // Enemy color
    Color enemyColor = game->isInverted ? 
        (Color){200, 200, 200, 255} : 
        (Color){50, 50, 50, 255};     // Dark gray when normal
    
    // Draw enemy with a pulsating effect
    static float enemyTimer = 0.0f;
    enemyTimer += GetFrameTime() * 3.0f;
    float pulseValue = fabsf(sinf(enemyTimer));
    
    // Draw enemy as a square with rounded corners
    float size = (CELL_SIZE * 0.7f) * (0.8f + 0.2f * pulseValue);
    float offset = (CELL_SIZE - size) / 2;
    
    DrawRectangleRounded(
        (Rectangle){ 
            enemyX + offset, 
            enemyY + offset, 
            size, 
            size 
        },
        0.3f, // Roundness
        8,    // Segments
        enemyColor
    );
    
    // Draw enemy outline
    DrawRectangleRoundedLines(
        (Rectangle){ 
            enemyX + offset, 
            enemyY + offset, 
            size, 
            size 
        },
        0.3f, // Roundness
        8,    // Segments
        game->isInverted ? BLACK : WHITE
    );
}

void movePlayer(GameState* game, int dx, int dy) {
    // Calculate new position
    int newX = game->player.x + dx;
    int newY = game->player.y + dy;
    
    // Check if the new position is valid (within bounds and not a wall)
    if(newX >= 0 && newX < game->currentWidth && 
       newY >= 0 && newY < game->currentHeight && 
       game->maze[newY][newX] == 1) {
        
        // Update player grid position
        game->player.x = newX;
        game->player.y = newY;
    }
}

void initializeGame(GameState* game) {
    // Initialize maze to all walls
    for(int i = 0; i < MAX_HEIGHT; i++) {
        for(int j = 0; j < MAX_WIDTH; j++) {
            game->maze[i][j] = 0;
        }
    }
    
    // Start with base size
    game->currentWidth = WIDTH;
    game->currentHeight = HEIGHT;
    
    // Initialize player
    game->player.x = 1;
    game->player.y = 1;
    game->player.color = (Color){50, 50, 50, 255}; // Dark gray instead of red
    
    // Initialize endpoint
    game->endPoint.x = game->currentWidth - 2;
    game->endPoint.y = game->currentHeight - 2;
    game->endPoint.color = (Color){220, 220, 220, 255}; // Light gray instead of green
    
    // Initialize enemy
    game->enemy.x = game->currentWidth / 2;
    game->enemy.y = game->currentHeight / 2;
    game->enemy.color = (Color){150, 150, 150, 255}; // Gray instead of purple
    
    // Set enemy speed and behavior based on difficulty
    switch(game->difficulty) {
        case VERY_EASY:
            game->enemyActive = false;  // No enemy in VERY_EASY mode
            break;
        case EASY:
            game->enemy.speed = 0.7f;   // Slower enemy in EASY mode
            game->enemyActive = true;
            break;
        case MEDIUM:
            game->enemy.speed = 0.6f;   // Medium speed in MEDIUM mode
            game->enemyActive = true;
            break;
        case HARD:
            game->enemy.speed = 0.5f;   // Faster in HARD mode
            game->enemyActive = true;
            break;
        case EXPERT:
            game->enemy.speed = 0.3f;   // Very fast in EXPERT mode
            game->enemyActive = true;
            break;
        default:
            game->enemy.speed = 0.5f;
            game->enemyActive = false;
    }
    
    game->gameWon = false;
    game->gameOver = false; // Initialize game over flag
    game->blinkTimer = 0.0f; // Initialize blink timer
    game->musicVolume = 0.5f; // Set default sound volume to 50%
    game->isInverted = false; // Start in normal mode
    game->gameCount = 0; // Initialize game counter
    game->randomPaths = 5; // Default value for random paths
    
    // Initialize inversion timer
    resetInversionTimer(game);
}

void resetGame(GameState* game) {
    // Reset player position
    game->player.x = 1;
    game->player.y = 1;
    
    // Reset game state
    game->gameWon = false;
    game->gameOver = false; // Reset game over flag
    game->blinkTimer = 0.0f; // Reset blink timer
    
    // Increment game counter
    game->gameCount++;
    
    // Increase maze complexity with each level completed
    if (game->gameCount > 0) {
        // Increase size every completed level, but don't exceed maximum
        if (game->currentWidth < MAX_WIDTH - 4) {
            game->currentWidth += 2;
        }
        if (game->currentHeight < MAX_HEIGHT - 4) {
            game->currentHeight += 2;
        }
        
        // Reduce the number of random paths to make maze more complex
        // But ensure we don't go below 0
        if (game->randomPaths > 0) {
            game->randomPaths = game->randomPaths > 1 ? game->randomPaths - 1 : 0;
        }
        
        // Adjust endpoint position with new maze size
        game->endPoint.x = game->currentWidth - 2;
        game->endPoint.y = game->currentHeight - 2;
        
        // Every few levels, increase enemy speed if it's active (harder to escape)
        if (game->enemyActive && game->gameCount % 3 == 0 && game->enemy.speed > 0.2f) {
            game->enemy.speed -= 0.05f; // Lower value = faster movement
        }
    }
    
    // Reset inversion timer with random time
    resetInversionTimer(game);
    
    // Generate new maze
    generateMaze(game);
}

// Function to check if the player has reached the goal
bool checkWin(GameState* game) {
    return (game->player.x == game->endPoint.x && game->player.y == game->endPoint.y);
}

// Function to generate the maze using an origin-shifting algorithm
void generateMaze(GameState* game) {
    // First apply difficulty settings to set dimensions
    adjustMazeForDifficulty(game);
    
    // Initialize maze to all walls
    for(int i = 0; i < game->currentHeight; i++) {
        for(int j = 0; j < game->currentWidth; j++) {
            game->maze[i][j] = 0;
        }
    }
    
    // Set up the initial origin point and ensure it's valid
    int originX = 1;
    int originY = 1;
    
    // Make sure player and endpoint positions are valid and within bounds
    if (game->player.x >= game->currentWidth) game->player.x = 1;
    if (game->player.y >= game->currentHeight) game->player.y = 1;
    if (game->endPoint.x >= game->currentWidth) game->endPoint.x = game->currentWidth - 2;
    if (game->endPoint.y >= game->currentHeight) game->endPoint.y = game->currentHeight - 2;
    
    // Ensure player and endpoint are at odd positions for better maze structure
    if (game->player.x % 2 == 0) game->player.x = 1;
    if (game->player.y % 2 == 0) game->player.y = 1;
    if (game->endPoint.x % 2 == 0) game->endPoint.x = game->currentWidth - 2;
    if (game->endPoint.y % 2 == 0) game->endPoint.y = game->currentHeight - 2;
    
    // Mark the start and end as paths
    game->maze[game->player.y][game->player.x] = 1;
    game->maze[game->endPoint.y][game->endPoint.x] = 1;
    
    // For origin shifting, we'll use a multi-origin approach
    // This will create multiple "starting points" for maze generation
    // Typically ensuring more interesting and varied path structures
    
    // Set up directions for carving 
    int dx[] = {0, 0, -2, 2};  // For cell jumps (skip walls)
    int dy[] = {-2, 2, 0, 0};
    int ddx[] = {0, 0, -1, 1}; // For wall carving
    int ddy[] = {-1, 1, 0, 0};
    
    // Stack for backtracking during maze generation
    typedef struct {
        int x, y;
    } Cell;
    
    Cell stack[1000]; // Size should be enough for most mazes
    int stackSize = 0;
    
    // We'll use a visited array to track cells that have been processed
    bool visited[MAX_HEIGHT][MAX_WIDTH];
    memset(visited, 0, sizeof(visited));
    
    // Initial origin point
    stack[stackSize].x = game->player.x;
    stack[stackSize].y = game->player.y;
    stackSize++;
    visited[game->player.y][game->player.x] = true;
    
    // Set up multiple origin points for more variety
    // The number of additional starting points decreases with game count
    // This makes the maze more complex (fewer branches) as you progress
    int baseOrigins = 2 + (game->difficulty / 2);
    int extraOrigins;
    
    if (game->gameCount <= 1) {
        extraOrigins = baseOrigins; // Start with normal number
    } else {
        // Reduce origins as game progresses, but keep at least 1
        extraOrigins = baseOrigins - (game->gameCount / 2);
        if (extraOrigins < 1) extraOrigins = 1;
    }
    
    for (int o = 0; o < extraOrigins; o++) {
        // Place origin at random odd coordinate
        int ox = 1 + 2 * (rand() % ((game->currentWidth-1)/2));
        int oy = 1 + 2 * (rand() % ((game->currentHeight-1)/2));
        
        // Ensure within bounds
        if (ox >= game->currentWidth) ox = game->currentWidth - 2;
        if (oy >= game->currentHeight) oy = game->currentHeight - 2;
        if (ox < 1) ox = 1;
        if (oy < 1) oy = 1;
        
        // Add this origin to our stack if not already visited
        if (!visited[oy][ox]) {
            game->maze[oy][ox] = 1;
            stack[stackSize].x = ox;
            stack[stackSize].y = oy;
            stackSize++;
            visited[oy][ox] = true;
        }
    }
    
    // Make sure the endpoint is always a valid origin
    if (!visited[game->endPoint.y][game->endPoint.x]) {
        stack[stackSize].x = game->endPoint.x;
        stack[stackSize].y = game->endPoint.y;
        stackSize++;
        visited[game->endPoint.y][game->endPoint.x] = true;
    }
    
    // Process cells from our stack until it's empty
    while (stackSize > 0) {
        // Get a random cell from the stack instead of just the top
        // This creates more varied branching
        int idx = rand() % stackSize;
        Cell current = stack[idx];
        
        // Remove the cell by swapping with the last one and decreasing size
        stack[idx] = stack[stackSize - 1];
        stackSize--;
        
        // Try to carve in all 4 directions
        int dir[4] = {0, 1, 2, 3};
        
        // Shuffle directions for more randomness
        for (int i = 0; i < 4; i++) {
            int j = rand() % 4;
            int temp = dir[i];
            dir[i] = dir[j];
            dir[j] = temp;
        }
        
        // Check each direction
        for (int i = 0; i < 4; i++) {
            int nx = current.x + dx[dir[i]];
            int ny = current.y + dy[dir[i]];
            
            // Check if within bounds and not visited
            if (nx > 0 && nx < game->currentWidth - 1 && 
                ny > 0 && ny < game->currentHeight - 1 && 
                !visited[ny][nx]) {
                
                // Carve path by removing the wall between cells
                game->maze[current.y + ddy[dir[i]]][current.x + ddx[dir[i]]] = 1;
                game->maze[ny][nx] = 1;
                
                // Mark as visited
                visited[ny][nx] = true;
                
                // Add to stack for further exploration
                stack[stackSize].x = nx;
                stack[stackSize].y = ny;
                stackSize++;
                
                // Occasionally, shift origin by adding current cell back to stack
                // This creates more loops and branches
                // The chance decreases as game count increases, making mazes more linear and harder
                int loopChance = 10 - game->gameCount;
                if (loopChance < 2) loopChance = 2; // Minimum 2% chance
                
                if (rand() % 100 < loopChance) {
                    stack[stackSize].x = current.x;
                    stack[stackSize].y = current.y;
                    stackSize++;
                }
            }
        }
    }
    
    // Add some random openings based on difficulty setting
    for (int i = 1; i < game->currentHeight-1; i++) {
        for (int j = 1; j < game->currentWidth-1; j++) {
            // If randomPaths is 0, skip this step completely (no random paths for expert)
            if (game->randomPaths == 0) continue;
            
            // Chance to create an opening based on randomPaths value
            if (game->maze[i][j] == 0 && (rand() % 100) < game->randomPaths * 2) {
                // Check surrounding cells to avoid destroying too much structure
                int paths = 0;
                if (game->maze[i-1][j] == 1) paths++;
                if (game->maze[i+1][j] == 1) paths++;
                if (game->maze[i][j-1] == 1) paths++;
                if (game->maze[i][j+1] == 1) paths++;
                
                // Make sure we have at least 2 neighboring paths (more conservative for harder difficulties)
                int requiredPaths = (game->difficulty >= HARD) ? 3 : 2;
                
                // If there are enough paths nearby, make this a path too
                if (paths >= requiredPaths) {
                    game->maze[i][j] = 1;
                }
            }
        }
    }
    
    // Run a pathfinding check to ensure start and end are connected
    // If not, connect them manually
    if (!checkPathExists(game, game->player.x, game->player.y, 
                        game->endPoint.x, game->endPoint.y)) {
        // Carve a path from start to end
    int x = game->player.x;
    int y = game->player.y;
    int targetX = game->endPoint.x;
    int targetY = game->endPoint.y;
    
    while (x != targetX || y != targetY) {
        // Move closer to target
        if (x < targetX) {
            game->maze[y][x+1] = 1;
            x++;
        } else if (x > targetX) {
            game->maze[y][x-1] = 1;
            x--;
        } else if (y < targetY) {
            game->maze[y+1][x] = 1;
            y++;
        } else if (y > targetY) {
            game->maze[y-1][x] = 1;
            y--;
        }
    }
    }
    
    // Place enemy in HARD mode at a distance from player
    if (game->difficulty == HARD || game->difficulty == EXPERT) {
        // Try to place enemy at a position far from player
        for (int i = game->currentHeight-3; i > 0; i--) {
            for (int j = game->currentWidth-3; j > 0; j--) {
                if (game->maze[i][j] == 1 && 
                    abs(j - game->player.x) + abs(i - game->player.y) > game->currentWidth/2) {
                    game->enemy.x = j;
                    game->enemy.y = i;
                    game->enemyActive = true;
                    return;
                }
            }
        }
        
        // Fallback if no good position found
        game->enemy.x = game->currentWidth / 2;
        game->enemy.y = game->currentHeight / 2;
        game->enemyActive = true;
    } else {
        game->enemyActive = false;
    }
}

// Helper function to check if a path exists from start to end using BFS
bool checkPathExists(GameState* game, int startX, int startY, int targetX, int targetY) {
    // BFS search to verify path exists
    typedef struct {
        int x, y;
    } Point;
    
    // Queue for BFS
    Point queue[MAX_WIDTH * MAX_HEIGHT];
    int queueStart = 0;
    int queueEnd = 0;
    
    // Visited array
    bool visited[MAX_HEIGHT][MAX_WIDTH];
    memset(visited, 0, sizeof(visited));
    
    // Add start point to queue
    queue[queueEnd].x = startX;
    queue[queueEnd].y = startY;
    queueEnd++;
    visited[startY][startX] = true;
    
    // Directions for exploration
    int dx[] = {0, 0, -1, 1};
    int dy[] = {-1, 1, 0, 0};
    
    // BFS main loop
    while (queueStart < queueEnd) {
        Point current = queue[queueStart++];
        
        // Check if we reached the target
        if (current.x == targetX && current.y == targetY) {
            return true;
        }
        
        // Try all four directions
        for (int i = 0; i < 4; i++) {
            int nx = current.x + dx[i];
            int ny = current.y + dy[i];
            
            // Check bounds and if it's a path and not visited
            if (nx >= 0 && nx < game->currentWidth && 
                ny >= 0 && ny < game->currentHeight && 
                game->maze[ny][nx] == 1 && !visited[ny][nx]) {
                
                // Add to queue
                queue[queueEnd].x = nx;
                queue[queueEnd].y = ny;
                queueEnd++;
                visited[ny][nx] = true;
            }
        }
    }
    
    // If we get here, no path exists
    return false;
}

// Function to adjust maze based on difficulty level
void adjustMazeForDifficulty(GameState* game) {
    // Set dimensions and path density based on difficulty
    switch (game->difficulty) {
        case VERY_EASY:
            game->currentWidth = 7;
            game->currentHeight = 7;
            game->randomPaths = 10; // More random paths = easier
            break;
        case EASY:
            game->currentWidth = 11;
            game->currentHeight = 11;
            game->randomPaths = 6;  // Reduced from 10 to make slightly harder
            break;
        case MEDIUM:
            game->currentWidth = 15;
            game->currentHeight = 15;
            game->randomPaths = 5;  // Reduced from 7 to make slightly harder
            break;
        case HARD:
            game->currentWidth = 19;
            game->currentHeight = 19;
            game->randomPaths = 3;  // Reduced from 5 to make slightly harder
            break;
        case EXPERT:
            game->currentWidth = 25;
            game->currentHeight = 25;
            game->randomPaths = 0;  // No random paths for expert
            break;
    }
    
    // Make sure endpoint position is updated with new dimensions
    game->endPoint.x = game->currentWidth - 2;
    game->endPoint.y = game->currentHeight - 2;
}

// Function to move enemy toward player
void moveEnemy(GameState* game) {
    if (!game->enemyActive) return;
    
    static float enemyTimer = 0;
    enemyTimer += GetFrameTime();
    
    // Only move every few frames
    if (enemyTimer < game->enemy.speed) return;
    enemyTimer = 0;
    
    // Safety check for valid position
    if (game->enemy.x < 0 || game->enemy.x >= game->currentWidth || 
        game->enemy.y < 0 || game->enemy.y >= game->currentHeight) {
        // Reset to a safe position
        game->enemy.x = game->currentWidth / 2;
        game->enemy.y = game->currentHeight / 2;
        return;
    }
    
    // Improved pathfinding using a more intelligent algorithm
    // Initialize direction variables
    int dx = 0, dy = 0;
    
    // Define possible movement directions (up, right, down, left)
    int directions[4][2] = {{0,-1}, {1,0}, {0,1}, {-1,0}};
    
    // Compute distance from each adjacent cell to the player
    float minDistance = 999999.0f;
    int bestDx = 0, bestDy = 0;
    bool foundPath = false;
    
    // Try each direction
    for (int i = 0; i < 4; i++) {
        int newX = game->enemy.x + directions[i][0];
        int newY = game->enemy.y + directions[i][1];
        
        // Check if this position is valid and is a path
        if (newX >= 0 && newX < game->currentWidth && 
            newY >= 0 && newY < game->currentHeight && 
            game->maze[newY][newX] == 1) {
            
            // Calculate Euclidean distance to player
            float distance = sqrtf(
                powf(newX - game->player.x, 2) + 
                powf(newY - game->player.y, 2)
            );
            
            // If this is the shortest path so far, remember it
            if (distance < minDistance) {
                minDistance = distance;
                bestDx = directions[i][0];
                bestDy = directions[i][1];
                foundPath = true;
            }
        }
    }
    
    // If we found a valid path, move the enemy
    if (foundPath) {
        game->enemy.x += bestDx;
        game->enemy.y += bestDy;
    }
    // If we're stuck, add some randomness (30% chance of random move)
    else if (rand() % 100 < 30) {
        // Shuffle directions
        for (int i = 0; i < 4; i++) {
            int j = rand() % 4;
            int tempX = directions[i][0];
            int tempY = directions[i][1];
            directions[i][0] = directions[j][0];
            directions[i][1] = directions[j][1];
            directions[j][0] = tempX;
            directions[j][1] = tempY;
        }
        
        // Try each direction randomly
        for (int i = 0; i < 4; i++) {
            int newX = game->enemy.x + directions[i][0];
            int newY = game->enemy.y + directions[i][1];
            
            if (newX >= 0 && newX < game->currentWidth && 
                newY >= 0 && newY < game->currentHeight && 
                game->maze[newY][newX] == 1) {
                game->enemy.x = newX;
                game->enemy.y = newY;
                break;
            }
        }
    }
    
    // Check if enemy caught player (with bounds checking)
    if (game->enemy.x >= 0 && game->enemy.x < game->currentWidth && 
        game->enemy.y >= 0 && game->enemy.y < game->currentHeight &&
        game->player.x >= 0 && game->player.x < game->currentWidth && 
        game->player.y >= 0 && game->player.y < game->currentHeight &&
        game->enemy.x == game->player.x && game->enemy.y == game->player.y) {
        // Set game over state
        game->gameOver = true;
    }
}

// Function to check and loop sounds if they have finished playing
void updateAndLoopSounds(GameState* game, bool soundsLoaded) {
    if (!soundsLoaded) return;
    
    // Determine if any sound is currently playing
    bool anyPlaying = IsSoundPlaying(game->gameSound) || 
                      IsSoundPlaying(game->hardSound) || 
                      IsSoundPlaying(game->invertedSound);
    
    // Only start a sound if nothing is playing
    if (!anyPlaying) {
        // Select appropriate sound based on game state
        Sound* soundToPlay = NULL;
        
        if (game->isInverted) {
            soundToPlay = &game->invertedSound;
        } else if (game->difficulty == HARD || game->difficulty == EXPERT) {
            soundToPlay = &game->hardSound;
        } else {
            soundToPlay = &game->gameSound;
        }
        
        // Play the appropriate sound
        if (soundToPlay != NULL) {
            SetSoundVolume(*soundToPlay, game->musicVolume);
            PlaySound(*soundToPlay);
        }
    }
}

// Function to reset inversion timer
void resetInversionTimer(GameState* game) {
    game->inversionTimer = 0.0f;
    // Random time between 30 and 180 seconds (0.5 to 3 minutes)
    game->nextInversionTime = 30.0f + (float)(rand() % 150);
}

// Export game function to be called from menu
void startGame(Difficulty difficulty, float musicVolume) {
    srand(time(0));
    
    // Reset screen dimensions to initial values when starting a new game
    SCREEN_WIDTH = INITIAL_SCREEN_WIDTH;
    SCREEN_HEIGHT = INITIAL_SCREEN_HEIGHT;
    
    // Initialize window only once and check if it's ready
    if (!IsWindowReady()) {
        InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "2D Maze Game");
    } else {
        // If window exists but size is different, resize it
        if (GetScreenWidth() != SCREEN_WIDTH || GetScreenHeight() != SCREEN_HEIGHT) {
            SetWindowSize(SCREEN_WIDTH, SCREEN_HEIGHT);
        }
    }
    SetTargetFPS(60);
    
    // Create a static GameState structure to avoid memory allocation issues
    static GameState game;
    
    // Zero out the game state to avoid uninitialized memory
    memset(&game, 0, sizeof(GameState));
    
    // Set current dimensions
    game.currentWidth = WIDTH;
    game.currentHeight = HEIGHT;
    
    // Set difficulty
    game.difficulty = difficulty;
    
    // Initialize player
    game.player.x = 1;
    game.player.y = 1;
    game.player.color = (Color){50, 50, 50, 255}; // Dark gray instead of red
    
    // Initialize endpoint
    game.endPoint.x = game.currentWidth - 2;
    game.endPoint.y = game.currentHeight - 2;
    game.endPoint.color = (Color){220, 220, 220, 255}; // Light gray instead of green
    
    // Initialize enemy
    game.enemy.x = game.currentWidth / 2;
    game.enemy.y = game.currentHeight / 2;
    game.enemy.color = (Color){150, 150, 150, 255}; // Gray instead of purple
    
    // Set enemy speed and behavior based on difficulty
    switch(difficulty) {
        case VERY_EASY:
            game.enemyActive = false;  // No enemy in VERY_EASY mode
            break;
        case EASY:
            game.enemy.speed = 0.7f;   // Slower enemy in EASY mode
            game.enemyActive = true;
            break;
        case MEDIUM:
            game.enemy.speed = 0.6f;   // Medium speed in MEDIUM mode
            game.enemyActive = true;
            break;
        case HARD:
            game.enemy.speed = 0.5f;   // Faster in HARD mode
            game.enemyActive = true;
            break;
        case EXPERT:
            game.enemy.speed = 0.3f;   // Very fast in EXPERT mode
            game.enemyActive = true;
            break;
        default:
            game.enemy.speed = 0.5f;
            game.enemyActive = false;
    }
    
    game.gameWon = false;
    game.gameOver = false; // Initialize game over flag
    game.blinkTimer = 0.0f; // Initialize blink timer
    game.musicVolume = musicVolume; // Store volume value
    game.isInverted = false; // Start in normal mode
    game.gameCount = 0; // Initialize game counter
    game.randomPaths = 5; // Default value for random paths
    
    // Initialize inversion timer for random color changes
    resetInversionTimer(&game);
    
    // Initialize the maze array
    for(int i = 0; i < MAX_HEIGHT; i++) {
        for(int j = 0; j < MAX_WIDTH; j++) {
            game.maze[i][j] = 0;
        }
    }
    
    // Generate the maze
    generateMaze(&game);
    
    // Try to load sound files instead of music streams
    bool soundsLoaded = false;
    
    // Try to load OGG files first (better compression), then fallback to WAV
    if (FileExists("resources/game_music.ogg")) {
        game.gameSound = LoadSound("resources/game_music.ogg");
        soundsLoaded = true;
    } 
    else if (FileExists("resources/game_music.wav")) {
        game.gameSound = LoadSound("resources/game_music.wav");
        soundsLoaded = true;
    }
    
    if (FileExists("resources/game_music2.ogg")) {
        game.hardSound = LoadSound("resources/game_music2.ogg");
    }
    else if (FileExists("resources/game_music2.wav")) {
        game.hardSound = LoadSound("resources/game_music2.wav");
    }
    else if (soundsLoaded) {
        // Fallback to normal sound
        game.hardSound = game.gameSound;
    }
    
    if (FileExists("resources/inverted_music.ogg")) {
        game.invertedSound = LoadSound("resources/inverted_music.ogg");
    }
    else if (FileExists("resources/inverted_music.wav")) {
        game.invertedSound = LoadSound("resources/inverted_music.wav");
    }
    else if (soundsLoaded) {
        // Fallback to normal sound
        game.invertedSound = game.gameSound;
    }
    
    // Modify the sound-playing portion when starting the game
    // Play appropriate sound based on difficulty - but don't play if a sound is already playing
    if (soundsLoaded) {
        // Play appropriate sound using the non-overlapping function
        if (game.difficulty == HARD) {
            playSoundWithoutOverlap(&game, game.hardSound, soundsLoaded);
        } else {
            playSoundWithoutOverlap(&game, game.gameSound, soundsLoaded);
        }
    }
    
    // Game loop
    while (!WindowShouldClose()) {
        // Update blink timer
        game.blinkTimer += GetFrameTime();
        
        // Update inversion timer
        game.inversionTimer += GetFrameTime();
        
        // Check if inverted music has finished and exit inverted mode if it has
        if (soundsLoaded && checkInvertedMusicFinished(&game)) {
            // Play normal game music when exiting inverted mode
            if (game.difficulty == HARD || game.difficulty == EXPERT) {
                playSoundWithoutOverlap(&game, game.hardSound, soundsLoaded);
            } else {
                playSoundWithoutOverlap(&game, game.gameSound, soundsLoaded);
            }
        }
        
        // Check if it's time for random inversion
        if (game.inversionTimer >= game.nextInversionTime) {
            // Toggle inversion
            game.isInverted = !game.isInverted;
            
            // Play appropriate sound if inversion changed, but don't interrupt if not necessary
            if (soundsLoaded) {
                // Only change the sound if it's significantly different - this prevents music resets
                // For inverting color, we only want to change sound if we're specifically entering inverted mode
                if (game.isInverted) {
                    // Only switch to inverted sound if not already playing
                    if (!IsSoundPlaying(game.invertedSound)) {
                        playSoundWithoutOverlap(&game, game.invertedSound, soundsLoaded);
                    }
                }
                // We don't switch back when leaving inverted mode - let the current song finish
            }
            
            // Reset inversion timer with new random time
            resetInversionTimer(&game);
        }
        
        // Check and loop sounds if needed
        updateAndLoopSounds(&game, soundsLoaded);
        
        // Move enemy in all modes (only if game is not over)
        if (game.enemyActive && !game.gameOver) {
            moveEnemy(&game);
        }
        
        // Handle input (only if game is not over)
        if (!game.gameOver) {
            // Movement controls for 2D grid
            if (IsKeyPressed(KEY_W) || IsKeyPressed(KEY_UP)) {
                movePlayer(&game, 0, -1); // Move up
            }
            if (IsKeyPressed(KEY_S) || IsKeyPressed(KEY_DOWN)) {
                movePlayer(&game, 0, 1);  // Move down
            }
            if (IsKeyPressed(KEY_A) || IsKeyPressed(KEY_LEFT)) {
                movePlayer(&game, -1, 0); // Move left
            }
            if (IsKeyPressed(KEY_D) || IsKeyPressed(KEY_RIGHT)) {
                movePlayer(&game, 1, 0);  // Move right
            }
        } else {
            // Restart the game if SPACE is pressed when game over
            if (IsKeyPressed(KEY_SPACE)) {
                game.gameOver = false;
                game.player.x = 1;
                game.player.y = 1;
                generateMaze(&game);
            }
        }
        
        // Return to menu when ESC is pressed
        if (IsKeyPressed(KEY_ESCAPE)) {
            break;
        }
        
        // Volume control with + and - keys
        if (soundsLoaded) {
        if (IsKeyPressed(KEY_EQUAL)) { // + key
            game.musicVolume += 0.1f;
            if (game.musicVolume > 1.0f) game.musicVolume = 1.0f;
            
                // Update volume of appropriate sound
            if (game.isInverted) {
                    SetSoundVolume(game.invertedSound, game.musicVolume);
                } else if (game.difficulty == HARD) {
                    SetSoundVolume(game.hardSound, game.musicVolume);
            } else {
                    SetSoundVolume(game.gameSound, game.musicVolume);
            }
        }
            
        if (IsKeyPressed(KEY_MINUS)) { // - key
            game.musicVolume -= 0.1f;
            if (game.musicVolume < 0.0f) game.musicVolume = 0.0f;
            
                // Update volume of appropriate sound
            if (game.isInverted) {
                    SetSoundVolume(game.invertedSound, game.musicVolume);
                } else if (game.difficulty == HARD) {
                    SetSoundVolume(game.hardSound, game.musicVolume);
            } else {
                    SetSoundVolume(game.gameSound, game.musicVolume);
                }
            }
        }
        
        // Check for win and automatically start new game
        if (!game.gameWon && checkWin(&game)) {
            game.gameWon = true;
            
            // Small delay to show win state
            WaitTime(1.0f);
            
            // Automatically reset and generate new maze
            resetGame(&game);
            
            // Only update sound if the mode really changed
            // For most level completions, we'll keep the current music playing
            if (soundsLoaded && 
                ((game.isInverted && !IsSoundPlaying(game.invertedSound)) ||
                 (game.difficulty == HARD && !game.isInverted && !IsSoundPlaying(game.hardSound)) ||
                 (!game.isInverted && game.difficulty != HARD && !IsSoundPlaying(game.gameSound)))) {
                
                // Only play a new sound if the appropriate one for the current state isn't playing
                if (game.isInverted) {
                    playSoundWithoutOverlap(&game, game.invertedSound, soundsLoaded);
                } else if (game.difficulty == HARD) {
                    playSoundWithoutOverlap(&game, game.hardSound, soundsLoaded);
                } else {
                    playSoundWithoutOverlap(&game, game.gameSound, soundsLoaded);
                }
            }
        }
        
        BeginDrawing();
        ClearBackground(game.isInverted ? WHITE : BLACK);
        
        // Draw the 2D maze and entities
        drawMaze2D(&game);
        drawEndPoint2D(&game);
        
        // Draw enemy in HARD mode or EXPERT mode
        if ((game.difficulty == HARD || game.difficulty == EXPERT) && game.enemyActive) {
            drawEnemy2D(&game);
        }
        
        drawPlayer2D(&game);
        
        // Draw instructions
        const char* instructions = "Use arrow keys or WASD to move | Find the blue goal | ESC for menu";
        DrawText(instructions, 20, 20, 20, game.isInverted ? BLACK : WHITE);
        
        // Draw volume indicator
        char volumeText[20];
        sprintf(volumeText, "Volume: %d%%", (int)(game.musicVolume * 100));
        DrawText(volumeText, SCREEN_WIDTH - 150, 20, 20, game.isInverted ? BLACK : WHITE);
        
        // Draw difficulty indicator
        const char* difficultyText;
        switch(game.difficulty) {
            case VERY_EASY: difficultyText = "VERY EASY"; break;
            case EASY: difficultyText = "EASY"; break;
            case MEDIUM: difficultyText = "MEDIUM"; break;
            case HARD: difficultyText = "HARD"; break;
            case EXPERT: difficultyText = "EXPERT"; break;
            default: difficultyText = "UNKNOWN";
        }
        DrawText(difficultyText, SCREEN_WIDTH - 150, 50, 20, game.isInverted ? BLACK : WHITE);
        
        // Draw maze size indicator for all difficulty levels
        char sizeText[30];
        sprintf(sizeText, "Maze: %dx%d", game.currentWidth, game.currentHeight);
        DrawText(sizeText, SCREEN_WIDTH - 150, 80, 20, game.isInverted ? BLACK : WHITE);
        
        // Draw game over screen if the game is over
        if (game.gameOver) {
            // Semi-transparent overlay
            DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 
                           (Color){0, 0, 0, 180});
            
            // Game over text
            const char* gameOverText = "GAME OVER";
            int gameOverWidth = MeasureText(gameOverText, 60);
            DrawText(gameOverText, 
                     (SCREEN_WIDTH - gameOverWidth) / 2, 
                     SCREEN_HEIGHT / 2 - 60, 
                     60, 
                     game.isInverted ? BLACK : WHITE);
            
            // Restart instructions
            const char* restartText = "Press SPACE to restart";
            int restartWidth = MeasureText(restartText, 30);
            DrawText(restartText, 
                     (SCREEN_WIDTH - restartWidth) / 2, 
                     SCREEN_HEIGHT / 2 + 20, 
                     30, 
                     game.isInverted ? BLACK : WHITE);
            
            // Menu instructions
            const char* menuText = "Press ESC to return to menu";
            int menuWidth = MeasureText(menuText, 20);
            DrawText(menuText, 
                     (SCREEN_WIDTH - menuWidth) / 2, 
                     SCREEN_HEIGHT / 2 + 60, 
                     20, 
                     game.isInverted ? BLACK : WHITE);
        }
        
        EndDrawing();
    }
    
    // Clean up sounds before returning to menu
    if (soundsLoaded) {
        UnloadSound(game.gameSound);
        UnloadSound(game.hardSound);
        UnloadSound(game.invertedSound);
    }
    
    // Don't close the window here - let the menu handle window management
}

// Fix sound overlapping by ensuring only the necessary sounds are stopped
void playSoundWithoutOverlap(GameState* game, Sound soundToPlay, bool soundsLoaded) {
    if (!soundsLoaded) return;
    
    // Check if the requested sound is already playing
    if ((soundToPlay.frameCount == game->gameSound.frameCount && IsSoundPlaying(game->gameSound)) ||
        (soundToPlay.frameCount == game->hardSound.frameCount && IsSoundPlaying(game->hardSound)) ||
        (soundToPlay.frameCount == game->invertedSound.frameCount && IsSoundPlaying(game->invertedSound))) {
        // Sound is already playing, don't interrupt it
        return;
    }
    
    // Stop only the currently playing sound
    if (IsSoundPlaying(game->gameSound)) StopSound(game->gameSound);
    if (IsSoundPlaying(game->hardSound)) StopSound(game->hardSound);
    if (IsSoundPlaying(game->invertedSound)) StopSound(game->invertedSound);
    
    // Set volume and play the requested sound
    SetSoundVolume(soundToPlay, game->musicVolume);
    PlaySound(soundToPlay);
}

// Function to check if inverted music has finished playing and exit inverted phase
bool checkInvertedMusicFinished(GameState* game) {
    // If the game is not in inverted mode, no need to check
    if (!game->isInverted) return false;
    
    // Check if inverted music is still playing
    if (!IsSoundPlaying(game->invertedSound)) {
        // Music has stopped, turn off inverted mode
        game->isInverted = false;
        return true;
    }
    
    return false;
} 