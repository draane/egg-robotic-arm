#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>


#define MAX_INFO_TO_SEND_SIZE 100

void communicate_with_manager(int pipe_write, int pipe_read) {
    /*
     * Actions to perform:
     * 0) Wait for the start communication message from manager ("start").
     * 1) Collect information from sensors.
     * 2) Write in output the information.
     */
    char msg_received[100];

    // TODO: must be eliminated.
    srand(time(NULL));
    while (1) {
        // 0 Wait for the start communication message from manager ("start").
        read(pipe_read, msg_received, MAX_INFO_TO_SEND_SIZE);
        if (strcmp(msg_received, "start\0") == 0){
            // 1) Collect information from sensors.

            // 2) Write in output the information.
            char msg_to_send[MAX_INFO_TO_SEND_SIZE];
            strcpy(msg_to_send, "Hello, I'm the input process\0");
            int num_messages_to_send = (rand() % 4) + 1;
            int i;
            for (i = 0; i<num_messages_to_send; i++){
                write(pipe_write, msg_to_send, MAX_INFO_TO_SEND_SIZE);
                read(pipe_read, msg_received, MAX_INFO_TO_SEND_SIZE);
                if (strcmp(msg_received, "ack\0") != 0){
                    fprintf(stdout, "manager didn't receive correctly: %s\n", msg_received);
                    exit(1);
                }
            }
            write(pipe_write, "finish_input\0", MAX_INFO_TO_SEND_SIZE);
            read(pipe_read, msg_received, MAX_INFO_TO_SEND_SIZE);
            if (strcmp(msg_received, "ack\0") != 0){
                fprintf(stdout, "manager didn't receive correctly: %s\n", msg_received);
                exit(1);
            }

        }
        else {
            // Message received wasn't expected, kills itself.
            fprintf(stdout, "Didnt' expect such a message: %s\n", msg_received);
            exit(1);
        }
    }
}
void start_input(int pipe_write, int pipe_read){
    // Parameters passed are the ends of the pipe that the method will use.
    // Other ones have already been close by the manager process.

    // DO SOME STUFF HERE TO INITIALIZE ALL INPUT PROCESSES!

    communicate_with_manager(pipe_write, pipe_read);

}
