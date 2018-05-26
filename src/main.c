#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "manager_io.h"

static int get_command_line_arguments(int argv, char** argc);


int main(int argv, char** argc) {
    fprintf(stdout, "Start the main process \n");
    
    if (argv > 1){
        int return_message = get_command_line_arguments(argv, argc);
        if (return_message != 0){
            // Some error occurred;
            exit(return_message);
        }
    }

    int pid_manager = fork();
    if (pid_manager == -1) {
        printf("Error: Could not create manager\n" );
        exit (1);
    } else if(pid_manager > 0) {
        manager_io();
    }else{
        exit(0); //fare nulla
    }
    return 0;
}



static int get_command_line_arguments(int argv, char** argc){
    int pos_r = 1;
    while (pos_r < argv){
        char* arg = argc[pos_r];
        printf("arg1 %s\n", arg);
        if ((strcmp(arg, "-h") == 0) || (strcmp(arg, "--help") == 0)){
            printf("Usage: main.out [OPTION] [FILE]\n");
            printf("Welcome to the egg more or less robotic auto refiller program.\n");
            printf("-if, --input_file name_file Set 8 GPIO pin for the RaspberryPi as 8 space separated integers.(example 0 1 2 3 4 5 6 7)\n");
        }
        pos_r ++;
    }

    return 0;
}

