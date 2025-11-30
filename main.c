#include <raylib.h>
#include <unistd.h>
#include <stdlib.h>
#include <limits.h>
#include <stdio.h>
#include <time.h>
#include <semaphore.h>
#include <pthread.h>

// STOVE: STOVE
// CTBD: cutting board
// FLOR: floor
// DLVR: deliver (onde entregamos os pedidos)
typedef enum block{
WALL,
BRED,
BBLUE,
BYELL,
FLOR,
DLVR,
} block;

typedef struct pos_st {
    int x;
    int y;
} Pos;

typedef struct player_st{
    Pos pos;
    short int color;
} Player;

typedef struct customer_st{
    Pos pos;
} Customer;

#define LEVEL_SIZE_Y 12
#define LEVEL_SIZE_X 10
#define N_THREADS LEVEL_SIZE_X
#define screenHeight 500
#define screenWidth 500

#include "boards.h"
const char *game_over_message = "O JOGO ACABOU";
int GAME_OVER = 0;

void render(block board[LEVEL_SIZE_Y][LEVEL_SIZE_X], Player *p1, int customers[N_THREADS]){

   // Vector2 offset = {screenWidth%LEVEL_SIZE_X,screenHeight%LEVEL_SIZE_Y};
    BeginDrawing();
    ClearBackground(GRAY);

    if (GAME_OVER == 1) {
        ClearBackground(RED);
        DrawText(game_over_message, screenWidth/2, screenHeight/2, 20, WHITE);
    } else{
    Vector2 sizeofsquare = {
        screenWidth/LEVEL_SIZE_X ,
        screenHeight/LEVEL_SIZE_Y ,
    };
for (int y = 0; y < LEVEL_SIZE_Y; y++) {
    for (int x = 0; x < LEVEL_SIZE_X; x++) {
        switch (board[y][x]) {
            case FLOR:
                DrawRectangleV(
                    (Vector2){ sizeofsquare.x * x, sizeofsquare.y * y },
                    sizeofsquare,
                    (Color){50, 50, 50, 255}
                );
                break;
            case BYELL:
                DrawRectangleV(
                    (Vector2){ sizeofsquare.x * x, sizeofsquare.y * y },
                    sizeofsquare,
                    YELLOW
                );
                break;
            case BRED:
                DrawRectangleV(
                    (Vector2){ sizeofsquare.x * x, sizeofsquare.y * y },
                    sizeofsquare,
                    RED
                );
                break;
            case BBLUE:
                DrawRectangleV(
                    (Vector2){ sizeofsquare.x * x, sizeofsquare.y * y },
                    sizeofsquare,
                    BLUE
                );
                break;
            case DLVR:
                DrawRectangleV(
                    (Vector2){ sizeofsquare.x * x, sizeofsquare.y * y },
                    sizeofsquare,
                    (Color){50, 255, 50, 255}
                );
                break;
        }
    }
}
    for(int i = 0; i < N_THREADS; i++) {
        if (customers[i] != 0){
            DrawRectangleV((Vector2){sizeofsquare.x*i,0},sizeofsquare,BLUE);
        }
    }
    for (int i = 0; i < screenWidth/LEVEL_SIZE_X + 1; i++){
        DrawLineV((Vector2){i*sizeofsquare.x, 0}, (Vector2){i*sizeofsquare.x, screenHeight}, LIGHTGRAY);
    }

    for (int i = 0; i < screenHeight/LEVEL_SIZE_Y + 1; i++){
        DrawLineV((Vector2){0, i*sizeofsquare.y}, (Vector2){screenWidth, i*sizeofsquare.y}, LIGHTGRAY);
    }

    switch(p1->color){
        case(0b000):
            DrawRectangleV((Vector2){sizeofsquare.x*p1->pos.x,sizeofsquare.y*p1->pos.y},sizeofsquare,BLACK);
            break;
        case(0b001):
            DrawRectangleV((Vector2){sizeofsquare.x*p1->pos.x,sizeofsquare.y*p1->pos.y},sizeofsquare,BLUE);
            break;
        case(0b010):
            DrawRectangleV((Vector2){sizeofsquare.x*p1->pos.x,sizeofsquare.y*p1->pos.y},sizeofsquare,YELLOW);
            break;
        case(0b011):
            DrawRectangleV((Vector2){sizeofsquare.x*p1->pos.x,sizeofsquare.y*p1->pos.y},sizeofsquare,GREEN);
            break;
        case(0b100):
            DrawRectangleV((Vector2){sizeofsquare.x*p1->pos.x,sizeofsquare.y*p1->pos.y},sizeofsquare,RED);
            break;
        case(0b101):
            DrawRectangleV((Vector2){sizeofsquare.x*p1->pos.x,sizeofsquare.y*p1->pos.y},sizeofsquare,PURPLE);
            break;
        case(0b110):
            DrawRectangleV((Vector2){sizeofsquare.x*p1->pos.x,sizeofsquare.y*p1->pos.y},sizeofsquare,ORANGE);
            break;
        case(0b111):
            DrawRectangleV((Vector2){sizeofsquare.x*p1->pos.x,sizeofsquare.y*p1->pos.y},sizeofsquare,WHITE);
            break;
    }



}
    EndDrawing();

}

