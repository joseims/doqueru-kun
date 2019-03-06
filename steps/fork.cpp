#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>

int main() {
    pid_t pid_parent = getpid();
    
    fork(); // one process will run this line

    pid_t pid = getpid();   // two processes will run this line

    if (pid == pid_parent) {    // will be truth for parent
        printf("parent process [PID = %d]\n", pid);
    } else {    // will be truth for child
        printf("child process [PID = %d]\n", pid);
        printf("\n\npress [CTRL + C] to exit\n");
    }

    while (1) {
        sleep(1);
    }

    return 0;
}