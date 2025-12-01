#include <raylib.h>
#include <unistd.h>
#include <stdlib.h>
#include <limits.h>
#include <stdio.h>
#include <time.h>
#include <semaphore.h>
#include <pthread.h>

#include <fcntl.h>


// STOVE: STOVE
// CTBD: cutting board
// FLOR: floor
// DLVR: deliver (onde entregamos os pedidos)
// TRSH: trash (descartar itens feitos por engano)
typedef enum block{
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

typedef struct player_st{
    Pos pos;
    short int color;
} Player;


#define LEVEL_SIZE_Y 12
#define LEVEL_SIZE_X 10
#define N_THREADS LEVEL_SIZE_X
#define screenHeight 500
#define screenWidth 500

#define N_ACTIVE_ORDERS 5

#define n_pedidos 10

#include "boards.h"
#include "list.h"

const char *game_over_message = "O JOGO ACABOU";
int GAME_OVER = 0;
bool quit = false;

float tempo_spawn_cliente = 2.0;
float tempo_espera_cliente = 3.0;
float tempo_spawn_inicial = 1.0;

int n_pedidos_feitos = 0;
int n_erros = 0;
int score = 0;


void render(block board[LEVEL_SIZE_Y][LEVEL_SIZE_X], Player *p1, list **orders, sem_t* sem){

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
			case TRSH:
				DrawRectangleV(
                    (Vector2){ sizeofsquare.x * x, sizeofsquare.y * y },
                    sizeofsquare,
                    (Color){0, 255, 255, 255}
                );
        }
    }
}
	sem_wait(sem);

	int index = 0;
    for(list *node = *orders; node; node = node->next) {
		switch(*((int*) node->data)) {
			case -1:
				DrawRectangleV((Vector2){sizeofsquare.x*index,0},sizeofsquare, LIGHTGRAY);
				break;
			case(0b001):
				DrawRectangleV((Vector2){sizeofsquare.x*index,0},sizeofsquare,BLUE);
				break;
			case(0b010):
				DrawRectangleV((Vector2){sizeofsquare.x*index,0},sizeofsquare,YELLOW);
				break;
			case(0b011):
				DrawRectangleV((Vector2){sizeofsquare.x*index,0},sizeofsquare,GREEN);
				break;
			case(0b100):
				DrawRectangleV((Vector2){sizeofsquare.x*index,0},sizeofsquare,RED);
				break;
			case(0b101):
				DrawRectangleV((Vector2){sizeofsquare.x*index,0},sizeofsquare,PURPLE);
				break;
			case(0b110):
				DrawRectangleV((Vector2){sizeofsquare.x*index,0},sizeofsquare,ORANGE);
				break;
			case(0b111):
				DrawRectangleV((Vector2){sizeofsquare.x*index,0},sizeofsquare,WHITE);
				break;
		}

		index += 1;
    }

	sem_post(sem);

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
		case(TRSH):
			p1->color = 0b000;
			break;
    }
}


list* get_first_matching_color(list* orders, int color) {
	list* node = orders;
	while(node) {
		if (*((int*) node->data) == color)
			return node;
		node = node->next;
	}

	return NULL;
}


void deliver(Player *p1, sem_t* orders_buf_semaphore, list* orders) {
	if (p1->color == 0)
		return;

	sem_wait(orders_buf_semaphore);

	struct node* it = get_first_matching_color(orders, p1->color);
	if (it) {
		*((int*) it->data) = -1;
		p1->color = 0b000;
	}

	sem_post(orders_buf_semaphore);
}


void moveUp(block board[LEVEL_SIZE_Y][LEVEL_SIZE_X], Player *p1, sem_t* sem, list* orders){
    if (p1->pos.y < LEVEL_SIZE_Y){
        if (board[p1->pos.y - 1][p1->pos.x] == FLOR){
            p1->pos.y -= 1;
        }
        else if (board[p1->pos.y - 1][p1->pos.x] == DLVR){
            deliver(p1, sem, orders);
        }
        else if (board[p1->pos.y - 1][p1->pos.x] != WALL){
            update_player_color(p1, board[p1->pos.y - 1][p1->pos.x]);
        }
    }
}

void moveDown(block board[LEVEL_SIZE_Y][LEVEL_SIZE_X],Player *p1, sem_t* sem, list* orders){
    if (p1->pos.y > 0){
        if (board[p1->pos.y + 1][p1->pos.x] == FLOR){
            p1->pos.y += 1;
        }
        else if (board[p1->pos.y + 1][p1->pos.x] == DLVR){
            deliver(p1, sem, orders);
        }
        else if (board[p1->pos.y + 1][p1->pos.x] != WALL){
            update_player_color(p1, board[p1->pos.y + 1][p1->pos.x]);
        }
    }
}

