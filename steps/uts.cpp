#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <cstring>
#include <sched.h>

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif // _GNU_SOURCE

#define STRINGIZE(x) #x

/**
 * A shortcut to call _assert with the correct line number.
 */ 
#define ASSERTMSG(condition, msg) _assert(condition, msg, __LINE__)

/** 
 * Same as ASSERTMSG, but will use the condition as string to make the error message.
 */
#define ASSERT(condition) ASSERTMSG(condition, "assert " + STRINGIZE(condition) + " has failed")

/** 
 * Show error message and exit if condition is false.
 * If is_syscall is set to true, will show errno current value.
 */
bool _assert(bool condition, const char msg_onerror[], int line, bool is_syscall=true) {
    if (!condition) {
        fprintf(stderr, "[%s] on line: %d\n", msg_onerror, line);
        
        if (is_syscall)
            fprintf(stderr, "errno=%d [%s]\n", errno, std::strerror(errno));
        
        exit(1);
    }
    return true;
}

void unsharenamespaces() {
    const int UNSHARE_FLAGS = CLONE_NEWUTS;
    const char* hostname = "doqueru";

    ASSERTMSG(-1 != unshare(UNSHARE_FLAGS), "unshare has failed");
    ASSERTMSG(-1 != sethostname(hostname, strlen(hostname)), "sethostname has failed");
}

void doqueru(char* exec_path, char** argv) {
    pid_t child_pid = fork();
    ASSERTMSG(-1 != child_pid, "fork has failed");

    if (child_pid == 0) {
        unsharenamespaces();
        
        ASSERTMSG(-1 != execvp(exec_path, argv), "exec has failed");
    }
}

int main(int argc, char** argv) {
    doqueru(argv[1], &argv[1]);

    int status;
    while (wait(&status) != -1 || errno != ECHILD);

    exit(0);
}
