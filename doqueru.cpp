#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>

using namespace std;


//ps fc pra visualizar
void fork_demostration() {

  pid_t pid;
  pid = fork();

  if (pid == 0) {
    printf("Hello !! ( child ) \n");
  } else {
    printf("Hello, World! ( parent ) \n");
  }

}

void updated_fork() {
  pid_t pid;
  if ((pid = fork()) == 0) {
    printf("Hello !! ( child ) \n");
  }
    printf("Hello, World! ( parent ) \n");
}

void exec_demo(char** argv) {
  execvp("./hello_worldo", argv+1);
}

void fork_and_exec(char** argv) {
  int* status;
  pid_t pid;
  if ((pid = fork()) == 0) {
    execvp("./hello_worldo", argv+1);
  }
  wait(status);
  printf("===============\n");
  printf("Now the father!\n");
  printf("===============\n");
  execvp("./hello_worldo", argv+1);
}


void fork_and_exec_env(char** argv) {
  int* status;
  pid_t pid;
  if ((pid = fork()) == 0) {
    clearenv();
    execvp("./hello_worldo", argv+1);
  }
  wait(status);
  printf("===============\n");
  printf("Now the father!\n");
  printf("===============\n");
  execvp("./hello_worldo", argv+1);
}


int main(int argc, char** argv) {
  //fork_demostration();
  //updated_fork();
  //exec_demo(argv);
  //fork_and_exec(argv);
  fork_and_exec_env(argv);
  return 0;
}

//unshare for fork