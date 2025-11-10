all: build run
	
build:
	gcc main.c -o ./target/clicker -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
run:
	./target/clicker
