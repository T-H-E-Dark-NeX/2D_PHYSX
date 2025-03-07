
#include "stdio.h"
#include "time.h"
#include "stdlib.h"
#include "windows.h"



//0 = wall
//1 = path

#define WIDTH 993
#define HEIGHT 993

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
            printf(maze[i][j] ? " " : "#"   ); // if value true(!= 0)
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
         //   printMaze();
         //  Sleep(10); // 50ms delay (requires unistd.h on Linux)
         //  system("cls"); // Clear screen for animation


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
                          //   printMaze();
                         //    Sleep(10);
                          //   system("cls");


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




int main() {
    srand(time(0));
    intialize();
    int startX = (rand() % ((WIDTH-1)/2))*2 +1;
    int startY = (rand() % ((HEIGHT-1)/2))*2 +1;

    maze[startY][startX]=1;

    huntAndKill(startX,startY);
    
    printMaze();

    return 0;
}