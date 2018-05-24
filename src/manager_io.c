
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

// Global variables for parent process;
int child_input;
int child_output;

int fd_input_manager[2];
int fd_manager_input[2];
int fd_output_manager[2];
int fd_manager_output[2];

void manage_input_output(int, int, int, int, int, int);
void trigger_input(int);
char* read_input(int, int);
void trigger_output(int);
void write_output(int, int, int*);
static int* process_input(char *msg_received);
static int count_eggs_in_the_box(unsigned int);
static int count_eggs_in_the_warehouse(unsigned int);
void sigterm_handler(int);
static void shutdown();

void shutdown(){
    
    close(fd_input_manager[READ_PIPE]);
    close(fd_manager_input[WRITE_PIPE]);
    close(fd_manager_output[WRITE_PIPE]);
    close(fd_output_manager[READ_PIPE]);

    kill(child_input, SIGTERM);
    kill(child_output, SIGTERM);
    exit(1);
}

void sigterm_handler(int signal_rec){
    if (signal_rec == SIGINT){
        PRINT("Sigterm received by master, killing sons and himself!\n");
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
    PRINT("Eggs in the box: %d\n", eggs);
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
    PRINT("counter of eggs in the box: %d\n", counter);
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
       exit(1);
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
            PRINT("Received string as unsigned integer %d\n", res);
        }
        PRINT("Received string as unsigned integer %d\n", res);
        res_char = res;
        PRINT("Received in char %u\n", res_char);
        return res_char;
    }
}

static int* process_input(char* msg_received){
    /* This process needs to convert the data in input into output data.
        The data received consists of: 
        - bits 0-5: the values of the 6 sensors of the egg box (a = false | b = true)
        - bits 6-7: the values of the 2 bits of the warehouse. Same syntax as before.

        Data to send in output: 
        - bits 0-2: eggs in the box (configuration 111 is not used.)
        - bits 3-4: usually set to 0, when set they sign the number of eggs to move from the warehouse to the box.
        - bits 5-7: eggs to order (To refill the warehouse or to fill the box?).
    */

    PRINT("Size of input string: %lu\n", strlen(msg_received));
    if (strlen(msg_received) != NUMBER_OF_OUTPUT_BYTE){
        PRINT("Some error occurred: the input process printed a wrong number of pins status(1 correct, %lu received).\n", strlen(msg_received));
        shutdown();
    }

    unsigned char byte_received = msg_received[0];

    PRINT("Manager received : %d\n", byte_received);
    // Count eggs in the box (0-5 bits)
    int eggs_in_the_box = count_eggs_in_the_box(byte_received);
    // Count eggs in the warehouse (6-7 bits)
    int eggs_int_the_warehouse = count_eggs_in_the_warehouse(byte_received);

    int eggs_to_move_to_box = 6 - eggs_in_the_box;
    int eggs_to_order;
    if (eggs_int_the_warehouse == 3){
        eggs_to_order = 0;
    }
    else {
        eggs_to_order = eggs_to_move_to_box - eggs_int_the_warehouse;
        if (eggs_to_order < 0){
            eggs_to_order = 0;
        }
    }

    int* output_msg = malloc(sizeof(int) * 3);
    output_msg[0] = eggs_in_the_box;
    output_msg[1] = eggs_int_the_warehouse;
    output_msg[2] = eggs_to_order;
    
    //unsigned char output = generate_output(eggs_in_the_box, eggs_int_the_warehouse);

    PRINT("Generated output = %d, %d, %d\n", output_msg[0], output_msg[1], output_msg[2]);
    return output_msg;
}


void trigger_input(int pipe_input_write){
    /*
     * 0) send "start" command to input.
     */
    write(pipe_input_write, START_MSG, MAX_INFO_TO_SEND_SIZE);
}

char* read_input(int pipe_input_read, int pipe_input_write){
    // input just sends one string. No acks made.

    char * msg_received = malloc(sizeof(char) * 2);
    strcpy(msg_received, "\0");
    read(pipe_input_read, msg_received, DIM_OF_MSG_PIPE);
    PRINT("received %s\n", msg_received);

    return msg_received;
     
}

