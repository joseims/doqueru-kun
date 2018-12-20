#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/syscall.h>
#include <sys/mount.h>
#include <stdlib.h>
#include <errno.h>
#include <sched.h>
#include <string>
#include <string.h>

using namespace std;

char MOUNT_DIR[] = "./doquerinhos_shell";	//checar pronuncia


#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#define STRINGIZE(x) #x
#define pivot_root(new_root, put_old) syscall(SYS_pivot_root, new_root, put_old)
#define ASSERT(CONDITION) assert(CONDITION, STRINGIZE(CONDITION), __LINE__)

void assert(bool condition, char msg_onerror[], int line) {
    if (!condition) {
        printf("assertion failed (%s)) on line: %d\n", msg_onerror, line);
        exit(1);
    }
}

void error(char error_case[]) {
    printf("Error at %s (%s)\n", error_case, strerror(errno));
    exit(1);
}

void safe_unshare() {
  int flags = CLONE_NEWUTS | CLONE_NEWPID | CLONE_NEWNS;
	if (unshare(flags)) {
      error("unshare");
	}
}

void safe_sethostname() {
	char new_name[] = "Doquerinho";
	if (sethostname(new_name, strlen(new_name))) {
    error("set hostname");
	}
}


void setup_new_env() {
  clearenv();
  setenv("TERM", "xterm-256color", 0);
  setenv("PATH", "/bin/:/sbin/:usr/bin:/usr/sbin", 0);	//remember to explain paths
}

void ls() {
  int status;
  if (fork() == 0) {
    char* args_ls[] = {"/bin/ls", "-la", NULL};
    execv(args_ls[0], args_ls);
    printf("%s %s exited with errorno=%d\n", args_ls[0], args_ls[1], errno);
  }
  wait(&status);
}

void pwd() {
  char cwd[1024];
  getcwd(cwd, sizeof cwd);
  printf("pwd: %s\n", cwd);
}


int copy(char source[], char dest[]) {
  char c[1024];
  strcpy(c,"");
  strcat(c,"cp -r ");
  strcat(c,source);
  strcat(c,"/* ");
  strcat(c," ");
  strcat(c,dest);
  printf("%s\n",c );
  return system(c);
}


void mount_proc() {
  if(mount("proc","/proc","proc",NULL,NULL)) {
    error("proc mount");
  }
}

void pivot_root_routine() {
      if (mount(MOUNT_DIR, MOUNT_DIR, NULL, MS_BIND | MS_PRIVATE | MS_REC, NULL)) {
        error("new root mount");
      }

      chdir(MOUNT_DIR);

      /* Make a place for the old (intermediate) root filesystem to land. */
      if (mkdir("oldroot", 0755) && errno != EEXIST) {
        error("create old root");
      };

      if(pivot_root(".", "./oldroot")) {
        error("pivot root");
      }

      if(umount2("./oldroot", MNT_DETACH)) {
        error("umount old root");
      }

      if (rmdir("./oldroot")) {
        error("remove old root");
      }
}

void exec(char command[]) {
    int sys_error = system(command);
    if ( -1 != sys_error) {
        error("exec");
    }
}

#define DEFAULT_CPU_SHARES        1024
#define DEFAULT_CPU_PERIOD        100000
#define DEFAULT_CPU_QUOTA_PERCENT 100
#define DEFAULT_CPU_QUOTA_CPUS    1


#define DEFAULT_MEMORY_LIMIT      -1      // memory.memsw.limit_in_bytes

inline long cpu_quota(unsigned long period, unsigned long quota_percent, unsigned char cpus) {
    ASSERT(quota_percent <= 100L);
    return (period * quota_percent * cpus) / 100;
}

void config(size_t argc, char** argv) {
    unsigned long cpu_shares   = DEFAULT_CPU_SHARES;
    unsigned long cpu_period   = DEFAULT_CPU_PERIOD;
    unsigned long cpu_percent  = DEFAULT_CPU_QUOTA_PERCENT;
    unsigned long cpu_cpus     = DEFAULT_CPU_QUOTA_CPUS;

    for (size_t i=1; i < argc; i++) {
        if (!strcmp(argv[i], "--shares"))
        {
            cpu_shares = strtoul(argv[++i], NULL, 0);
            ASSERT(cpu_shares > 0);
            printf("CPU_SHARES is set to %lu\n", cpu_shares);
        }
        else if (!strcmp(argv[i], "--period"))
        {
            cpu_period = strtoul(argv[++i], NULL, 0);
            ASSERT(cpu_period > 0);
            printf("CPU_PERIOD is set to %lu\n", cpu_period);
        }
        else if (!strcmp(argv[i], "--percent"))
        {
            cpu_percent = strtoul(argv[++i], NULL, 0);
            ASSERT(cpu_percent > 0 && cpu_percent <= 100);
            printf("CPU_PERCENT is set to %lu\n", cpu_percent);
        }
        else if (!strcmp(argv[i], "--cpus"))
        {
            cpu_cpus = strtoul(argv[++i], NULL, 0);
            ASSERT(cpu_percent > 0);
            printf("CPU_CPUS is set to %lu\n", cpu_cpus);
        }
        else
        {
            printf("%s is not a valid configuration. See --help\n", argv[i]);
            exit(1);
        }
    }

    char command[200];

    system("mkdir /sys/fs/cgroup/doquerinho");
    system("mount -t cgroup -o cpu none /sys/fs/cgroup/doquerinho");
}

void doqueru(int argc, char** argv) {
    int status;
    pid_t pid_proc0, pid_exec;

    config(argc, argv); //Future

    setup_new_env();
    safe_unshare();
    safe_sethostname();

    if (mount("none", "/", NULL, MS_REC|MS_PRIVATE, NULL)) { // This must happen, otherwise the unshare Mount namespace wont work properly
      error("mount for unshare");
    }


    pivot_root_routine();
    mount_proc();// Verify if another stuff should be mounted too

    if ((pid_proc0 = fork()) == 0) {  // PID = 1
        char COMMAND_TO_EXECUTE[] = "/bin/bash";
        char* COMMAND_ARGS[2] = {COMMAND_TO_EXECUTE, NULL};
        if (fork() == 0) {
            if (execvp(COMMAND_TO_EXECUTE, COMMAND_ARGS)) {
                error("exec");
            }
        }

        while (wait(&status) != -1 || errno != ECHILD);
        printf("init has died!\n");
    }
    wait(&status);
}


int main(int argc, char** argv) {
  doqueru(argc, argv);
  return 0;
}
