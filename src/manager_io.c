
#include"manager_input.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

#include <signal.h>

#include "utils.h"
#include "manager_output.h"
#include "manager_input.h"
#include "utils.h"

#define OUTPUT_FINISHES_MESSAGE "0\0"

#define LEN_OF_MESSAGE_TO_OUTPUT 2

// Global variables for parent process;
int child_input;
int child_output;

int fd_input_manager[2];
int fd_manager_input[2];
int fd_output_manager[2];
int fd_manager_output[2];

static void manage_input_output(int, int, int, int, int, int);
static void trigger_input(int);
static char* read_input(int, int);
static void write_output(int, int, int*);
static int* process_input(char *msg_received);
static int count_eggs_in_the_box(unsigned int);
static int count_eggs_in_the_warehouse(unsigned int);
static void sigterm_handler(int);
static void shutdown();

static void shutdown(){

    close(fd_input_manager[READ_PIPE]);
    close(fd_manager_input[WRITE_PIPE]);
    close(fd_manager_output[WRITE_PIPE]);
    close(fd_output_manager[READ_PIPE]);

    kill(child_input, SIGTERM);
    kill(child_output, SIGTERM);
    exit(1);
}

static void sigterm_handler(int signal_rec){
    if (signal_rec == SIGINT){
        PRINT("Sigint received by master, killing sons and himself!\n");
        shutdown();
    }
    if (signal_rec == SIGTERM){
        PRINT("Sigterm received by master, killing sons and himself!\n");
        shutdown();
    }
}

static int count_eggs_in_the_box(unsigned int byte_received){
    // mask with last three bits. 11111100 = 252;
    unsigned int mask= 252;
    unsigned int eggs = byte_received & mask;
    eggs = eggs >> 2;
    //PRINT("Eggs in the box: %d\n", eggs);
    int counter = 0;
    int i;
    for (i = 0; i<NUMBER_OF_EGGS_IN_THE_BOX; i++){
        unsigned char mask = 1;
        unsigned char temp = eggs & mask;
        if (temp == 1){
            counter ++;
        }
        eggs = eggs >> 1;
    }
    PRINT("Eggs in the box: %d\n", counter);
    return counter;
}

static int count_eggs_in_the_warehouse(unsigned int byte_received){
    // read last 2 bits. Mask is 3.
    unsigned char eggs_in_the_warehouse = byte_received & 3;
    int res = eggs_in_the_warehouse;
    PRINT("Eggs in the warehouse: %d\n", res);
    return res;
}


unsigned char make_one_byte_from_string(char* str){
    unsigned char res_char;
    if (strlen(str) != 8){
       PRINT("Some error occurred: the input process printed a wrong number of pins status(8 correct, %lu received).\n", strlen(str));
       shutdown();
    }
    else {
        unsigned int res = 0;
        unsigned int one = 1;
        int i;
        for (i = 0; i < 8; i++){
            res = res << 1;
            if (str[i] == 'a'){
                // leave 0
            }
            else if (str[i] == 'b'){
                res = res | one;
            }
            else {
                PRINT("Some error occurred: received %c\n", str[i]);
                shutdown();
            }
        }
        res_char = res;
        return res_char;
    }
    return '0';
}

static int* process_input(char* msg_received){
    /* This process needs to convert the data in input into output data.
        The data received consists of:
        - bits 0-5: the values of the 6 sensors of the egg box (a = false | b = true)
        - bits 6-7: the values of the 2 bits of the warehouse. Same syntax as before.

        Data to send in output:
        - Eggs in the box;
        - Eggs in the warehouse
        - Eggs to order
        - Eggs to move with the robotic arm?
    */
   
    if (strlen(msg_received) != NUMBER_OF_OUTPUT_BYTE){
        PRINT("Some error occurred: the input process printed a wrong number of pins status(1 correct, %lu received).\n", strlen(msg_received));
        shutdown();
    }

    unsigned char byte_received = msg_received[0];

    PRINT("Manager received : %d\n", byte_received);
    int eggs_in_the_box = count_eggs_in_the_box(byte_received);
    int eggs_in_the_warehouse = count_eggs_in_the_warehouse(byte_received);

    int eggs_to_move_to_box = 6 - eggs_in_the_box;
    eggs_to_move_to_box = eggs_to_move_to_box > eggs_in_the_warehouse ? 
                        eggs_in_the_warehouse : 
                        eggs_to_move_to_box;
    int eggs_to_order;
    if (eggs_in_the_warehouse == 3){
        eggs_to_order = 0;
    }
    else {
        eggs_to_order = 6 - eggs_in_the_box - eggs_to_move_to_box;
        if (eggs_to_order < 0){
            eggs_to_order = 0;
        }
    }

    int* output_msg = malloc(sizeof(int) * 3);
    output_msg[0] = eggs_in_the_box;
    output_msg[1] = eggs_to_move_to_box;
    output_msg[2] = eggs_to_order;

    PRINT("Generated output = %d, %d, %d\n", output_msg[0], output_msg[1], output_msg[2]);
    return output_msg;
}


void trigger_input(int pipe_input_write){
    //send START command to input.
    write(pipe_input_write, START_MSG, MAX_INFO_TO_SEND_SIZE);
}


char* read_input(int pipe_input_read, int pipe_input_write){
    // input just sends one string. No acks made.
    char * msg_received = malloc(sizeof(char) * 2);
    strcpy(msg_received, "\0");
    read(pipe_input_read, msg_received, DIM_OF_MSG_PIPE);
    return msg_received;

}


