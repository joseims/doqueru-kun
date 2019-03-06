#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>

int main(int argc, char** argv) {

    if (fork() == 0) {
        printf("\n\n[CHILD RUNNING]:\n");
        execvp(argv[1], &argv[1]);
    }

    // busy waiting child(en) to end
    int status;
    while (wait(&status) != -1 || errno != ECHILD);

    printf("\n\n[PARENT RUNNING]:\n");
    execvp(argv[1], &argv[1]);

    return 0;
}