int init_player(block board[LEVEL_SIZE_Y][LEVEL_SIZE_X],Player *p1){
    p1->color = 0b000;
    for(int i = 1; i < LEVEL_SIZE_Y; i++){
        for(int j = 0; j < LEVEL_SIZE_X; j++){
            if (board[i][j] == FLOR) {
                p1->pos.x = j;
                p1-> pos.y =i;
                return 0;
            }
        }
    }
    return -1;
}

void update_player_color(Player *p1, block color){
    switch(color){
        case(BRED):
            p1->color = (p1->color & 0b111) | 0b100;
            break;
        case(BYELL):
            p1->color = (p1->color & 0b111) | 0b010;
            break;
        case(BBLUE):
            p1->color = (p1->color & 0b111) | 0b001;
            break;
        default:
            break;
    }
}
void deliver(Player *p1){
    p1->color = 0x000;
}

void moveUp(block board[LEVEL_SIZE_Y][LEVEL_SIZE_X],Player *p1){
    if (p1->pos.y < LEVEL_SIZE_Y){
        if (board[p1->pos.y - 1][p1->pos.x] == FLOR){
            p1->pos.y -= 1;
        }
        else if (board[p1->pos.y - 1][p1->pos.x] == DLVR){
            deliver(p1);
        }
        else if (board[p1->pos.y - 1][p1->pos.x] != WALL){
            update_player_color(p1, board[p1->pos.y - 1][p1->pos.x]);
        }
    }
}

void moveDown(block board[LEVEL_SIZE_Y][LEVEL_SIZE_X],Player *p1){
    if (p1->pos.y > 0){
        if (board[p1->pos.y + 1][p1->pos.x] == FLOR){
            p1->pos.y += 1;
        }
        else if (board[p1->pos.y + 1][p1->pos.x] == DLVR){
            deliver(p1);
        }
        else if (board[p1->pos.y + 1][p1->pos.x] != WALL){
            update_player_color(p1, board[p1->pos.y + 1][p1->pos.x]);
        }
    }
}

void moveLeft(block board[LEVEL_SIZE_Y][LEVEL_SIZE_X],Player *p1){
    if (p1->pos.x > 0){
        if (board[p1->pos.y][p1->pos.x - 1] == FLOR){
            p1->pos.x -= 1;
        }
        else if (board[p1->pos.y][p1->pos.x -1] == DLVR){
            deliver(p1);
        }
        else if (board[p1->pos.y ][p1->pos.x-1] != WALL){
            update_player_color(p1, board[p1->pos.y][p1->pos.x -1]);
        }
    }
}

void moveRight(block board[LEVEL_SIZE_Y][LEVEL_SIZE_X],Player *p1){
    if (p1->pos.x < LEVEL_SIZE_X){
        if (board[p1->pos.y][p1->pos.x + 1] == FLOR){
            p1->pos.x += 1;
        }
        else if (board[p1->pos.y][p1->pos.x +1] == DLVR){
            deliver(p1);
        }
        else if (board[p1->pos.y ][p1->pos.x+1] != WALL){
            update_player_color(p1, board[p1->pos.y][p1->pos.x +1]);
        }
    }
}

int find_open_thread_index(int threads[N_THREADS]){
    for(int i = 0; i < N_THREADS; i++){
        if (threads[i] == 0){
            return i;
        }
    }
    return -1;
}

void * imprime_thread(void* numero_thread){
    printf("Alocada a thread %ld\n",(long) numero_thread);
}



int main(){
    srand(time(NULL));
    InitWindow(500,500,"clicker");
    SetTargetFPS(60);
    Player p1;
    int customer_buffer[N_THREADS] = {0};
    pthread_t customer_threads[N_THREADS];
    if(init_player(map2, &p1) < 0){
        printf("Could not position player\n");
        return -1;
    }

    time_t initial_time = time(NULL); 

    while(!WindowShouldClose()){
        render(map2,&p1,customer_buffer);
        time_t current_time = time(NULL);

        
        if (IsKeyPressed('Q')){
            CloseWindow();
            break;
        }

        if (GAME_OVER) {continue;}
        if (IsKeyPressed('W')){
            moveUp(map2,&p1);
        }
        if (IsKeyPressed('S')){
            moveDown(map2,&p1);
        }
        if (IsKeyPressed('A')){
            moveLeft(map2,&p1);
        }
        if (IsKeyPressed('D')){
            moveRight(map2,&p1);
        }
        if (difftime(current_time, initial_time) >= 2.0){
            initial_time = current_time;
            int customer_thread_id = find_open_thread_index(customer_buffer);
            if (customer_thread_id < 0){
                GAME_OVER = 1;
            } else {
                if (pthread_create(&customer_threads[customer_thread_id],NULL, imprime_thread,(void*)customer_thread_id) != 0){
                    printf("Erro ao criar thread\n");
                    return 1;
                }
                customer_buffer[customer_thread_id] = 1;

            }
            printf("Se passaram 10 segundos\n");
    }
}

    return 0;
}
