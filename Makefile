SHELL = /bin/sh

CC = gcc
CFLAGS = -Wall -std=gnu90

build: bin bin/main.o
	$(CC) bin/*.o -o bin/main.out $(CFLAGS)

bin/main.o: bin/manager_io.o
	$(CC) -c src/main.c -o bin/main.o $(CFLAGS)

bin/manager_io.o: bin/manager_input.o bin/manager_output.o
	$(CC) -c src/manager_io.c -o bin/manager_io.o $(CFLAGS)

bin/manager_output.o: bin/output_pin.o bin/gpio.o bin/serial.o
	$(CC) -c src/manager_output.c -o bin/manager_output.o $(CFLAGS)

bin/output_pin.o: bin/gpio.o
	$(CC) -c src/output_pin.c -o bin/output_pin.o $(CFLAGS)

bin/manager_input.o:
	$(CC) -c src/manager_input.c -o bin/manager_input.o $(CFLAGS)

bin/gpio.o:
	$(CC) -c src/gpio.c -o bin/gpio.o $(CFLAGS)

bin/serial.o: bin/arduino-serial.o
	$(CC) -c src/serial.c -o serial.o $(CFLAGS)

bin/arduino-serial.o:
		$(CC) -c src/arduino-serial.c -o arduino-serial.o $(CFLAGS)

bin:
	mkdir bin

clean:
	rm -rfv bin/

rb:clean build

rebuild: clean build
