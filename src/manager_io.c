#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "manager_output.h"
#include "manager_input.h"

void manage_input_output(int pid_input, int pid_output){

}


void manager_io(void){
    // This process creates the two Input and Output manager processes,
    // Then manages all the interactions among them.

    int pid_input = fork();

    if (pid_input < 0){
        fprintf(stdout, "Couldn't create the Input process. \n");
        exit(1);
    }
    else if (pid_input == 0){
        // Child process, can invoke start_input method.
        fprintf(stdout, "Initialize the input process.\n");
        start_input();
    }
    else {
        // Parent process, forks again to create the output process.
        int pid_output = fork();
        if (pid_output < 0){
            fprintf(stdout, "Couldn't create the Output process.\n");
            exit(1);
        }
        if (pid_output == 0){
            // Child process, can invoke start_output method.
            fprintf(stdout, "Initialize the output process\n");
            start_output();
        }
        else {
            // Parent process again, can start communication among output and input process.
            manage_input_output(pid_input, pid_output);
        }

    }
}


