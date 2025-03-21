#include "stdio.h"
#include "time.h"
#include "stdlib.h"
#include "windows.h"

//0 = wall
//1 = path

#define WIDTH 511
#define HEIGHT 511
#define NUM_THREADS 16  // Increased to 16 threads
#define VISUALIZATION_DELAY 50  // Milliseconds between visualization updates

int dir[] = {0,1,2,3}; //directions to be used in random direction

int maze[HEIGHT][WIDTH]; //maze initialize
CRITICAL_SECTION mazeLock; // For thread synchronization
CRITICAL_SECTION visualLock; // For visualization synchronization
volatile int activeThreads = 0; // Count of threads still actively carving
volatile BOOL visualizationRunning = TRUE; // Flag to control visualization thread

// Directions Up, Down, Left, Right
int dx[] = {0, 0, -2, 2};  // Change in x
int dy[] = {-2, 2, 0, 0};  // Change in y

// Color codes for different threads
WORD threadColors[16] = {
    FOREGROUND_RED | FOREGROUND_INTENSITY,
    FOREGROUND_GREEN | FOREGROUND_INTENSITY,
    FOREGROUND_BLUE | FOREGROUND_INTENSITY,
    FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY,
    FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_INTENSITY,
    FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY,
    FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY,
    FOREGROUND_RED,
    FOREGROUND_GREEN,
    FOREGROUND_BLUE,
    FOREGROUND_RED | FOREGROUND_GREEN,
    FOREGROUND_RED | FOREGROUND_BLUE,
    FOREGROUND_GREEN | FOREGROUND_BLUE,
    FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE,
    FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY | FOREGROUND_BLUE,
    FOREGROUND_INTENSITY
};

// Thread parameter structure
typedef struct {
    int startX;
    int startY;
    int threadId;
} ThreadParams;

// Structure to track thread positions for visualization
typedef struct {
    int x;
    int y;
    int active;
} ThreadPosition;

ThreadPosition threadPositions[NUM_THREADS];

int isvalid(int x, int y) {
    if (x <= 0 || x >= WIDTH-1 || y <= 0 || y >= HEIGHT-1)
        return 0;
    
    EnterCriticalSection(&mazeLock);
    int result = (maze[y][x] == 0);
    LeaveCriticalSection(&mazeLock);
    
    return result;
}

void initialize() {
    for(int i=0; i<HEIGHT; i++) {
        for(int j=0; j<WIDTH; j++) {
            maze[i][j] = 0;
        }
    }
    
    // Initialize thread positions
    for(int i=0; i<NUM_THREADS; i++) {
        threadPositions[i].x = -1;
        threadPositions[i].y = -1;
        threadPositions[i].active = 0;
    }
}

// Set console cursor position
void setCursorPosition(int x, int y) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    COORD pos = {(SHORT)x, (SHORT)y};
    SetConsoleCursorPosition(hConsole, pos);
}

// Hide console cursor
void hideCursor() {
    HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO info;
    info.dwSize = 100;
    info.bVisible = FALSE;
    SetConsoleCursorInfo(consoleHandle, &info);
}

// Set text color
void setColor(WORD color) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, color);
}

// Reset color to default
void resetColor() {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
}