void trigger_output(int pipe_output_write){
    /*
     * 0) send "start" command to output.
     */
    PRINT("Message to send in output.\n");
    write(pipe_output_write, START_MSG, DIM_OF_MSG_PIPE);
    //PRINT("Message written \n");
}

void write_output(int pipe_output_read, int pipe_output_write, int* msg_output){
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
    if (strcmp(msg_received, "ack") == 0){
        PRINT("ACK received after sending start\n");
        // 1) Write in output the information.
        char eggs_in_the_box[5];
        sprintf(eggs_in_the_box, "%d", msg_output[0]);
        char eggs_in_the_warehouse[5];
        sprintf(eggs_in_the_warehouse, "%d", msg_output[1]);
        char eggs_to_order[5];
        sprintf(eggs_to_order, "%d", msg_output[2]);

        PRINT("Manager produced these strings: %s, %s, %s", eggs_in_the_box, eggs_in_the_warehouse, eggs_to_order);
        
        write(pipe_output_write, eggs_in_the_box, MAX_INFO_TO_SEND_SIZE);
        write(pipe_output_write, eggs_in_the_warehouse, MAX_INFO_TO_SEND_SIZE);
        write(pipe_output_write, eggs_to_order, MAX_INFO_TO_SEND_SIZE);
        
        read (pipe_output_read, msg_received, MAX_INFO_TO_SEND_SIZE);
        
        // 2) Wait for ack.
        if (strcmp(msg_received, "ack\0") != 0){
            PRINT("manager didn't receive correctly: %s\n", msg_received);
            shutdown();
        }
        else {
            //PRINT("switch to input.\n");
        }
        
    }
    else {
        // Message received wasn't expected, kills itself.
        PRINT("Didnt' expect such a message: %s\n", msg_received);
        shutdown();
    }
    free (msg_output);
}

void wait_for_output_to_finish(int pipe_output_read, int pipe_output_write){
    char msg_received[MAX_INFO_TO_SEND_SIZE];
    read (pipe_output_read, msg_received, MAX_INFO_TO_SEND_SIZE);
    if (strcmp(msg_received, "finish_output\0") == 0){
        write(pipe_output_write, "ack\0", MAX_INFO_TO_SEND_SIZE);
    }
    else {
        PRINT("Output crashed\n");
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
    signal(SIGINT, &sigterm_handler);
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
        char* msg_received = read_input(pipe_input_read, pipe_input_write);

        /* 3) Create output information based on the input received messages.
         *      Still to decided how
         *      TODO: create output information.
         */
        int* output = process_input(msg_received);
        free(msg_received);
        /* 4) Trigger the output_manager process and send information.
         *      Perform all the operations like before, but now the manager is writing.
         */

        trigger_output(pipe_output_write);
        write_output(pipe_output_read, pipe_output_write, output);

        /* 5) Wait for the end of the output_process (and mainly for the robotic arm).
        */
        wait_for_output_to_finish(pipe_output_read, pipe_output_write);


    }
}


void manager_io(void){
    // This process creates the two Input and Output manager processes,
    // Then manages all the interactions among them.

    // Prepare the 4 pipes for the two bidirectional communications.
    pipe(fd_input_manager);
    pipe(fd_manager_input);
    pipe(fd_output_manager);
    pipe(fd_manager_output);

    int pid_input = fork();

    if (pid_input < 0){
        PRINT("Couldn't create the Input process. \n");
        exit(1);
    }
    else if (pid_input == 0){
        // Child process
        // Closes the ends of the pipes it doesn't need.
        close(fd_input_manager[READ_PIPE]);
        close(fd_manager_input[WRITE_PIPE]);
        PRINT("Initialize the input process.\n");
        // Starts the input process.
        start_input(fd_input_manager[WRITE_PIPE], fd_manager_input[READ_PIPE]);
    }
    else {
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
            start_output(fd_output_manager[WRITE_PIPE], fd_manager_output[READ_PIPE]);
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