void moveLeft(block board[LEVEL_SIZE_Y][LEVEL_SIZE_X],Player *p1, sem_t* sem, list* orders){
    if (p1->pos.x > 0){
        if (board[p1->pos.y][p1->pos.x - 1] == FLOR){
            p1->pos.x -= 1;
        }
        else if (board[p1->pos.y][p1->pos.x -1] == DLVR){
            deliver(p1, sem, orders);
        }
        else if (board[p1->pos.y ][p1->pos.x-1] != WALL){
            update_player_color(p1, board[p1->pos.y][p1->pos.x -1]);
        }
    }
}

void moveRight(block board[LEVEL_SIZE_Y][LEVEL_SIZE_X],Player *p1, sem_t* sem, list* orders){
    if (p1->pos.x < LEVEL_SIZE_X){
        if (board[p1->pos.y][p1->pos.x + 1] == FLOR){
            p1->pos.x += 1;
        }
        else if (board[p1->pos.y][p1->pos.x +1] == DLVR){
            deliver(p1, sem, orders);
        }
        else if (board[p1->pos.y ][p1->pos.x+1] != WALL){
            update_player_color(p1, board[p1->pos.y][p1->pos.x +1]);
        }
    }
}


void* customer(void* args);


// Structs com os argumentos que serão passados
// para as funções de produtor e consumidor.
typedef struct {

	// Semáforo que controla a exclusão mútua
	// do acesso ao buffer de pedidos.
	sem_t* semaphore;

	// Semáforo que guarda o número de threads
	// ativas.
	sem_t* avalible_threads;

	// Listas encadeadas que guardam os pedidos
	// pendentes e as threads ativas.
	list** active_threads;
	list** active_orders;

} producer_args;

typedef struct {

	sem_t* semaphore;
	sem_t* avalible_threads;

	// Nós nas listas encadeadas passadas como
	// argumento para o produtor.
	struct node* order_it;
	struct node* thread_it;

	list** active_orders;
	list** active_threads;
} consumer_args;


void *produtor(void *args) {

	sem_t* sem_buf_orders = ((producer_args*) args)->semaphore;
	sem_t* sem_av_threads = ((producer_args*) args)->avalible_threads;

	list** active_threads =
		((producer_args*) args)->active_threads;

	list** active_orders =
		((producer_args*) args)->active_orders;

	time_t initial_time = time(NULL);
	time_t current_time = time(NULL);

	float tempo_spawn = tempo_spawn_inicial;

	// Cria novas threads para cada cliente a cada intervalo
	// de tempo.
	//
	// Cria até n_pedidos threads.
	for (; n_pedidos_feitos < n_pedidos; ) {

		// Espera um tempo antes de criar um pedido novo.
		while (difftime(current_time, initial_time) < tempo_spawn) {
			current_time = time(NULL);

			if (GAME_OVER || quit)
				return NULL;
		}

		tempo_spawn = tempo_spawn_cliente;

		initial_time = current_time;

		if (GAME_OVER || quit)
			break;

		// Diminui o semáforo de threads disponíveis.
		sem_wait(sem_av_threads);

		sem_wait(sem_buf_orders);

		struct node* order_it = NULL;
		struct node* thread_it = NULL;

		int* next_order = malloc(sizeof(int));
		pthread_t* next_thread = malloc(sizeof(pthread_t));

		// Cria um valor aleatório entre 1 e 7 para
		// o novo pedido e insere ele na lista.
		*next_order = (rand() % 7) + 1;

		order_it = list_push_back(next_order, active_orders);
		thread_it = list_push_back(next_thread, active_threads);
		
		consumer_args c_args = {
			.semaphore = sem_buf_orders,
			.avalible_threads = sem_av_threads,
			.active_orders = active_orders,
			.active_threads = active_threads,
			.order_it = order_it,
			.thread_it = thread_it
		};

		pthread_create(thread_it->data, NULL, &customer, &c_args);

		sem_post(sem_buf_orders);

		n_pedidos_feitos += 1;
	}

	printf("Produtor finalizou.\n");

	return NULL;
}

	// Deleta um cliente da lista de pedidos e da lista de
	// threads.
void vai_embora(
	sem_t* buf_sem,
	list** active_orders,
	list* order_it,
	list** active_threads,
	list* thread_it,
	bool feliz) {

	int pontos_ganhos = 10;
	sem_wait(buf_sem);

	if (feliz)
		score += pontos_ganhos;
	else {
		score -= pontos_ganhos;

		n_erros += 1;
		if (n_erros == 2)
			GAME_OVER = 1;
	}

	list_remove(order_it, active_orders);
	list_remove(thread_it, active_threads);

	sem_post(buf_sem);
}