// Visualization function that runs in a separate thread
DWORD WINAPI visualizationThread(LPVOID param) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    
    // Set console to a reasonable size
    system("mode con cols=120 lines=40");
    
    // Hide cursor
    hideCursor();
    
    // Clear the screen once
    system("cls");
    
    // Get the console window size
    GetConsoleScreenBufferInfo(hConsole, &csbi);
    int consoleWidth = csbi.srWindow.Right - csbi.srWindow.Left + 1;
    int consoleHeight = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
    
    // Calculate the visible section of the maze
    int visibleWidth = consoleWidth - 2;
    int visibleHeight = consoleHeight - 5;
    
    // Display information
    setCursorPosition(0, 0);
    printf("Maze Generation in Progress - 16 Threads Working");
    setCursorPosition(0, 1);
    printf("---------------------------------------------");
    
    // Main visualization loop
    while(visualizationRunning) {
        EnterCriticalSection(&visualLock);
        
        // Display active threads count
        setCursorPosition(0, consoleHeight - 2);
        printf("Active Threads: %-3d", activeThreads);
        
        // Find the center of activity (average position of active threads)
        int centerX = WIDTH / 2;
        int centerY = HEIGHT / 2;
        int activeCount = 0;
        
        for(int i = 0; i < NUM_THREADS; i++) {
            if(threadPositions[i].active) {
                centerX += threadPositions[i].x;
                centerY += threadPositions[i].y;
                activeCount++;
            }
        }
        
        if(activeCount > 0) {
            centerX /= (activeCount + 1);
            centerY /= (activeCount + 1);
        }
        
        // Calculate view boundaries
        int startX = max(1, centerX - visibleWidth / 2);
        int startY = max(1, centerY - visibleHeight / 2);
        int endX = min(WIDTH - 1, startX + visibleWidth);
        int endY = min(HEIGHT - 1, startY + visibleHeight);
        
        // Display the visible section of the maze
        for(int y = startY; y < endY; y++) {
            setCursorPosition(0, y - startY + 3);
            for(int x = startX; x < endX; x++) {
                // Check if this position has a thread in it
                BOOL hasThread = FALSE;
                for(int t = 0; t < NUM_THREADS; t++) {
                    if(threadPositions[t].active && 
                       threadPositions[t].x == x && 
                       threadPositions[t].y == y) {
                        setColor(threadColors[t]);
                        printf("o");
                        hasThread = TRUE;
                        break;
                    }
                }
                
                if(!hasThread) {
                    resetColor();
                    printf(maze[y][x] ? " " : "O");
                }
            }
            printf("   "); // Clear any leftover characters
        }
        
        LeaveCriticalSection(&visualLock);
        
        // Wait a bit before next update
        Sleep(VISUALIZATION_DELAY);
    }
    
    resetColor();
    return 0;
}

void randomdirection(int arr[], int size) {
    for(int i = 0; i < size; ++i) {
        int j = rand() % size;
        int temp = arr[i];
        arr[i] = arr[j];
        arr[j] = temp;
    }
}

// Threaded Hunt and Kill algorithm
DWORD WINAPI threadedHuntAndKill(LPVOID param) {
    ThreadParams* tp = (ThreadParams*)param;
    int x = tp->startX, y = tp->startY;
    int threadId = tp->threadId;
    
    // Update thread position
    EnterCriticalSection(&visualLock);
    threadPositions[threadId].x = x;
    threadPositions[threadId].y = y;
    threadPositions[threadId].active = 1;
    LeaveCriticalSection(&visualLock);
    
    int threadActive = 1;
    
    while(threadActive && activeThreads > 0) {
        int localDir[4] = {0, 1, 2, 3};
        randomdirection(localDir, 4);
        int found = 0;

        for(int i = 0; i < 4; i++) {
            int nx = x + dx[localDir[i]];
            int ny = y + dy[localDir[i]];

            if(isvalid(nx, ny)) {
                EnterCriticalSection(&mazeLock);
                maze[(y+ny)/2][(x+nx)/2] = 1; // make path
                maze[ny][nx] = 1;             // make path to other cell
                LeaveCriticalSection(&mazeLock);

                // Update thread position for visualization
                EnterCriticalSection(&visualLock);
                threadPositions[threadId].x = nx;
                threadPositions[threadId].y = ny;
                LeaveCriticalSection(&visualLock);
                
                // dir update
                x = nx;
                y = ny;
                found = 1; // found path
                
                // Small delay to make visualization visible
                Sleep(10);
                break;     // onto next iteration
            }
        }

        if(found == 0) {
            // Hunt phase
            int newX = -1, newY = -1;
            
            // Check if we can find any valid cell to continue
            for(int i = 1; i < HEIGHT; i += 2) {
                for(int j = 1; j < WIDTH; j += 2) {
                    EnterCriticalSection(&mazeLock);
                    int cellIsPath = (maze[i][j] == 1);
                    LeaveCriticalSection(&mazeLock);
                    
                    if(cellIsPath) {
                        for(int s = 0; s < 4; s++) {
                            int nx = j + dx[s];
                            int ny = i + dy[s];

                            if(isvalid(nx, ny)) {
                                newX = j;
                                newY = i;
                                
                                // Update thread position for visualization
                                EnterCriticalSection(&visualLock);
                                threadPositions[threadId].x = j;
                                threadPositions[threadId].y = i;
                                LeaveCriticalSection(&visualLock);
                                
                                break;
                            }
                        }
                    }
                    if(newX != -1) break;
                }
                if(newX != -1) break;
            }

            if(newX == -1) {
                // This thread is done, decrement active threads
                EnterCriticalSection(&mazeLock);
                activeThreads--;
                threadActive = 0;
                LeaveCriticalSection(&mazeLock);
                
                // Mark thread as inactive for visualization
                EnterCriticalSection(&visualLock);
                threadPositions[threadId].active = 0;
                LeaveCriticalSection(&visualLock);
                
                // Sleep a bit to give other threads a chance
                Sleep(10);
            } else {
                x = newX;
                y = newY; // update x & y
            }
        }
    }
    
    free(tp);
    return 0;
}

