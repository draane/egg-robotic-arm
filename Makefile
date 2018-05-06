build: bin bin/main.o
	gcc bin/main.o bin/manager_io.o bin/manager_output.o -o bin/main.out

bin/main.o: bin/manager_io.o
	gcc -c src/main.c -o bin/main.o

bin/manager_io.o: bin/manager_input.o bin/manager_output.o
	gcc -c src/manager_io.c -o bin/manager_io.o

bin/manager_output.o:
	gcc -c src/manager_output.c -o bin/manager_output.o

bin/manager_input.o:
	gcc -c src/manager_input.c -o bin/manager_input.o

bin:
	mkdir bin

clean:
	rm -rfv bin/

rebuild: clean build
