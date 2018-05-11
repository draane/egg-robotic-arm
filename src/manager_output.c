#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "utils.h"

#define MAX_INFO_TO_SEND_SIZE 100
#define num_parameters_received 3

void communicate_output_with_manager(int pipe_output_write, int pipe_output_read){
    /*
         * Actions to perform:
         * 0) Wait for the "start" command from manager and send ack.
         * 1) Read all the information from manager and send ack.
         * 2) Read "finsh_output" command, send ack.
    */

    char msg_received[MAX_INFO_TO_SEND_SIZE];
    strcpy(msg_received, "\0");
    int egg_in_the_case, egg_to_move, egg_to_order;
    while (1){
        PRINT("Output is reading... \n");
        strcpy(msg_received, "\0");
        read(pipe_output_read, msg_received, MAX_INFO_TO_SEND_SIZE);

        if (strcmp(msg_received, "start\0") != 0){
            fprintf(stdout, "Output process didn't receive the start command as expected.\n");
            close(pipe_output_read);
            close(pipe_output_write);
            exit(1);
        }
        else {
            PRINT("output starts to receive parameters\n");
            write(pipe_output_write, "ack\0", MAX_INFO_TO_SEND_SIZE);
        }
        

        int parameters[num_parameters_received];

        for (int i = 0; i< num_parameters_received; i++){
            read(pipe_output_read, msg_received, MAX_INFO_TO_SEND_SIZE);
            parameters[i] = atoi(msg_received);
            fprintf(stdout, "received %s\n", msg_received);
            write(pipe_output_write, "ack\0", MAX_INFO_TO_SEND_SIZE);
        }

        read(pipe_output_read, msg_received, MAX_INFO_TO_SEND_SIZE);
        if (strcmp(msg_received, "finish_input\0") == 0){
            // Everything is ok.
            fprintf(stdout, "received %s\n", msg_received);
            write(pipe_output_write, "ack\0", MAX_INFO_TO_SEND_SIZE);

            // DO COSTA'S STUFF
        }

        else {
            close(pipe_output_write);
            close(pipe_output_read);
            exit(0);
        }
        
        
    }

}

void start_output(int pipe_write, int pipe_read){
    // Parameters passed are the ends of the pipe that the method will use.
    // Other ones have already been close by the manager process.

    // DO SOME STUFF HERE TO INITIALIZE ALL OUTPUT PROCESSES!

    communicate_output_with_manager(pipe_write, pipe_read);
}

