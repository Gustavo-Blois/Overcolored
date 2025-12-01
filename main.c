#include <raylib.h>
#include <unistd.h>
#include <stdlib.h>
#include <limits.h>
#include <stdio.h>
#include <time.h>
#include <semaphore.h>
#include <pthread.h>
#include <fcntl.h>
#include <errno.h>

typedef enum block {
    WALL,
    BRED,
    BBLUE,
    BYELL,
    FLOR,
    DLVR,
    TRSH
} block;

typedef struct pos_st {
    int x;
    int y;
} Pos;

typedef struct player_st {
    Pos pos;
    short int color;
} Player;

#define LEVEL_SIZE_Y 11
#define LEVEL_SIZE_X 10
#define N_ACTIVE_ORDERS 5

#include "boards.h"

const char *game_over_message = "O JOGO ACABOU\n Aperte Q para sair\n Sua pontuação é %d\n\nAperte R para recomeçar";

float tempo_spawn_cliente = 1.0f;
float tempo_espera_cliente = 15.0f;

int GAME_OVER = 0;
int n_erros = 0;
int score = 0;

typedef struct node {
    int v;
    float tempo_restante;
    struct node *next;
    pthread_t client_thread;
} order_list;

sem_t orders_sem;
order_list *active_orders = NULL;
pthread_t producer_thread;

typedef struct {
    order_list *node;
} consumer_args;

order_list* list_push_back_locked(int e) {
    order_list *node = active_orders;
    if (!node) {
        active_orders = malloc(sizeof(order_list));
        active_orders->v = e;
        active_orders->tempo_restante = tempo_espera_cliente;
        active_orders->next = NULL;
        return active_orders;
    }
    while (node->next)
        node = node->next;
    node->next = malloc(sizeof(order_list));
    node->next->v = e;
    node->next->tempo_restante = tempo_espera_cliente;
    node->next->next = NULL;
    return node->next;
}

void list_remove_locked(order_list *it) {
    if (!it)
        return;
    order_list *node = active_orders;
    order_list *prev = NULL;
    while (node) {
        if (node == it) {
            if (prev)
                prev->next = node->next;
            else
                active_orders = node->next;
            free(node);
            return;
        }
        prev = node;
        node = node->next;
    }
}

int any_open_slot_locked() {
    int count = 0;
    order_list *node = active_orders;
    while (node) {
        count++;
        node = node->next;
    }
    return count < N_ACTIVE_ORDERS;
}

order_list* get_next_matching_color_locked(int color) {
    order_list *node = active_orders;
    while (node) {
        if (node->v == color)
            return node;
        node = node->next;
    }
    return NULL;
}

void vai_embora(order_list *order_it, int feliz) {
    sem_wait(&orders_sem);
    if (feliz) {
        score += 10;
    } else {
		if (score > 0){
        	score -= 10;
		}
			n_erros += 1;
        if (n_erros == 2)
            GAME_OVER = 1;
    }
    list_remove_locked(order_it);
    sem_post(&orders_sem);
}

void* consumidor(void *args) {
    consumer_args *cargs = args;
    order_list *it = cargs->node;
    free(cargs);

    time_t initial_time = time(NULL);
    float tempo_de_espera = (float)(rand() % 5);
    struct timespec ts;
    ts.tv_sec = 0;
    ts.tv_nsec = 100000000L;

    while (1) {
        time_t current_time = time(NULL);
        double elapsed = difftime(current_time, initial_time);

        sem_wait(&orders_sem);
        int game_over_local = GAME_OVER;
        int v_local = it ? it->v : -2;
        float restante = (float)(tempo_espera_cliente + tempo_de_espera - elapsed);
        if (restante < 0.0f)
            restante = 0.0f;
        if (it)
            it->tempo_restante = restante;
        sem_post(&orders_sem);

        if (!it)
            return NULL;

        if (game_over_local) {
            sem_wait(&orders_sem);
            list_remove_locked(it);
            sem_post(&orders_sem);
            return NULL;
        }

        if (v_local < 0) {
            vai_embora(it, 1);
            printf("Cliente foi embora satisfeito :D\n");
            return NULL;
        }

        if (elapsed >= tempo_espera_cliente + tempo_de_espera) {
            vai_embora(it, 0);
            printf("Cliente foi embora não satisfeito :(\n");
            return NULL;
        }

        nanosleep(&ts, NULL);
    }

    return NULL;
}

