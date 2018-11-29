#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <errno.h>
#include <sched.h>
#include <string.h>

using namespace std;

#define MOUNT_DIR "./doquerinho's_shell"	//checar pronuncia

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

void fork_and_exec_env_unshareutspid_chdir_pivot(char** argv) {
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


int main(int argc, char** argv) {
  fork_and_exec_env_unshareutspid_chdir(argv);
  return 0;
}
