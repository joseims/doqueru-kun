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
#include <fcntl.h>

#include <string>


#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif // _GNU_SOURCE

#define STRINGIZE2(x) #x
#define STRINGIZE(x) STRINGIZE2(x)

/**
 * A shortcut to call _assert with the correct line number.
 */
#define ASSERTMSG(condition, msg) _assert(condition, msg, __LINE__)

/**
 * Same as ASSERTMSG, but will use the condition as string to make the error message.
 */
#define ASSERT(condition) ASSERTMSG(condition, "assert " STRINGIZE(condition) " has failed")

#define pivot_root(new_root, put_old) syscall(SYS_pivot_root, new_root, put_old)

const char MOUNT_DIR[] = "./doquerinhos_shell";

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


const int DEFAULT_CPU_SHARES        =   1024;
const int DEFAULT_CPU_PERIOD        = 100000;
const int DEFAULT_CPU_QUOTA_PERCENT =    100;
const int DEFAULT_CPU_QUOTA_CPUS    =      1;
const int DEFAULT_MEMORY_LIMIT      =     -1;      // memory.memsw.limit_in_bytes

inline long cpu_quota(unsigned long period, unsigned long quota_percent, unsigned char cpus) {
    ASSERT(quota_percent <= 100L);
    return (period * quota_percent * cpus) / 100;
}

struct rule {
    std::string path;
    std::string name;
    std::string value;
    std::string controller;
};

void cgroup_rule(const char path[], const char rule_name[], const char rule_value[]) {
    char full_path[256];
    strcpy(full_path, path);
    strcat(full_path, rule_name);
    printf("%s = %s\n", full_path, rule_value);

    int fd = open(full_path, O_CREAT | O_WRONLY | O_APPEND);
    ASSERTMSG(0 != write(fd, rule_value, strlen(rule_value)), full_path);
    close(fd);

    strcpy(full_path, path);
    strcat(full_path, "cgroup.procs");

    ASSERTMSG(1 != (fd = open(full_path, O_CREAT | O_WRONLY | O_APPEND)), "error on opening cgroup.procs");
    ASSERTMSG(0 != write(fd, "0", strlen("0")), "error on registering groups");
    close(fd);
}


bool IsPathExist(const std::string &s) {
    struct stat buffer;
    return (stat (s.c_str(), &buffer) == 0);
}

void cgroup(const char name[], const rule* configs, size_t configs_len) {
    ASSERTMSG(name[strlen(name) - 1] == '/', "last character of cgroup name should be \"/\"");  // just to not mess up when concatening

    for (int i=0; i < configs_len; i++) {
        char path[200] = "/sys/fs/cgroup/";
        strcat(path, configs[i].controller.c_str());
        strcat(path, configs[i].path.c_str());

        ASSERTMSG(-1 != mkdir(path, 0755) || errno == EEXIST, "error on creating cgroup directory");

        cgroup_rule(path, configs[i].name.c_str(), configs[i].value.c_str());
    }
}

std::string from_cstr(const char word[]) {
    std::string out = word;
    return out;
}

int get_execpath_index(int argc, char ** argv) {
    int i;
    for(i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "--shares") || !strcmp(argv[i], "--period") || !strcmp(argv[i], "--percent") || !strcmp(argv[i], "--cpus") || !strcmp(argv[i], "--memlimit")) {
            i++;
        } else {
            return i;
        }
    }
    return i;
}

void config(size_t argc, char** argv) {
    char cgroup_name[]   = "doquerinho/";   // should be possible to create different cgroups
    unsigned long cpu_shares   = DEFAULT_CPU_SHARES;
    unsigned long cpu_period   = DEFAULT_CPU_PERIOD;
    unsigned long cpu_percent  = DEFAULT_CPU_QUOTA_PERCENT;
    unsigned long cpu_cpus     = DEFAULT_CPU_QUOTA_CPUS;

    unsigned long mem_max      = 0;

    rule configs[20];

    int execpath_index = get_execpath_index(argc,argv);

    int c = 0;
    const char* temp;
    for (size_t i=1; i < execpath_index; i++) {
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
            ASSERT(mem_max > 0);
            printf("MEM_MAX is set to %lu\n", mem_max);
        }
        else
        {
            printf("%s is not a valid configuration. See --help\n", argv[i]);
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

void doqueru(char* exec_path, int argc, char** argv) {

    unsharenamespaces();
    pivot_root_routine();
    config(argc, argv);

    pid_t child_pid;
    ASSERTMSG(-1 != (child_pid = fork()), "fork PID 1 has failed");

    int status;

    if (child_pid == 0) {   // init process
        mount_proc();

        ASSERTMSG(-1 != (child_pid = fork()), "fork PID 2 has failed");

        int execpath_index = get_execpath_index(argc,argv);
        if (child_pid == 0) ASSERTMSG(-1 != execvp(exec_path, &argv[execpath_index + 1]), "exec has failed");
        while (wait(&status) != -1 || errno != ECHILD);

        fprintf(stderr, "init died\n");
    }

    wait(&status);
}

int main(int argc, char** argv) {
    char* execpath = argv[get_execpath_index(argc,argv)];
    doqueru(execpath, argc-1, argv);
    exit(0);
}
