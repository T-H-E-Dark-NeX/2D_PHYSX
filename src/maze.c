#include "raylib.h"
#include "stdio.h"
#include "time.h"
#include "stdlib.h"
#include "math.h"


//0 = wall
//1 = path

#define WIDTH 23
#define HEIGHT 23
#define CELL_SIZE 25  // Increased from 20 for bigger cells
#define WALL_HEIGHT 35 // Increased height for better proportions
#define SCREEN_WIDTH 1024
#define SCREEN_HEIGHT 768

// Isometric projection constants
#define ISO_ANGLE 30.0f
#define ISO_SCALE 0.6f  // Increased from 0.4 for bigger appearance

int dir[] = {0,1,2,3}; //directions to be used in random direction


int maze[HEIGHT][WIDTH]; //maze intialize

    // Directions Up, Down, Left, Right
int dx[] = {0, 0, -2, 2};  // Change in x
int dy[] = {-2, 2, 0, 0};  // Change in y



int isvalid(int x,int y){ //check bounds and if cell unvisite(=0)
    return (x>0 && x < WIDTH-1 && y>0 && y<HEIGHT-1 && maze[y][x] == 0 );
}



void intialize(){             //intialize to 0 (unvisited)
    for(int i=0;i<HEIGHT;i++){
        for(int j=0;j<WIDTH;j++){
            maze[i][j]=0;
        }
    }
}

void printMaze() {
    for (int i = 0; i < HEIGHT; i++) {
        for (int j = 0; j < WIDTH; j++) {
            printf(maze[i][j] ? " " : "O"   ); // if value true(!= 0)
        }                                      //prints space path 
                                                 //otherwise wall
        printf("\n");
    }
}

void randomdirection(int arr[],int size){
    for(int i =0;i<size;++i){
        int j = rand() % size;
        int temp = arr[i];
        arr[i] = arr[j];
        arr[j] = temp;
    }
    
    
    
}

void huntAndKill(int startX, int startY) {
    int x = startX, y = startY;
    
    while(1){
     
        randomdirection(dir,4);
        int found = 0;

        for(int i = 0; i < 4; i++){
              
            int nx = x +dx[dir[i]];
            int ny = y +dy[dir[i]];

            if(isvalid(nx,ny)){

                maze[(y+ny)/2][(x+nx)/2] = 1; //make path

                maze[ny][nx] = 1;// make path to other cell

                //dir update
                x = nx;
                y = ny;
                found = 1; //found path
                break; //onto next iteration

            }

            


        }

        if(found == 0){
            int newX = -1, newY = -1;

            for(int i =1;i<HEIGHT;i+=2){

                for(int j=1;j<WIDTH;j+=2){

                    if(maze[i][j]==1){

                        for(int s=0;s<4;s++){

                            int nx = j + dx[s];
                            int ny = i + dy[s];

                            if(isvalid(nx,ny)){
                                newX = j;
                                newY = i;
                                break;
                            }

                        }
                        
                    }
                    if(newX != -1){
                        break;
                    }
                        
                    
                }
                if(newX != -1){
                    break;
                }
            }

            if(newX == -1){
                return; //full grid scaned
            }

            x = newX;
            y = newY; //update x & y

        }





    }


}

// Add these new functions for isometric projection
Vector2 isoProject(float x, float y, float z) {
    float isoX = (x - y) * cos(ISO_ANGLE * DEG2RAD) * ISO_SCALE;
    float isoY = (x + y) * sin(ISO_ANGLE * DEG2RAD) * ISO_SCALE - z * ISO_SCALE;
    return (Vector2){isoX, isoY};
}

void drawWall3D(int x, int y) {
    // Calculate isometric positions for the wall corners
    Vector2 topLeft = isoProject(x * CELL_SIZE, y * CELL_SIZE, WALL_HEIGHT);
    Vector2 topRight = isoProject((x + 1) * CELL_SIZE, y * CELL_SIZE, WALL_HEIGHT);
    Vector2 bottomLeft = isoProject(x * CELL_SIZE, (y + 1) * CELL_SIZE, WALL_HEIGHT);
    Vector2 bottomRight = isoProject((x + 1) * CELL_SIZE, (y + 1) * CELL_SIZE, WALL_HEIGHT);
    
    // Draw the wall faces with white color scheme
    Color wallColor = WHITE;
    Color topColor = {wallColor.r - 30, wallColor.g - 30, wallColor.b - 30, 255};
    Color sideColor = {wallColor.r - 60, wallColor.g - 60, wallColor.b - 60, 255};
    
    // Draw top face with adjusted vertical offset
    DrawTriangle(
        (Vector2){SCREEN_WIDTH/2 + topLeft.x, SCREEN_HEIGHT/3 + topLeft.y},
        (Vector2){SCREEN_WIDTH/2 + topRight.x, SCREEN_HEIGHT/3 + topRight.y},
        (Vector2){SCREEN_WIDTH/2 + bottomLeft.x, SCREEN_HEIGHT/3 + bottomLeft.y},
        topColor
    );
    
    // Draw side face with adjusted vertical offset
    DrawTriangle(
        (Vector2){SCREEN_WIDTH/2 + bottomLeft.x, SCREEN_HEIGHT/3 + bottomLeft.y},
        (Vector2){SCREEN_WIDTH/2 + bottomRight.x, SCREEN_HEIGHT/3 + bottomRight.y},
        (Vector2){SCREEN_WIDTH/2 + topLeft.x, SCREEN_HEIGHT/3 + topLeft.y},
        sideColor
    );
}

void drawMaze() {
    // Draw floor grid with subtle dark gray
    for (int i = 0; i < HEIGHT; i++) {
        for (int j = 0; j < WIDTH; j++) {
            Vector2 pos = isoProject(j * CELL_SIZE, i * CELL_SIZE, 0);
            DrawRectangle(
                SCREEN_WIDTH/2 + pos.x - CELL_SIZE/2.5,
                SCREEN_HEIGHT/3 + pos.y - CELL_SIZE/2.5,
                CELL_SIZE/1.2,
                CELL_SIZE/1.2,
                (Color){40, 40, 40, 255}  // Dark gray for floor
            );
        }
    }
    
    // Draw walls
    for (int i = 0; i < HEIGHT; i++) {
        for (int j = 0; j < WIDTH; j++) {
            if (maze[i][j] == 0) {  // Wall
                drawWall3D(j, i);
            }
        }
    }
}

int main() {
    srand(time(0));
    intialize();
    int startX = (rand() % ((WIDTH-1)/2))*2 +1;
    int startY = (rand() % ((HEIGHT-1)/2))*2 +1;

    maze[startY][startX]=1;

    huntAndKill(startX,startY);
    
    // Initialize window
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "3D Isometric Maze Generator");
    SetTargetFPS(60);

    // Main game loop
    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(BLACK);  // Changed to black background
        
        drawMaze();
        
        EndDrawing();
    }

    CloseWindow();
    return 0;
}