void* produtor(void *args) {
    (void)args;
    struct timespec ts;
    while (1) {
        sem_wait(&orders_sem);
        int game_over_local = GAME_OVER;
        int can_spawn = any_open_slot_locked();
        sem_post(&orders_sem);

        if (game_over_local)
            break;

        if (can_spawn) {
            int next_order = (rand() % 7) + 1;
            sem_wait(&orders_sem);
            order_list *it = list_push_back_locked(next_order);
            sem_post(&orders_sem);

            consumer_args *cargs = malloc(sizeof(consumer_args));
            cargs->node = it;
            pthread_create(&it->client_thread, NULL, consumidor, cargs);
        }

        float proximo_spawn = (float)(rand() % 5);
        float delay = tempo_spawn_cliente + proximo_spawn;
        ts.tv_sec = (time_t)delay;
        ts.tv_nsec = (long)((delay - (float)ts.tv_sec) * 1000000000L);
        nanosleep(&ts, NULL);
    }
    return NULL;
}

int init_player(block board[LEVEL_SIZE_Y][LEVEL_SIZE_X], Player *p1) {
    p1->color = 0b000;
    for (int i = 1; i < LEVEL_SIZE_Y; i++) {
        for (int j = 0; j < LEVEL_SIZE_X; j++) {
            if (board[i][j] == FLOR) {
                p1->pos.x = j;
                p1->pos.y = i;
                return 0;
            }
        }
    }
    return -1;
}

void update_player_color(Player *p1, block color) {
    switch (color) {
        case BRED:
            p1->color = (p1->color & 0b111) | 0b100;
            break;
        case BYELL:
            p1->color = (p1->color & 0b111) | 0b010;
            break;
        case BBLUE:
            p1->color = (p1->color & 0b111) | 0b001;
            break;
        case TRSH:
            p1->color = 0b000;
            break;
        default:
            break;
    }
}

void deliver(Player *p1) {
    if (p1->color == 0)
        return;
    sem_wait(&orders_sem);
    order_list *it = get_next_matching_color_locked(p1->color);
    if (it) {
        it->v = -1;
        it->tempo_restante = -1.0f;
        p1->color = 0b000;
    }
    sem_post(&orders_sem);
}

void moveUp(block board[LEVEL_SIZE_Y][LEVEL_SIZE_X], Player *p1) {
    if (p1->pos.y < LEVEL_SIZE_Y) {
        if (board[p1->pos.y - 1][p1->pos.x] == FLOR) {
            p1->pos.y -= 1;
        } else if (board[p1->pos.y - 1][p1->pos.x] == DLVR) {
            deliver(p1);
        } else if (board[p1->pos.y - 1][p1->pos.x] != WALL) {
            update_player_color(p1, board[p1->pos.y - 1][p1->pos.x]);
        }
    }
}

void moveDown(block board[LEVEL_SIZE_Y][LEVEL_SIZE_X], Player *p1) {
    if (p1->pos.y > 0) {
        if (board[p1->pos.y + 1][p1->pos.x] == FLOR) {
            p1->pos.y += 1;
        } else if (board[p1->pos.y + 1][p1->pos.x] == DLVR) {
            deliver(p1);
        } else if (board[p1->pos.y + 1][p1->pos.x] != WALL) {
            update_player_color(p1, board[p1->pos.y + 1][p1->pos.x]);
        }
    }
}

void moveLeft(block board[LEVEL_SIZE_Y][LEVEL_SIZE_X], Player *p1) {
    if (p1->pos.x > 0) {
        if (board[p1->pos.y][p1->pos.x - 1] == FLOR) {
            p1->pos.x -= 1;
        } else if (board[p1->pos.y][p1->pos.x - 1] == DLVR) {
            deliver(p1);
        } else if (board[p1->pos.y][p1->pos.x - 1] != WALL) {
            update_player_color(p1, board[p1->pos.y][p1->pos.x - 1]);
        }
    }
}

