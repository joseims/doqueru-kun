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
  printf("Now the father! My child has gone :(\n");
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

void safe_unshare(int flags) {
	if (0 == unshare(flags)) {
		printf("unshare worked!\n");
	} else {
		printf("unshare has failed! :( [errno=%d]\n", errno);
	}
}

void safe_sethostname() {
	char new_name[] = "carl";
	if (0 == sethostname(new_name, strlen(new_name))) {
		printf("sethostname worked!\n");
	} else {
		printf("sethostname has failed! :( [errno=%d]\n", errno);
	}
}

void fork_and_exec_env_unshareuts(char** argv) {
  int* status;
  pid_t pid;
  int uns_flags = CLONE_NEWUTS;
  if ((pid = fork()) == 0) {
    clearenv();
	safe_unshare(uns_flags);
	safe_sethostname();
    execvp("./hello_worldo", argv+1);
  }
  wait(status);
  printf("===============\n");
  printf("Now the father!\n");
  printf("===============\n");
  execvp("./hello_worldo", argv+1);
}

void fork_and_exec_env_unshareutspid(char** argv) {
  int* status;
  pid_t pid;
  int uns_flags = CLONE_NEWUTS | CLONE_NEWPID;
  if ((pid = fork()) == 0) {
    clearenv();
	safe_unshare(uns_flags);
	safe_sethostname();
    execvp("./hello_worldo", argv+1);
  }
  wait(status);
  printf("===============\n");
  printf("Now the father!\n");
  printf("===============\n");
  execvp("./hello_worldo", argv+1);
}

void download_alpine();

void setup_new_env() {
  clearenv();
  setenv("TERM", "xterm-256color", 0);
  setenv("PATH", "/bin/:/sbin/:usr/bin:/usr/sbin", 0);	//remember to explain paths
}

void fork_and_exec_env_unshareutspid_chdir(char** argv) {
  int* status;
  pid_t pid;
  int uns_flags = CLONE_NEWUTS | CLONE_NEWPID;
  if ((pid = fork()) == 0) {
    setup_new_env();
	safe_unshare(uns_flags);
	safe_sethostname();
	//remember alpine
	chroot(MOUNT_DIR);
  	chdir("/");
    execvp("./hello_worldo", argv+1);
  }
  wait(status);
  printf("===============\n");
  printf("Now the father!\n");
  printf("===============\n");
  execvp("./hello_worldo", argv+1);
}

#define pivot_root(new_root, put_old) syscall(SYS_pivot_root, new_root, put_old)

void ls() {
  int status;
  if (fork() == 0) {
    char* args_ls[] = {"./doquerinhos_shell/bin/ls", "-la", NULL};
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

void fork_and_exec_env_unshareutspid_chdir_pivot(char** argv) {
  int status;
  pid_t pid;
  int uns_flags = CLONE_NEWUTS | CLONE_NEWPID | CLONE_NEWNS;

  if ((pid = fork()) == 0) {
    setup_new_env();
  	safe_unshare(uns_flags);
  	safe_sethostname();

    if (0 != chdir(MOUNT_DIR)){
      printf("chdir errno=%d\n", errno);
    }
    if (0 != chdir("..")) {
      printf("chdir(..) failed with errno=%d\n", errno);
    }
    pwd();
    ls();
    char old_mount[100];
    {
      strcpy(old_mount, MOUNT_DIR);
      strcat(old_mount, "/old_mount");
    }
  	if (0 != mkdir(old_mount, 0755)) {
      printf("mkdir(%s) failed with errno=%d\n", old_mount, errno);
    }
    if (0 != mount(MOUNT_DIR, MOUNT_DIR, NULL, MS_REC | MS_BIND | MS_PRIVATE, NULL)) {
      printf("mount(%s) failed with errno=%d\n", MOUNT_DIR, errno);
    }

    if (0 != pivot_root(MOUNT_DIR, old_mount)) {
      printf("pivot_root(%s, %s) failed with errno=%d\n", MOUNT_DIR, old_mount, errno);
    }
    chroot(".");
    chdir(MOUNT_DIR);
    umount2(old_mount,MNT_DETACH);
    rmdir(old_mount);

    printf("===============\n");
    printf("Running child! pid[%d]\n", pid);
    execvp("/bin/hello_worldo", argv + 1);
    printf("child keep running error[%d]\n", errno);
    exit(1);
  }
  wait(&status);
  printf("===============\n");
  printf("Now the father! pid[%d]\n", pid);
  printf("===============\n");
  execvp("./hello_worldo", argv + 1);
}


int main(int argc, char** argv) {
  fork_and_exec_env_unshareutspid_chdir_pivot(argv);
  return 0;
}
