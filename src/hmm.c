#define     UP 1
#define  RIGHT 2
#define   DOWN 4
#define   LEFT 8
#define   DEAD 16
#define MASKED 32

#define       white 0xffffffff
#define       black 0xff000000
#define transparent 0x00000000

#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>


typedef unsigned char byte; // Add this line to define the byte type

static void dfs_mazer( byte *M, int x, int y, byte dir, int W, int H ){

	int yW = y * W;

	if( M[x + yW] == 0 ) M[x + yW] = dir;
	else M[x + yW] |= dir;

	switch( dir ){
		case    UP: y -= 1; break;
		case RIGHT: x += 1; break;
		case  DOWN: y += 1; break;
		case  LEFT: x -= 1; break;
	}

	yW = y * W;

	byte options [4];
	int o_n;

	check_again:;

	o_n = 0;
	
	if( y > 0   && M[x + (yW-W)] == 0 ){ //UP
		options[0] = 1;
		o_n += 1;
	}else options[0] = 0;

    if( x < W-1 && M[x+1 + yW] == 0 ){ //RIGHT
    	options[1] = 1;
    	o_n += 1;
    }else options[1] = 0;

    if( y < H-1 && M[x + (yW+W)] == 0 ){ //DOWN
    	options[2] = 1;
    	o_n += 1;
    }else options[2] = 0;

    if( x > 0    && M[x-1 + yW] == 0 ){ //LEFT
    	options[3] = 1;
    	o_n += 1;
    }else options[3] = 0;


    //printf("x: %d, y: %d, pD: %d, dir: %hhd, depth %d, o_n:%d\n", x, y, pD, dir, depth, o_n );

    if( o_n == 0 ){
    	if( M[x + yW] == 0 ) M[x + yW] = DEAD;
		else M[x + yW] |= DEAD;
		//printf("died at depth %d, D: %d.\n", depth, D[x][y] );
    }
    else if ( o_n == 1 ){
    	for(int i = 0; i < 4; i++){
    		if( options[i] ){
    			dfs_mazer( M, x, y, 1 << i, W, H );
    			break;
    		}
    	}
    }
    else{
    	int o = rand() % o_n;
    	int c = 0;
    	for(int i = 0; i < 4; i++){
    		if( options[i] ){
    			if( c == o ){
					dfs_mazer( M, x, y, 1 << i, W, H );
					goto check_again;
				}
				else c++;
    		}
    	}
    }
}


void build_maze( byte *M, int W, int H ){

	//byte *M = calloc( W * H, 1 );
	int sx = rand() % W;
	int sy = rand() % H;
	byte dir = 1 << (rand() % 4);
	if( sx == 0   && dir == LEFT  ) dir = RIGHT;
	if( sx == W-1 && dir == RIGHT ) dir = LEFT;
	if( sy == 0   && dir == UP    ) dir = DOWN;
	if( sy == H-1 && dir == DOWN  ) dir = UP;
	//printf("mazin' %d x %d, %d\n", sx, sy, dir );
	dfs_mazer( M, sx, sy, dir, W, H );
	//return M;
}



void build_maze_masked( byte *M, byte *mask, int *unmasked, int W, int H ){
	//printf("building masked maze. W: %d, H: %d, mask: %p\n", W, H, mask );

	int un = 0;
	for(int j = 0; j < H; j++){
		int J = j * W;
		for(int i = 0; i < W; i++){
			if( mask[i + J] ){
				M[i + J] = MASKED;
			}
			else unmasked[un++] = i + J;
		}
	}
	if( un == 0 ){
		puts("fully masked!");
		return;
	}

	int si = unmasked[ rand() % un ];
	int sy = si / W;
	int sx = si - (sy*W);
	byte dir = 1 << (rand() % 4);

	if( sx == 0   && dir == LEFT  ) dir = RIGHT;
	if( sx == W-1 && dir == RIGHT ) dir = LEFT;
	if( sy == 0   && dir == UP    ) dir = DOWN;
	if( sy == H-1 && dir == DOWN  ) dir = UP;

	//printf( "si: %d, sx: %d, sy: %d\n", si, sx, sy );
	
	dfs_mazer( M, sx, sy, dir, W, H );
}


