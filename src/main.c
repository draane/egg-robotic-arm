#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include "manager_io.h"
#include "utils.h"

static int get_command_line_arguments(int argv, char** argc, FILE**, FILE**);

void list_of_pins_from_file(FILE*, int*);


int main(int argv, char** argc) {
    fprintf(stdout, "Start the main process \n");
    
    FILE* input_file = NULL, *output_file=NULL;
    if (argv > 1){
        int return_message = get_command_line_arguments(argv, argc, &input_file, &output_file);
        if (return_message != 0){
            // Some error occurred;
            exit(return_message);
        }

    }
    // If options -if, -of are specified, then update the list of pins with the new values.
    printf("file descriptors for input and output: \"%p\"\t\"%p\"\n", input_file, output_file);
    int input_pins_from_file[NUM_PINS];
    int output_pins_from_file[NUM_PINS];

    // Pins from file are -1 if not defined option -if, -of, or the integers in the file otherwise.

    // Update input list 
    list_of_pins_from_file(input_file, input_pins_from_file);
    // Update output list.
    list_of_pins_from_file(output_file, output_pins_from_file);
    
    int pid_manager = fork();
    if (pid_manager == -1) {
        printf("Error: Could not create manager\n" );
        exit (1);
    } else if(pid_manager > 0) {
        // to manager io the two lists of pins are passed.
        manager_io(input_pins_from_file, output_pins_from_file);
    }else{
        exit(0); //fare nulla
    }
    return 0;
}


void list_of_pins_from_file(FILE* fd, int* list_pin){
    if (fd != NULL){
        int i;
        for (i = 0; i<NUM_PINS; i++){
            fscanf(fd, "%d", &list_pin[i]);
        }
    }
    else {
        int i;
        for (i = 0; i<NUM_PINS; i++){
            list_pin[i] = -1;
        }
    }
}

static int get_command_line_arguments(int argv, char** argc, FILE** input_file, FILE** output_file){
    int pos_r = 1;
    while (pos_r < argv){
        char* arg = argc[pos_r];
        printf("arg1 %s\n", arg);
        if ((strcmp(arg, "-h") == 0) || (strcmp(arg, "--help") == 0)){
            printf("Usage: main.out [OPTION] [FILE]\n");
            printf("Welcome to the egg more or less robotic auto refiller program.\n");
            printf("\t-if, --input_file name_file: \tSet 8 GPIO pin for the RaspberryPi as\n\t\t\t\t\t 8 space separated integers.(example 0 1 2 3 4 5 6 7)\n");
            printf("\t-of, --output_file name_file: \tSet 8 GPIO pin for the RaspberryPi as\n\t\t\t\t\t 8 space separated integers.\n");

        }
        else if ((strcmp(arg, "-if") == 0) || (strcmp(arg, "--input_file") == 0)){
            
            printf("provided input file argument.\n");
            if (argv - pos_r == 1){
                fprintf(stderr, "Error: you need to provide an additional argument, the name of the settings file.\n");
                exit(1);
            }
            printf("arg2 %s\n", argc[pos_r + 1] );
            *input_file = fopen(argc[pos_r + 1], "r");
            printf("input file: %p\n", *input_file);
            if (*input_file == NULL){
                fprintf(stderr, "File specified \"%s\" doesn't exist.\n", argc[pos_r + 1]);
                exit(1);
            }
            pos_r ++;
        }

        else if ((strcmp(arg, "-of") == 0) || (strcmp(arg, "--output_file") == 0)){
            printf("Provided output file argument.\n");
            if (argv - pos_r == 1){
                fprintf(stderr, "Error: you need to provide an additional argument, the name of the settings file.\n");
                exit(1);
            }
            printf("arg2 %s\n", argc[pos_r + 1] );
            *output_file = fopen(argc[pos_r+1], "r");
            if (output_file == NULL){
                fprintf(stderr, "File specified \"%s\" doesn't exist.\n", argc[pos_r + 1]);
                exit(1);
            }
            pos_r ++;
        }

        pos_r ++;
    }

    return 0;
}

