#include <raylib.h>
#include <stdlib.h>
#include <time.h>

#define WIDTH 21   // Must be odd
#define HEIGHT 21  // Must be odd
#define CELL_SIZE 30 // Size of each cell in pixels

char maze[HEIGHT][WIDTH];

// Directions: Right, Left, Down, Up
int dx[] = {2, -2, 0, 0};
int dy[] = {0, 0, 2, -2};

// Initialize the maze with walls
void initialize_maze() {
    for (int i = 0; i < HEIGHT; i++) {
        for (int j = 0; j < WIDTH; j++) {
            maze[i][j] = '#';  // Fill everything with walls
        }
    }
}

// Check if a cell is within the maze bounds
int is_valid(int x, int y) {
    return (x > 0 && x < HEIGHT - 1 && y > 0 && y < WIDTH - 1);
}

// Recursive function to generate the maze
void carve_maze(int x, int y) {
    maze[x][y] = '.';  // Mark the current cell as a path

    // Shuffle directions for randomness
    int order[] = {0, 1, 2, 3};
    for (int i = 0; i < 4; i++) {
        int r = rand() % 4;
        int temp = order[i];
        order[i] = order[r];
        order[r] = temp;
    }

    // Explore neighbors
    for (int i = 0; i < 4; i++) {
        int nx = x + dx[order[i]];
        int ny = y + dy[order[i]];
        
        if (is_valid(nx, ny) && maze[nx][ny] == '#') {
            maze[x + dx[order[i]] / 2][y + dy[order[i]] / 2] = '.'; // Remove wall
            carve_maze(nx, ny);
        }
    }
}

// Draw the maze using Raylib
void draw_maze() {
    for (int i = 0; i < HEIGHT; i++) {
        for (int j = 0; j < WIDTH; j++) {
            if (maze[i][j] == '#') {
                DrawRectangle(j * CELL_SIZE, i * CELL_SIZE, CELL_SIZE, CELL_SIZE, BLACK);
            } else {
                DrawRectangle(j * CELL_SIZE, i * CELL_SIZE, CELL_SIZE, CELL_SIZE, WHITE);
            }
        }
    }
}

int main() {
    srand(time(NULL));  // Seed random generator
    InitWindow(WIDTH * CELL_SIZE, HEIGHT * CELL_SIZE, "Maze Generator - Raylib");
    SetTargetFPS(60);

    initialize_maze();  // Set up walls
    carve_maze(1, 1);   // Generate maze from (1,1)

    while (!WindowShouldClose()) {
        if (IsKeyPressed(KEY_R)) { // Press 'R' to regenerate the maze
            initialize_maze();
            carve_maze(1, 1);
        }

        BeginDrawing();
        ClearBackground(RAYWHITE);
        draw_maze();
        DrawText("Press R to regenerate", 10, 10, 20, RED);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
gi