byte *build_meta_maze( byte *meta, int meta_w, int meta_h, int cell_w, int cell_h ){

	int W = meta_w * cell_w;
	int H = meta_h * cell_h;
	byte *M = calloc( W * H, 1 );

	int ssize = cell_w * cell_h;
	byte *sector = malloc( ssize );

	for(int i = 0; i < meta_w; i++){
		for(int j = 0; j < meta_h; j++){

			//printf("\nbegin cell %d, %d\n", i, j );

			memset( sector, 0, ssize );
			build_maze( sector, cell_w, cell_h );
			//puts("built cell");

			for(int x = 0; x < cell_w; x++){
				for(int y = 0; y < cell_h; y++){
					M[ x + (i*cell_w) + ((y + (j*cell_h)) * W) ] = sector[ x + (y*cell_w) ];
				}
			}//puts("copied cell");
			
			int here = i + (j * meta_w);

			if( meta[ here ] & UP ){
				int N = 1;//random_from_list( 7, 1, 1, 1, 1, 2, 2, 3 );
				int min = cell_w;
				int max = 0;
				int offset = (i*cell_w) + ((j*cell_h) * W);
				for(int x = 0; x < cell_w; x++){
					if( M[ x + offset ] != MASKED ){
						if( x < min ) min = x;
						if( x > max ) max = x;
					}
				}
				max += 1;
				for(int n = 0; n < N; n++){
					int x = rand() % (max - min) + min;
					M[ x + offset ] |= UP;
				}
			}	
			if( meta[ here ] & RIGHT ){
				int N = 1;//random_from_list( 7, 1, 1, 1, 1, 2, 2, 3 );
				int min = cell_h;
				int max = 0;
				int offset = (cell_w-1) + (i*cell_w) + ((j*cell_h) * W);
				for(int y = 0; y < cell_h; y++){
					if( M[ offset + (y * W)  ] != MASKED ){
						if( y < min ) min = y;
						if( y > max ) max = y;
					}
				}
				max += 1;
				for(int n = 0; n < N; n++){
					int y = rand() % (max - min) + min;
					M[ offset + (y * W) ] |= RIGHT;
				}
			}	
			if( meta[ here ] & DOWN  ){
				int N = 1;//random_from_list( 7, 1, 1, 1, 1, 2, 2, 3 );
				int min = cell_w;
				int max = 0;
				int offset = (i*cell_w) + ( ((cell_h-1)+(j*cell_h)) * W );
				for(int x = 0; x < cell_w; x++){
					if( M[ x + offset ] != MASKED ){
						if( x < min ) min = x;
						if( x > max ) max = x;
					}
				}
				max += 1;
				for(int n = 0; n < N; n++){
					int x = rand() % (max - min) + min;
					M[ x + offset ] |= DOWN;
				}
			}	
			if( meta[ here ] & LEFT  ){
				int N = 1;//random_from_list( 7, 1, 1, 1, 1, 2, 2, 3 );
				int min = cell_h;
				int max = 0;
				int offset = (i*cell_w) + ((j*cell_h) * W);
				for(int y = 0; y < cell_h; y++){
					if( M[ offset + (y * W) ] != MASKED ){
						if( y < min ) min = y;
						if( y > max ) max = y;
					}
				}
				max += 1;
				for(int n = 0; n < N; n++){
					int y = rand() % (max - min) + min;
					M[ offset + (y * W) ] |= LEFT;
				}
			}
			//puts("did connections");
		}
	}

	free( sector );   //puts("freed sector");

	return M;
}



Texture2D rasterize_maze(byte *M, int W, int H, int pw) {
    int pw1 = pw + 1;
    int FW = (W * pw1) + 1;
    int FH = (H * pw1) + 1;
    Image image = GenImageColor(FW, FH, BLACK);

    for (int j = H - 1; j >= 0; j--) {
        int jW = j * W;
        int y = pw1 * j;
        for (int i = W - 1; i >= 0; i--) {
            int x = pw1 * i;
            if (M[i + jW] & MASKED || M[i + jW] == 0) {
                for (int l = 0; l <= pw; l++) {
                    for (int k = 0; k <= pw; k++) {
                        ImageDrawPixel(&image, x + k, y + l, BLACK);
                    }
                }
            } else {
                ImageDrawPixel(&image, x, y, WHITE);
                for (int l = 1; l <= pw; l++) {
                    for (int k = 1; k <= pw; k++) {
                        ImageDrawPixel(&image, x + k, y + l, BLACK);
                    }
                }
                // LEFT WALL
                if (i > 0 && ((M[i + jW] & LEFT) || (M[i - 1 + jW] & RIGHT))) {
                    for (int l = 1; l <= pw; l++) {
                        ImageDrawPixel(&image, x, y + l, BLACK);
                    }
                } else {
                    for (int l = 1; l <= pw; l++) {
                        ImageDrawPixel(&image, x, y + l, WHITE);
                    }
                }
                // TOP WALL
                if (j > 0 && ((M[i + jW] & UP) || (M[i + ((j - 1) * W)] & DOWN))) {
                    for (int k = 1; k <= pw; k++) {
                        ImageDrawPixel(&image, x + k, y, BLACK);
                    }
                } else {
                    for (int k = 1; k <= pw; k++) {
                        ImageDrawPixel(&image, x + k, y, WHITE);
                    }
                }
                // RIGHT WALL
                if (i == W - 1 || M[i + 1 + jW] == MASKED || M[i + 1 + jW] == 0) {
                    for (int l = 0; l <= pw1; l++) {
                        ImageDrawPixel(&image, x + pw1, y + l, WHITE);
                    }
                }
                // BOTTOM WALL
                if (j == H - 1 || M[i + ((j + 1) * W)] == MASKED || M[i + ((j + 1) * W)] == 0) {
                    for (int k = 0; k <= pw1; k++) {
                        ImageDrawPixel(&image, x + k, y + pw1, WHITE);
                    }
                }
            }
        }
    }

    return LoadTextureFromImage(image);
}


