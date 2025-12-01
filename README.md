
# Overcolored

Jogo em C para a disciplina SSC0140 – Sistemas Operacionais I (ICMC/USP).  
O objetivo é demonstrar o uso de threads e semáforos em um modelo produtor–consumidor, usando Raylib apenas como interface gráfica.

---

## 1. Plataforma e estrutura do projeto

Plataforma: Linux (amd64) -> Testado em nixos e arch

Árvore principal do projeto:

```text
.
├── boards.h
├── external
│   └── raylib-5.5_linux_amd64
├── flake.lock
├── flake.nix
├── main.c
├── Makefile
└── README.md

```

A pasta `external/raylib-5.5_linux_amd64` já contém headers e bibliotecas da Raylib.  
Não é necessário instalar Raylib no sistema.

----------

## 2. Compilação e execução

No diretório raiz do projeto:

Compilar:

```sh
make

```

Executar:

```sh
make run

```

Se der erro:

```sh
make clean && make run

```

O binário é gerado em `target/clicker`.

----------

## 3. Como jogar

O jogador se move em um tabuleiro com blocos coloridos, coleta cores e entrega combinações para atender pedidos de clientes.

Controles:

W: cima  
S: baixo  
A: esquerda  
D: direita  
Q: sair do jogo  
R: reiniciar após game over

Resumo da mecânica:

O topo da tela mostra os pedidos ativos (clientes) e o tempo restante de cada um.  
O jogador anda sobre blocos coloridos para formar uma cor (combinação por bits).  
Na área de entrega, se a cor do jogador corresponder a algum pedido, esse cliente é atendido.  
Se o tempo acabar e o pedido não for atendido, o cliente vai embora insatisfeito, o jogador perde pontos e acumula erros.  
Ao atingir o limite de erros (2), o jogo entra em estado de game over e mostra a pontuação final.

Local para screenshot da tela principal:

![Tela principal](screenshots/tela-principal.png)

Local para screenshot de game over:

![Game over](screenshots/game-over.png)

----------

## 4. Threads e semáforo no jogo

O jogo usa três tipos de execução:

1.  Thread principal  
    Responsável por inicializar a janela com Raylib, ler o teclado, chamar a função de renderização e controlar o fluxo geral (sair e reiniciar).  
    Nesta thread são criadas as demais threads.
    
2.  Thread produtora  
    Executa a função `produtor`.  
    Cria novos pedidos (clientes) enquanto o jogo não terminou.  
    Cada pedido é inserido em uma lista ligada compartilhada e recebe uma thread própria para controlar o tempo de espera.
    

Trecho essencial:

```c
sem_init(&orders_sem, 0, 1);
pthread_create(&producer_thread, NULL, produtor, NULL);

```

3.  Threads consumidoras  
    Cada pedido é representado por uma thread consumidora, que roda a função `consumidor`.  
    Ela atualiza o tempo restante do cliente e verifica três condições:  
    a) se o jogo acabou, remove o pedido e finaliza;  
    b) se o pedido foi atendido (valor negativo), aumenta a pontuação e remove o pedido;  
    c) se o tempo estourar sem atendimento, aplica penalidade, registra erro e, se necessário, ativa o game over.
    

Criação da thread consumidora:

```c
consumer_args *cargs = malloc(sizeof(consumer_args));
cargs->node = it;
pthread_create(&it->client_thread, NULL, consumidor, cargs);

```

Semáforo:

Existe um único semáforo global:

```c
sem_t orders_sem;

```

Ele protege:

Lista ligada de pedidos (`active_orders`)  
Campos dos pedidos (`v` e `tempo_restante`)  
Variáveis globais de estado (`GAME_OVER`, `score`, `n_erros`)

Exemplos de uso:

Inserção na lista:

```c
sem_wait(&orders_sem);
order_list *it = list_push_back_locked(next_order);
sem_post(&orders_sem);

```

Atualização de pontuação e remoção:

```c
void vai_embora(order_list *order_it, int feliz) {
    sem_wait(&orders_sem);
    if (feliz) {
        score += 10;
    } else {
        score -= 10;
        n_erros += 1;
        if (n_erros == 2)
            GAME_OVER = 1;
    }
    list_remove_locked(order_it);
    sem_post(&orders_sem);
}

```

Leitura segura para desenhar:

```c
int game_over_local;
int score_local;
sem_wait(&orders_sem);
game_over_local = GAME_OVER;
score_local = score;
sem_post(&orders_sem);

```

O semáforo garante exclusão mútua na lista de pedidos e no estado global, evitando condições de corrida entre produtor, consumidores e a thread principal.

Local para screenshot destacando a fila de pedidos:

![Fila de pedidos](screenshots/fila-pedidos.png)

----------

## 5. Vídeo de apresentação

TODO

----------

## 6. Créditos

Trabalho para a disciplina SSC0140 – Sistemas Operacionais I (ICMC/USP).

Integrantes do grupo:

- Gustavo Blois – 13688162
- Lucas Rodrigues Baptista – 15577631  

