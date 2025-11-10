#include <raylib.h>
#include <unistd.h>
#include <stdlib.h>
#include <limits.h>
#include <stdio.h>


int main(){
    int clicks = 0;
    InitWindow(500,500,"clicker");
    SetTargetFPS(60);
    char buffer[32];
    while(!WindowShouldClose()){
        if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)){
            clicks += 1;
            printf("clicks: %d\n",clicks);
        }
        if (clicks >= 10){
            CloseWindow();
            break;
        }
        snprintf(buffer,sizeof(buffer),"%d",clicks);
        BeginDrawing();
        ClearBackground(BLACK);
        DrawText(buffer,10,10,10,WHITE);
        EndDrawing();
    }

    return 0;
}