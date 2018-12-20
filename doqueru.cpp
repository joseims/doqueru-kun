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
#include <fcntl.h>
#include <map>

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

void mount_proc() {
    if (mkdir("/proc", 0755) && errno != EEXIST) {
        error("create /proc");
    }
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
    if (fork() == 0) {
        if (execvp("/bin/bash",NULL)) {
            error("exec");
        }
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

struct rule {
    string path;
    string name;
    string value;
    string controller;
};

void cgroup_rule(const char path[], const char rule_name[], const char rule_value[]) {
    char full_path[256];
    strcpy(full_path, path);
    strcat(full_path, rule_name);
    printf("%s = %s\n", full_path, rule_value);

    int fd = open(full_path, O_CREAT | O_WRONLY | O_APPEND);
    if (!write(fd, rule_value, strlen(rule_value))) {
        error("write rule fail");
    }
    close(fd);

    strcpy(full_path, path);
    strcat(full_path, "cgroup.procs");
    printf("registered at %s\n", full_path);

    if ((fd = open(full_path, O_CREAT | O_WRONLY | O_APPEND)) < 0) {
        error("opening cgroup.procs");
    }
    if (!write(fd, "0", strlen("0"))) {
        error("registering groups fail");
    }
    close(fd);
}

void cgroup(const char name[], const rule* configs, size_t configs_len) {
    ASSERT(name[strlen(name) - 1] == '/');  // just to not mess up when concatening

    for (int i=0; i < configs_len; i++) {
        char path[200] = "/sys/fs/cgroup/";
        strcat(path, configs[i].controller.c_str());
        strcat(path, configs[i].path.c_str());

        if (mkdir(path, 0755) && errno != EEXIST) {
            printf("%s\n", path);
            error("create /cgroup");
        }

        cgroup_rule(path, configs[i].name.c_str(), configs[i].value.c_str());
    }
}

string from_cstr(const char word[]) {
    string out = word;
    return out;
}

void config(size_t argc, char** argv) {
    char cgroup_name[]   = "doquerinho/";
    unsigned long cpu_shares   = DEFAULT_CPU_SHARES;
    unsigned long cpu_period   = DEFAULT_CPU_PERIOD;
    unsigned long cpu_percent  = DEFAULT_CPU_QUOTA_PERCENT;
    unsigned long cpu_cpus     = DEFAULT_CPU_QUOTA_CPUS;

    unsigned long mem_max      = 0;

    rule configs[20];

    int c = 0;
    const char* temp;
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
            ASSERT(cpu_cpus > 0);
            printf("CPU_CPUS is set to %lu\n", cpu_cpus);
        }
        else if (!strcmp(argv[i], "--memlimit"))
        {
            temp = argv[++i];
            configs[c].path = from_cstr(cgroup_name);
            configs[c].name = "memory.kmem.limit_in_bytes";
            configs[c].value = from_cstr(temp);
            configs[c++].controller = "memory/";
            mem_max = strtoul(temp, NULL, 0);
            ASSERT(cpu_percent > 0);
            printf("MEM_MAX is set to %lu\n", mem_max);
        }
        else
        {
            printf("%s is not a valid configuration. See --help\n", argv[i]);
            exit(1);
        }
    }

    {
      char value[50];
      sprintf(value, "%lu", cpu_shares);
      configs[c].path = from_cstr(cgroup_name);
      configs[c].name = "cpu.shares";
      configs[c].value = from_cstr(value);
      configs[c++].controller = "cpu/";
    }
    {
      char value[50];
      sprintf(value, "%lu", (cpu_period / 100) * cpu_percent);
      configs[c].path = from_cstr(cgroup_name);
      configs[c].name = "cpu.cfs_period_us";
      configs[c].value = from_cstr(value);
      configs[c++].controller = "cpu/";
    }
    {
      char value[50];
      sprintf(value, "%lu", cpu_quota(cpu_period, cpu_percent, cpu_cpus));
      configs[c].path = from_cstr(cgroup_name);
      configs[c].name = "cpu.cfs_quota_us";
      configs[c].value = from_cstr(value);
      configs[c++].controller = "cpu/";
    }


    cgroup(cgroup_name, configs, c);
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

    if ((pid_proc0 = fork()) == 0) {  // PID = 1
        mount_proc();// Verify if another stuff should be mounted too

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
