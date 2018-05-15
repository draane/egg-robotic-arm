build: bin bin/main.o
	gcc bin/*.o -o bin/main.out

bin/main.o: bin/manager_io.o src/main.c
	gcc -c src/main.c -o bin/main.o

bin/manager_io.o: bin/manager_input.o bin/manager_output.o src/manager_io.c
	gcc -c src/manager_io.c -o bin/manager_io.o

bin/manager_output.o: src/manager_output.c
	gcc -c src/manager_output.c -o bin/manager_output.o

bin/manager_input.o: src/manager_input.c
	gcc -c src/manager_input.c -o bin/manager_input.o

bin:
	mkdir bin

clean:
	rm -rfv bin/
rb:clean build
