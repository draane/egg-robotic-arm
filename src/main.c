#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "manager_io.h"

int main() {
    fprintf(stdout, "Start the main process \n");
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