// Function to print final maze
void printFinalMaze() {
    system("cls");
    for (int i = 0; i < HEIGHT; i++) {
        for (int j = 0; j < WIDTH; j++) {
            printf(maze[i][j] ? " " : "#");
        }
        printf("\n");
    }
}

int main() {
    srand(time(0));
    initialize();
    
    InitializeCriticalSection(&mazeLock);
    InitializeCriticalSection(&visualLock);
    
    HANDLE threads[NUM_THREADS];
    HANDLE visThread;
    
    // Initialize activeThreads
    activeThreads = NUM_THREADS;
    
    // Create visualization thread
    visThread = CreateThread(
        NULL,                  // Default security attributes
        0,                     // Default stack size
        visualizationThread,   // Thread function
        NULL,                  // Parameter to thread function
        0,                     // Run immediately
        NULL                   // Don't need thread ID
    );
    
    // Create worker threads with different starting points
    for (int i = 0; i < NUM_THREADS; i++) {
        ThreadParams* tp = (ThreadParams*)malloc(sizeof(ThreadParams));
        
        // Generate starting point - make sure it's odd to align with the grid
        tp->startX = (rand() % ((WIDTH-1)/2))*2 + 1;
        tp->startY = (rand() % ((HEIGHT-1)/2))*2 + 1;
        tp->threadId = i;
        
        // Set the starting point in the maze
        EnterCriticalSection(&mazeLock);
        maze[tp->startY][tp->startX] = 1;
        LeaveCriticalSection(&mazeLock);
        
        threads[i] = CreateThread(
            NULL,                  // Default security attributes
            0,                     // Default stack size
            threadedHuntAndKill,   // Thread function
            tp,                    // Parameter to thread function
            0,                     // Run immediately
            NULL                   // Don't need thread ID
        );
    }
    
    // Wait for all worker threads to complete
    WaitForMultipleObjects(NUM_THREADS, threads, TRUE, INFINITE);
    
    // Signal visualization thread to stop and wait for it
    visualizationRunning = FALSE;
    WaitForSingleObject(visThread, 1000);
    
    // Close thread handles
    for (int i = 0; i < NUM_THREADS; i++) {
        CloseHandle(threads[i]);
    }
    CloseHandle(visThread);
    
    DeleteCriticalSection(&mazeLock);
    DeleteCriticalSection(&visualLock);
    
    // Print final maze
    printf("\nMaze generation complete! Press Enter to see the final maze...");
    getchar();
    printFinalMaze();
    
    return 0;
}
