#include <raylib.h>
#include <unistd.h>
#include <stdlib.h>
#include <limits.h>
#include <stdio.h>
#include <time.h>

typedef enum block{
WALL,
COOKTOP,
CUTTING_BOARD,
FLOOR,
} block;


#define LEVEL_SIZE 10
#define screenHeight 500
#define screenWidth 500
block BOARD[LEVEL_SIZE][LEVEL_SIZE] = {FLOOR};

void render(){

    Vector2 offset = {screenWidth%LEVEL_SIZE,screenHeight%LEVEL_SIZE};
    BeginDrawing();
    ClearBackground(GRAY);

    
    Vector2 sizeofsquare = {
        screenWidth/LEVEL_SIZE ,
        screenHeight/LEVEL_SIZE ,
    };

    for(int i = 0; i < LEVEL_SIZE; i++){
        for(int j = 0; j < LEVEL_SIZE; j++){
            DrawRectangleV((Vector2){sizeofsquare.x*i,sizeofsquare.y*j},sizeofsquare,(Color){rand()%255, rand()%255, rand()%255, rand()%255});
        }
    }
    for (int i = 0; i < screenWidth/LEVEL_SIZE + 1; i++){
        DrawLineV((Vector2){i*sizeofsquare.x, 0}, (Vector2){i*sizeofsquare.x, screenHeight}, LIGHTGRAY);
    }

    for (int i = 0; i < screenHeight/LEVEL_SIZE + 1; i++){
        DrawLineV((Vector2){0, i*sizeofsquare.y}, (Vector2){screenWidth, i*sizeofsquare.y}, LIGHTGRAY);
    }




    EndDrawing();

}

int main(){
    int clicks = 0;
    srand(time(NULL));
    InitWindow(500,500,"clicker");
    SetTargetFPS(60);
    while(!WindowShouldClose()){
        render();
        if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)){
            clicks += 1;
            printf("clicks: %d\n",clicks);
        }
        if (IsKeyPressed('Q')){
            CloseWindow();
            break;
        }
        
    }

    return 0;
}