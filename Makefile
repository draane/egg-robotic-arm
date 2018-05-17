SHELL = /bin/sh

CC = gcc
CFLAGS = -Wall

build: bin bin/main.o
	$(CC) bin/*.o -o bin/main.out $(CFLAGS)

bin/main.o: bin/manager_io.o
	$(CC) -c src/main.c -o bin/main.o $(CFLAGS)

bin/manager_io.o: bin/manager_input.o bin/manager_output.o
	$(CC) -c src/manager_io.c -o bin/manager_io.o $(CFLAGS)

bin/manager_output.o: bin/output_pin.o bin/gpio.o
	$(CC) -c src/manager_output.c -o bin/manager_output.o $(CFLAGS)

bin/output_pin.o: bin/gpio.o
	$(CC) -c src/output_pin.c -o bin/output_pin.o $(CFLAGS)

bin/manager_input.o:
	$(CC) -c src/manager_input.c -o bin/manager_input.o $(CFLAGS)

bin/gpio.o:
	$(CC) -c src/gpio.c -o bin/gpio.o $(CFLAGS)

bin:
	mkdir bin

clean:
	rm -rfv bin/

rebuild: clean build