bool *boolify_maze(byte *M, int W, int H, int pw ){

	int pw1 = pw+1;
	int FW = (W * pw1) + 1;
	int FH = (H * pw1) + 1;

	bool *bm = malloc( FW * FH * sizeof(byte) );

	for(int j = H-1; j >= 0; j--){
		int jW = j * W;
		int y = pw1 * j;
		int yFW = y * FW;
		for(int i = W-1; i >= 0; i--){

			int x = pw1 * i;

			if( M[ i + jW ] & MASKED || M[ i + jW ] == 0 ){
				for(int l = 0; l <= pw; l++){
					int ylW = (y+l)*FW;
					for(int k = 0; k <= pw; k++){

						bm[ x + k + ylW ] = 0;
					}
				}
			}
			else{

				int x = pw1 * i;
				bm[ x + yFW ] = 1;

				for(int l = 1; l <= pw; l++){
					int ylW = (y+l)*FW;
					for(int k = 1; k <= pw; k++){

						bm[ x + k + ylW ] = 0;
					}
				}

				//LEFT WALL
				if( i > 0 && ((M[ i + jW ] & LEFT) || (M[ i-1 + jW ] & RIGHT)) ){
					for(int l = 1; l <= pw; l++){
						bm[ x + ((y+l)*FW) ] = 0;
					}
				}
				else{
					for(int l = 1; l <= pw; l++){
						bm[ x + ((y+l)*FW) ] = 1;
					}
				}
				//TOP WALL
				if( j > 0 && ((M[ i + jW ] & UP) || (M[ i + ((j-1)*W) ] & DOWN)) ){
					for(int k = 1; k <= pw; k++){
						bm[ x+k + yFW ] = 0;
					}
				}
				else{
					for(int k = 1; k <= pw; k++){
						bm[ x+k + yFW ] = 1;
					}
				}
				//RIGHT WALL
				if( i == W-1 || M[ i+1 + jW ] == MASKED || M[ i+1 + jW ] == 0 ){
					for(int l = 0; l <= pw1; l++){
						bm[ x+pw1 + ((y+l)*FW) ] = 1;
					}
				}
				//BOTTOM WALL
				if( j == H-1 || M[ i + ((j+1)*W) ] == MASKED || M[ i + ((j+1)*W) ] == 0 ){
					for(int k = 0; k <= pw1; k++){
						bm[ x+k + ((y+pw1)*FW) ] = 1;
					}
				}
			}
		}
	}

	return bm;
}

int main(void) {
    // Initialization
    const int screenWidth = 800;
    const int screenHeight = 600;

    InitWindow(screenWidth, screenHeight, "Maze Generator");

    SetTargetFPS(60);

    // Example usage of the maze functions
    int W = 20;
    int H = 20;
    byte *maze = calloc(W * H, sizeof(byte));
    build_maze(maze, W, H);
    Texture2D mazeTexture = rasterize_maze(maze, W, H, 10);

    // Main game loop
    while (!WindowShouldClose()) {
        // Update

        // Draw
        BeginDrawing();
        ClearBackground(RAYWHITE);
        DrawTexture(mazeTexture, 0, 0, WHITE);
        EndDrawing();
    }

    // De-Initialization
    UnloadTexture(mazeTexture);
    free(maze);
    CloseWindow();

    return 0;
}