void moveRight(block board[LEVEL_SIZE_Y][LEVEL_SIZE_X], Player *p1) {
    if (p1->pos.x < LEVEL_SIZE_X) {
        if (board[p1->pos.y][p1->pos.x + 1] == FLOR) {
            p1->pos.x += 1;
        } else if (board[p1->pos.y][p1->pos.x + 1] == DLVR) {
            deliver(p1);
        } else if (board[p1->pos.y][p1->pos.x + 1] != WALL) {
            update_player_color(p1, board[p1->pos.y][p1->pos.x + 1]);
        }
    }
}

void render(block board[LEVEL_SIZE_Y][LEVEL_SIZE_X], Player *p1) {
    int game_over_local;
    int score_local;
    sem_wait(&orders_sem);
    game_over_local = GAME_OVER;
    score_local = score;
    sem_post(&orders_sem);

    BeginDrawing();
    ClearBackground(GRAY);

    if (game_over_local) {
        ClearBackground(RED);
        const char *game_over_msg = TextFormat(game_over_message, score_local);
        DrawText(game_over_msg, GetScreenWidth() / 2, GetScreenHeight() / 2, 20, WHITE);
        EndDrawing();
        return;
    }

    Vector2 sizeofsquare = {
        GetScreenWidth() / (float)LEVEL_SIZE_X,
        GetScreenHeight() / (float)LEVEL_SIZE_Y
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
                case TRSH:
                    DrawRectangleV(
                        (Vector2){ sizeofsquare.x * x, sizeofsquare.y * y },
                        sizeofsquare,
                        (Color){0, 255, 255, 255}
                    );
                    break;
                default:
                    break;
            }
        }
    }

    sem_wait(&orders_sem);
    int index = 0;
    for (order_list *node = active_orders; node; node = node->next) {
        switch (node->v) {
            case -1:
                DrawRectangleV((Vector2){sizeofsquare.x * index, 0}, sizeofsquare, LIGHTGRAY);
                break;
            case 0b001:
                DrawRectangleV((Vector2){sizeofsquare.x * index, 0}, sizeofsquare, BLUE);
                break;
            case 0b010:
                DrawRectangleV((Vector2){sizeofsquare.x * index, 0}, sizeofsquare, YELLOW);
                break;
            case 0b011:
                DrawRectangleV((Vector2){sizeofsquare.x * index, 0}, sizeofsquare, GREEN);
                break;
            case 0b100:
                DrawRectangleV((Vector2){sizeofsquare.x * index, 0}, sizeofsquare, RED);
                break;
            case 0b101:
                DrawRectangleV((Vector2){sizeofsquare.x * index, 0}, sizeofsquare, PURPLE);
                break;
            case 0b110:
                DrawRectangleV((Vector2){sizeofsquare.x * index, 0}, sizeofsquare, ORANGE);
                break;
            case 0b111:
                DrawRectangleV((Vector2){sizeofsquare.x * index, 0}, sizeofsquare, WHITE);
                break;
            default:
                break;
        }
        if (node->tempo_restante > 0.0f) {
            DrawText(TextFormat(" %.2f", node->tempo_restante), sizeofsquare.x * index, 0, 12, BLACK);
        }
        index += 1;
    }
    sem_post(&orders_sem);

    for (int i = 0; i < GetScreenWidth() / LEVEL_SIZE_X + 1; i++) {
        DrawLineV((Vector2){i * sizeofsquare.x, 0}, (Vector2){i * sizeofsquare.x, GetScreenHeight()}, LIGHTGRAY);
    }

    for (int i = 0; i < GetScreenHeight() / LEVEL_SIZE_Y + 1; i++) {
        DrawLineV((Vector2){0, i * sizeofsquare.y}, (Vector2){GetScreenWidth(), i * sizeofsquare.y}, LIGHTGRAY);
    }

    DrawText(TextFormat("Pontuação: %d", score_local), 0, sizeofsquare.y * ((float)LEVEL_SIZE_Y - 0.9f), 12, BLACK);

    switch (p1->color) {
        case 0b000:
            DrawRectangleV((Vector2){sizeofsquare.x * p1->pos.x, sizeofsquare.y * p1->pos.y}, sizeofsquare, BLACK);
            break;
        case 0b001:
            DrawRectangleV((Vector2){sizeofsquare.x * p1->pos.x, sizeofsquare.y * p1->pos.y}, sizeofsquare, BLUE);
            break;
        case 0b010:
            DrawRectangleV((Vector2){sizeofsquare.x * p1->pos.x, sizeofsquare.y * p1->pos.y}, sizeofsquare, YELLOW);
            break;
        case 0b011:
            DrawRectangleV((Vector2){sizeofsquare.x * p1->pos.x, sizeofsquare.y * p1->pos.y}, sizeofsquare, GREEN);
            break;
        case 0b100:
            DrawRectangleV((Vector2){sizeofsquare.x * p1->pos.x, sizeofsquare.y * p1->pos.y}, sizeofsquare, RED);
            break;
        case 0b101:
            DrawRectangleV((Vector2){sizeofsquare.x * p1->pos.x, sizeofsquare.y * p1->pos.y}, sizeofsquare, PURPLE);
            break;
        case 0b110:
            DrawRectangleV((Vector2){sizeofsquare.x * p1->pos.x, sizeofsquare.y * p1->pos.y}, sizeofsquare, ORANGE);
            break;
        case 0b111:
            DrawRectangleV((Vector2){sizeofsquare.x * p1->pos.x, sizeofsquare.y * p1->pos.y}, sizeofsquare, WHITE);
            break;
        default:
            break;
    }

    EndDrawing();
}