void write_output(int pipe_output_read, int pipe_output_write, int* msg_output){
    // TODO: needs to take as parameter the information to pass to the output process.
    /*
     * Actions to perform:
     * 1) Write in output the information. (The integers are kept in the msg output list.)
     * 2) wait for the output to finish. (function "wait_for_output_to_finish").
     */

    //int len; // length of the integer converted to string.
    char eggs_in_the_box[LEN_OF_MESSAGE_TO_OUTPUT];
    sprintf(eggs_in_the_box, "%d", msg_output[0]);
    char eggs_in_the_warehouse[LEN_OF_MESSAGE_TO_OUTPUT];
    sprintf(eggs_in_the_warehouse, "%d", msg_output[1]);
    char eggs_to_order[LEN_OF_MESSAGE_TO_OUTPUT];
    sprintf(eggs_to_order, "%d", msg_output[2]);

    PRINT("Manager produced these strings: %s, %s, %s\n", eggs_in_the_box, eggs_in_the_warehouse, eggs_to_order);

    write(pipe_output_write, eggs_in_the_box, LEN_OF_MESSAGE_TO_OUTPUT);
    write(pipe_output_write, eggs_in_the_warehouse, LEN_OF_MESSAGE_TO_OUTPUT);
    write(pipe_output_write, eggs_to_order, LEN_OF_MESSAGE_TO_OUTPUT);
    // TODO: to fix the egg to move with the arm.
    write(pipe_output_write, eggs_to_order, LEN_OF_MESSAGE_TO_OUTPUT);

}

void wait_for_output_to_finish(int pipe_output_read, int pipe_output_write){
    char msg_received[MAX_INFO_TO_SEND_SIZE];
    read (pipe_output_read, msg_received, MAX_INFO_TO_SEND_SIZE);

    // Wait for finish_message from output to come to be able to resume the process back from beginning.
    if (strcmp(msg_received, OUTPUT_FINISHES_MESSAGE) != 0){
        PRINT("manager didn't receive correctly: %s\n", msg_received);
        shutdown();
    }
    else {
        // Switch back to input.
    }
}


void manage_input_output(int pid_input, int pid_output, int pipe_input_read, int pipe_input_write, int pipe_output_read, int pipe_output_write) {
    // Subscribe the parent process to the handler for the correct termination.
    signal(SIGINT, &sigterm_handler);
    signal(SIGTERM, &sigterm_handler);
    while (1){
        sleep(1);
        
        trigger_input(pipe_input_write);

        /* 1) Fetch information (1 byte of data) by the manager_input process. */
        char* msg_received = read_input(pipe_input_read, pipe_input_write);

        /* 2) Create output information based on the input received messages. */
        int* output = process_input(msg_received);
        

        write_output(pipe_output_read, pipe_output_write, output);
        free(msg_received); // Memory for message received is allocated dinamically.
        free(output);  // memory for output list of integers is allocated dinamically.

        /* 3) Wait for the end of the output_process (mainly for the robotic arm :'( ).
        */
        wait_for_output_to_finish(pipe_output_read, pipe_output_write);
    }
}


void manager_io(int* input_pins_from_file, int* output_pins_from_file){
    // This process creates the two Input and Output manager processes,
    // Then manages all the interactions among them.
    // It receives the two lists of input and output pins specified in the files.
    // all values are -1 if no file is specified.
    // Prepare the 4 pipes for the two bidirectional communications.
    pipe(fd_input_manager);
    pipe(fd_manager_input);
    pipe(fd_output_manager);
    pipe(fd_manager_output);

    int pid_input = fork();

    if (pid_input < 0){
        PRINT("Couldn't create the Input process. Closes all processes. \n");
        shutdown();
    }
    else if (pid_input == 0){
        // Child process
        // Closes the ends of the pipes it doesn't need.
        close(fd_input_manager[READ_PIPE]);
        close(fd_manager_input[WRITE_PIPE]);
        PRINT("Initialize the input process.\n");
        // Starts the input process: the two pipe ends for reading and writing are passed, along with the 
        // pins specified in the pin file (if no -if option is specified, then -1 is passed).
        start_input(fd_input_manager[WRITE_PIPE], fd_manager_input[READ_PIPE], input_pins_from_file);
    }
    else {
        sleep(2);
        child_input = pid_input;
        // Parent process:
        // Closes the ends of the pipes it doesn't need.
        close(fd_input_manager[WRITE_PIPE]);
        close(fd_manager_input[READ_PIPE]);

        // Forks again to create the output process manager.
        int pid_output = fork();
        if (pid_output < 0){
            // Error occurred.
            PRINT("Couldn't create the Output process.\n");
            shutdown();
        }
        if (pid_output == 0){
            // Child process
            // Closes the ends of the pipes it doesn't need.
            close(fd_manager_output[WRITE_PIPE]);
            close(fd_output_manager[READ_PIPE]);

            PRINT("Initialize the output process\n");
            // Invokes the output process manager.
            
            int i;
            
            for (i = 0; i<NUM_PINS; i++){
                PRINT("file pin %d: %d\n", i, output_pins_from_file[i]);
            }
            // the pins specified in the file are passed, among with the two pipe ends for reading and writing.
            start_output(fd_output_manager[WRITE_PIPE], fd_manager_output[READ_PIPE], output_pins_from_file);
        }
        else {
            child_output = pid_output;
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
