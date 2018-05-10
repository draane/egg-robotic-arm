#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

#define MAX_INFO_TO_SEND_SIZE 100

void communicate_output_with_manager(int pipe_output_write, int pipe_output_read){
    /*
         * Actions to perform:
         * 0) Wait for the "start" command from manager and send ack.
         * 1) Read all the information from manager and send ack.
         * 2) Read "finsh_output" command, send ack.
    */

    char msg_received[MAX_INFO_TO_SEND_SIZE];
    strcpy(msg_received, "\0");
    while (1){
        strcpy(msg_received, "\0");
        read(pipe_output_read, msg_received, MAX_INFO_TO_SEND_SIZE);

        if (strcmp(msg_received, "start\0") != 0){
            fprintf(stdout, "Output process didn't receive the start command as expected.\n");
            close(pipe_output_read);
            close(pipe_output_write);
            exit(1);
        }
        write(pipe_output_write, "ack\0", MAX_INFO_TO_SEND_SIZE);
        while(strcmp(msg_received, "finish_input\0") != 0){
            read(pipe_output_read, msg_received, MAX_INFO_TO_SEND_SIZE);
            fprintf(stdout, "received %s\n", msg_received);
            write(pipe_output_write, "ack\0", MAX_INFO_TO_SEND_SIZE);
        }
    }

}

void start_output(int pipe_write, int pipe_read){
    // Parameters passed are the ends of the pipe that the method will use.
    // Other ones have already been close by the manager process.

    // DO SOME STUFF HERE TO INITIALIZE ALL OUTPUT PROCESSES!

    communicate_output_with_manager(pipe_write, pipe_read);
}

