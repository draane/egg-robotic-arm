#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

#include "manager_output.h"
#include "manager_input.h"
#include "utils.h"

#define READ_PIPE 0
#define WRITE_PIPE 1
#define MAX_INFO_TO_SEND_SIZE 100

void trigger_input(int pipe_input_write){
    /*
     * 0) send "start" command to input.
     */
    fprintf(stdout, "Message to send in input.\n");
    write(pipe_input_write, "start\0", MAX_INFO_TO_SEND_SIZE);
}

void read_input(int pipe_input_read, int pipe_input_write){
    char msg_received[MAX_INFO_TO_SEND_SIZE];
    strcpy(msg_received, "\0");
    while(strcmp(msg_received, "finish_input\0") != 0){
        read(pipe_input_read, msg_received, MAX_INFO_TO_SEND_SIZE);
        fprintf(stdout, "received %s\n", msg_received);
        write(pipe_input_write, "ack\0", MAX_INFO_TO_SEND_SIZE);
    }
}

void trigger_output(int pipe_output_write){
    /*
     * 0) send "start" command to output.
     */
    fprintf(stdout, "Message to send in output.\n");
    write(pipe_output_write, "start\0", MAX_INFO_TO_SEND_SIZE);
    //PRINT("Message written \n");
}

void write_output(int pipe_output_read, int pipe_output_write){
    // TODO: needs to take as parameter the information to pass to the output process.
    /*
     * Actions to perform:
     * 0) Wait for the output "akc".
     * 1) Write in output the information.
     * 2) Write in output the "finish_output" command and read ack.
     */
    char msg_received[100];
    //PRINT("start message sent\n");

    // 0) Wait for the output "akc".
    read(pipe_output_read, msg_received, MAX_INFO_TO_SEND_SIZE);
    if (strcmp(msg_received, "ack\0") == 0){
        PRINT("ACK received after sending start\n");
        // 1) Write in output the information.
        char msg_to_send[MAX_INFO_TO_SEND_SIZE];
        strcpy(msg_to_send, "Hello, I'm the manager_process\0");
        int num_messages_to_send = 3;
        int messages_to_send[3];
        messages_to_send[0] = rand() % 7;
        messages_to_send[1] = rand() % 4;
        messages_to_send[2] = rand() % 7;
        for (int i = 0; i<num_messages_to_send; i++){
            sprintf(msg_to_send, "%d", messages_to_send[i]);
            write(pipe_output_write, msg_to_send, MAX_INFO_TO_SEND_SIZE);
            read(pipe_output_read, msg_received, MAX_INFO_TO_SEND_SIZE);
            if (strcmp(msg_received, "ack\0") != 0){
                fprintf(stdout, "manager didn't receive correctly: %s\n", msg_received);
                exit(1);
            }
        }
        // 2) Write in output the "finish_output" command and read ack.
        write(pipe_output_write, "finish_input\0", MAX_INFO_TO_SEND_SIZE);
        read(pipe_output_read, msg_received, MAX_INFO_TO_SEND_SIZE);
        if (strcmp(msg_received, "ack\0") != 0){
            fprintf(stdout, "manager didn't receive correctly: %s\n", msg_received);
            exit(1);
        }
        else {
            //PRINT("switch to input.\n");
        }

    }
    else {
        // Message received wasn't expected, kills itself.
        fprintf(stdout, "Didnt' expect such a message: %s\n", msg_received);
        exit(1);
    }
}

void wait_for_output_to_finish(int pipe_output_read, int pipe_output_write){
    char msg_received[MAX_INFO_TO_SEND_SIZE];
    read (pipe_output_read, msg_received, MAX_INFO_TO_SEND_SIZE);
    if (strcmp(msg_received, "finish_output\0")){
        write(pipe_output_write, "ack\n", MAX_INFO_TO_SEND_SIZE);
    }
    else {
        fprintf(stdout, "Output crashed\n");
        close(pipe_output_read);
        close(pipe_output_write);
        exit(1);
    }
}


void manage_input_output(int pid_input, int pid_output, int pipe_input_read, int pipe_input_write, int pipe_output_read, int pipe_output_write) {
    /*
     * Order of actions performed by manager_input_output:
     * 0) Wait one second.
     * 1) Trigger the input pipe.
     * 2) Fetch information by the manager_input process.
     * 3) Create output information based on the input received messages.
     * 4) Trigger the output_manager process and send information.
     * 5) Wait for the end of the output_process (and mainly for the robotic arm).
     */

    //TODO: remove it!
    srand(time(NULL));
    while (1){
        // 0) Wait one second.
        sleep(1);

        /* 1) Trigger the input pipe.
         *      Send "start" to fetch input information, other to kill the process.
         */
        trigger_input(pipe_input_write);

        /* 2) Fetch information by the manager_input process.
         *      Perform a series of read operation, acknowledged by "ack" messages.
         */
        read_input(pipe_input_read, pipe_input_write);

        /* 3) Create output information based on the input received messages.
         *      Still to decided how
         *      TODO: create output information.
         */

        /* 4) Trigger the output_manager process and send information.
         *      Perform all the operations like before, but now the manager is writing.
         */

        trigger_output(pipe_output_write);
        write_output(pipe_output_read, pipe_output_write);

        /* 5) Wait for the end of the output_process (and mainly for the robotic arm).
        */
        wait_for_output_to_finish(pipe_output_read, pipe_output_write);


    }
}


void manager_io(void){
    // This process creates the two Input and Output manager processes,
    // Then manages all the interactions among them.

    // Prepare the 4 pipes for the two bidirectional communications.
    int fd_input_manager[2];
    int fd_manager_input[2];
    int fd_output_manager[2];
    int fd_manager_output[2];

    pipe(fd_input_manager);
    pipe(fd_manager_input);
    pipe(fd_output_manager);
    pipe(fd_manager_output);

    int pid_input = fork();

    if (pid_input < 0){
        fprintf(stdout, "Couldn't create the Input process. \n");
        exit(1);
    }
    else if (pid_input == 0){
        // Child process
        // Closes the ends of the pipes it doesn't need.
        close(fd_input_manager[READ_PIPE]);
        close(fd_manager_input[WRITE_PIPE]);
        fprintf(stdout, "Initialize the input process.\n");
        // Starts the input process.
        start_input(fd_input_manager[WRITE_PIPE], fd_manager_input[READ_PIPE]);
    }
    else {
        // Parent process:
        // Closes the ends of the pipes it doesn't need.
        close(fd_input_manager[WRITE_PIPE]);
        close(fd_manager_input[READ_PIPE]);

        // Forks again to create the output process manager.
        int pid_output = fork();
        if (pid_output < 0){
            // Error occurred.
            fprintf(stdout, "Couldn't create the Output process.\n");
            exit(1);
        }
        if (pid_output == 0){
            // Child process
            // Closes the ends of the pipes it doesn't need.
            close(fd_manager_output[WRITE_PIPE]);
            close(fd_output_manager[READ_PIPE]);

            fprintf(stdout, "Initialize the output process\n");
            // Invokes the output process manager.
            start_output(fd_output_manager[WRITE_PIPE], fd_manager_output[READ_PIPE]);
        }
        else {
            // Parent process again
            // Closes the ends of the pipes it doesn't need.
            close(fd_manager_output[READ_PIPE]);
            close(fd_output_manager[WRITE_PIPE]);
            // starts the communications among input and output.
            manage_input_output(pid_input, pid_output, fd_input_manager[READ_PIPE],
                                fd_manager_input[WRITE_PIPE], fd_output_manager[READ_PIPE],
                                fd_manager_output[WRITE_PIPE]);
        }

    }
}