void* customer(void* args) {

	sem_t* orders_buf_sem = ((consumer_args*) args)->semaphore;
	sem_t* av_threads_sem = ((consumer_args*) args)->avalible_threads;
	list** active_orders = ((consumer_args*) args)->active_orders;
	list** active_threads = ((consumer_args*) args)->active_threads;
	struct node* order_it = ((consumer_args*) args)->order_it;
	struct node* thread_it = ((consumer_args*) args)->thread_it;

	time_t initial_time = time(NULL);
	time_t current_time = time(NULL);

	printf("Cliente criado!\n");

	while(difftime(current_time, initial_time) < tempo_espera_cliente) {
		if (GAME_OVER || quit) {
			sem_wait(orders_buf_sem);

			list_remove(order_it, active_orders);
			list_remove(thread_it, active_threads);

			sem_post(orders_buf_sem);
			sem_post(av_threads_sem);

			return NULL;
		}

		if (*((int*) order_it->data) < 0) {
			vai_embora(orders_buf_sem, active_orders, order_it, active_threads, thread_it, true);

			printf("Cliente foi embora satisfeito :D\n");

			return NULL;
		}

		current_time = time(NULL);
	}

	if (GAME_OVER || quit) {
		sem_wait(orders_buf_sem);

		list_remove(order_it, active_orders);
		list_remove(thread_it, active_threads);

		sem_post(orders_buf_sem);
		sem_post(av_threads_sem);

		return NULL;
	}

	vai_embora(orders_buf_sem, active_orders, order_it, active_threads, thread_it, false);
	printf("Cliente foi embora não satisfeito :(\n");

	sem_post(av_threads_sem);
	return NULL;
}

int setup(
	sem_t** orders,
	sem_t** av_th,
	Player* p1) {

	srand(time(NULL));
	InitWindow(500, 500, "overcooked");
	SetTargetFPS(60);

	if (init_player(map2, p1) < 0) {
		printf("Could not position player.\n");
		return -1;
	}

	// Verifica se os semáforos já existiam antes de
	// iniciar o programa. Isso pode acontecer caso o
	// programa tenha fechado sem deletá-los.

	*orders = sem_open("orderbuf", O_CREAT | O_EXCL, 0644, 1);
	if (*orders == SEM_FAILED)
		sem_unlink("orderbuf");

	*av_th = sem_open("av_threads", O_CREAT | O_EXCL, 0644, N_ACTIVE_ORDERS);
	if (*av_th == SEM_FAILED)
		sem_unlink("av_threads");

	*orders = sem_open("orderbuf", O_CREAT, 0644, 1);
	*av_th = sem_open("av_threads", O_CREAT, 0644, N_ACTIVE_ORDERS);

	return 0;
}

void main_loop(
	Player* p1,
	pthread_t* prod,
	list** active_orders,
	list** active_customers,
	sem_t* orders_buf_sem,
	sem_t* sem_avalible_threads,
	producer_args* prod_args) {
	while(!WindowShouldClose()) {
        render(map2, p1, active_orders, orders_buf_sem);
        time_t current_time = time(NULL);

        
        if (IsKeyPressed('Q')) {
			quit = true;
			CloseWindow();
            break;
        }

		if (GAME_OVER){
			if (IsKeyPressed('R')) {

				// Espera as threads terminarem.
				while (*active_orders || *active_customers);

				n_erros = 0;
				score = 0;

				// Reseta os valores dos semáforos.
				int sem_value;
				sem_getvalue(orders_buf_sem, &sem_value);
				if (!sem_value)
					sem_post(orders_buf_sem);

				sem_getvalue(sem_avalible_threads, &sem_value);
				while (sem_value < N_ACTIVE_ORDERS) {
					sem_post(sem_avalible_threads);
					sem_getvalue(sem_avalible_threads, &sem_value);
				}

				pthread_create(prod, NULL, &produtor, prod_args);

				GAME_OVER = 0;

				continue;
			}

			if (IsKeyPressed('Q')) {
				quit = true;
				CloseWindow();
				break;
			}
		}

        if (IsKeyPressed('W')){
            moveUp(map2,p1, orders_buf_sem, *active_orders);
        }
        if (IsKeyPressed('S')){
            moveDown(map2,p1, orders_buf_sem, *active_orders);
        }
        if (IsKeyPressed('A')){
            moveLeft(map2,p1, orders_buf_sem, *active_orders);
        }
        if (IsKeyPressed('D')){
            moveRight(map2,p1, orders_buf_sem, *active_orders);
        }
    }
}


int main(){
    Player p1;
	pthread_t prod;
	sem_t* orders_buf_sem;
	sem_t* sem_avalible_threads;
	list* active_orders = NULL;
	list* active_customers = NULL;

	if (setup(&orders_buf_sem, &sem_avalible_threads, &p1))
		return -1;

    time_t initial_time = time(NULL);

	producer_args prod_args = {
		.semaphore = orders_buf_sem,
		.avalible_threads = sem_avalible_threads,
		.active_orders = &active_orders,
		.active_threads = &active_customers
	};

	pthread_create(&prod, NULL, &produtor, &prod_args);

    main_loop(
		&p1,
		&prod,
		&active_orders,
		&active_customers,
		orders_buf_sem,
		sem_avalible_threads,
		&prod_args);

	sem_close(orders_buf_sem);
	sem_close(sem_avalible_threads);

	sem_unlink("orderbuf");
	sem_unlink("av_threads");

	free_list(&active_orders);
	free_list(&active_customers);

    return 0;
}
