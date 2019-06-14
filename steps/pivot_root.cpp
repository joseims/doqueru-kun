#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/syscall.h>
#include <sys/mount.h>
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

#define pivot_root(new_root, put_old) syscall(SYS_pivot_root, new_root, put_old)

const char MOUNT_DIR[] = "../doquerinhos_shell";

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

void mount_proc() {
    ASSERTMSG(-1 != mkdir("/proc", 0755) || errno == EEXIST, "failed to create /proc");
    ASSERTMSG(-1 != mount("proc", "/proc", "proc", 0, NULL), "failed at mounting /proc");
}

void unsharenamespaces() {
    const int UNSHARE_FLAGS = CLONE_NEWUTS | CLONE_NEWPID | CLONE_NEWNS;
    const char* hostname = "doqueru";

    ASSERTMSG(-1 != unshare(UNSHARE_FLAGS), "unshare has failed");
    ASSERTMSG(-1 != sethostname(hostname, strlen(hostname)), "sethostname has failed");
    ASSERTMSG(-1 != mount("none", "/", NULL, MS_REC | MS_PRIVATE, NULL), "mount MS_PRIVATE on / has failed");
}

void pivot_root_routine() {
    const char OLD_ROOT[] = "./oldroot";

    ASSERTMSG(-1 != mount(MOUNT_DIR, MOUNT_DIR, NULL, MS_BIND | MS_PRIVATE | MS_REC, NULL), "mount MS_PRIVATE on MOUNT_DIR has failed");
    ASSERTMSG(-1 != chdir(MOUNT_DIR), "chdir to MOUNT_DIR has failed");
    ASSERTMSG(-1 != mkdir(OLD_ROOT, 0755) || errno == EEXIST, "mkdir OLD_ROOT has failed");
    ASSERTMSG(-1 != pivot_root(".", OLD_ROOT), "pivot_root has failed");
    ASSERTMSG(-1 != chdir("/"), "chdir to the new / has failed");
    ASSERTMSG(-1 != umount2(OLD_ROOT, MNT_DETACH), "umount OLD_ROOT has failed");
    ASSERTMSG(-1 != rmdir(OLD_ROOT), "error on removing OLD_ROOT");
}

void doqueru(char* exec_path, char** argv) {
    unsharenamespaces();
    pivot_root_routine();
    
    pid_t child_pid;
    ASSERTMSG(-1 != (child_pid = fork()), "fork PID 1 has failed");

    int status;

    if (child_pid == 0) {   // init process
        mount_proc();

        ASSERTMSG(-1 != (child_pid = fork()), "fork PID 2 has failed");

        if (child_pid == 0) ASSERTMSG(-1 != execvp(exec_path, argv), "exec has failed");
        
        while (wait(&status) != -1 || errno != ECHILD);

        fprintf(stderr, "init died\n");
    }

    wait(&status);
}

int main(int argc, char** argv) {
    doqueru(argv[1], &argv[1]);
    exit(0);
}
