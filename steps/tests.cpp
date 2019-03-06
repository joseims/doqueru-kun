#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>


int main(int argc, char** argv) {
    printf("[process PID = %d]\n", getpid());

    return 0;
}