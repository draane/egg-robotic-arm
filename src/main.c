#include <stdio.h>
#include <stdlib.h>
#include "manager_io.h"

int main() {
  int pid_manager = fork();
  if (pid_manager == -1) {
    printf("Error: Could not create manager\n" );
    return 1;
  } else if(pid_manager > 0) {
    manager_io();
  }else{
    pause(); //fare nulla
  }
  return 0;
}