int main() {
    srand(time(NULL));
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
	InitWindow(500, 500, "clicker");
	SetWindowMinSize(300, 300);

    SetTargetFPS(60);

    Player p1;
    if (init_player(map2, &p1) < 0) {
        printf("Could not position player\n");
        CloseWindow();
        return -1;
    }

    sem_init(&orders_sem, 0, 1);
    GAME_OVER = 0;
    score = 0;
    n_erros = 0;
    active_orders = NULL;

    pthread_create(&producer_thread, NULL, produtor, NULL);

    while (!WindowShouldClose()) {
        render(map2, &p1);

        if (IsKeyPressed('Q')) {
            sem_wait(&orders_sem);
            GAME_OVER = 1;
            sem_post(&orders_sem);

            pthread_join(producer_thread, NULL);

            while (1) {
                sem_wait(&orders_sem);
                int empty = (active_orders == NULL);
                sem_post(&orders_sem);
                if (empty)
                    break;
                struct timespec ts = {0, 100000000L};
                nanosleep(&ts, NULL);
            }

            sem_destroy(&orders_sem);
            CloseWindow();
            return 0;
        }

        int game_over_local;
        sem_wait(&orders_sem);
        game_over_local = GAME_OVER;
        sem_post(&orders_sem);

        if (game_over_local) {
            if (IsKeyPressed('R')) {
                sem_wait(&orders_sem);
                GAME_OVER = 1;
                sem_post(&orders_sem);

                pthread_join(producer_thread, NULL);

                while (1) {
                    sem_wait(&orders_sem);
                    int empty = (active_orders == NULL);
                    sem_post(&orders_sem);
                    if (empty)
                        break;
                    struct timespec ts = {0, 100000000L};
                    nanosleep(&ts, NULL);
                }

                sem_wait(&orders_sem);
                GAME_OVER = 0;
                score = 0;
                n_erros = 0;
				update_player_color(&p1, TRSH);
                sem_post(&orders_sem);

                pthread_create(&producer_thread, NULL, produtor, NULL);
            }
            continue;
        }

        if (IsKeyPressed('W')) {
            moveUp(map2, &p1);
        }
        if (IsKeyPressed('S')) {
            moveDown(map2, &p1);
        }
        if (IsKeyPressed('A')) {
            moveLeft(map2, &p1);
        }
        if (IsKeyPressed('D')) {
            moveRight(map2, &p1);
        }
    }

    sem_wait(&orders_sem);
    GAME_OVER = 1;
    sem_post(&orders_sem);

    pthread_join(producer_thread, NULL);

    sem_destroy(&orders_sem);
    CloseWindow();
    return 0;
}
