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

char MOUNT_DIR[] = "./doquerinhos_shell2";	//checar pronuncia

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif


void error(char error_case[]) {
    printf("Error at %s(%d)\n",error_case,errno);
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

#define pivot_root(new_root, put_old) syscall(SYS_pivot_root, new_root, put_old)


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

void config(char** argv) {
  return;
}

void doqueru(char** argv) {
  int status;
  pid_t pid_proc0,pid_exec;

  config(argv); //Future
   if ((pid_proc0 = fork()) == 0) { 
      setup_new_env();
      safe_unshare();
      safe_sethostname();
      
      if ((pid_exec = fork()) == 0) {
        if (mount("none", "/", NULL, MS_REC|MS_PRIVATE, NULL)) { // This must happen, otherwise the unshare Mount namespace wont work properly
          error("mount for unshare");
        } 

        mount_proc();// Verify if another stuff should be mounted too
        pivot_root_routine();
        if (execvp("/bin/sh",argv+1)) {
          error("exec");
        }
        exit(1);
      }
   wait(&status); 
   exit(1);
   }
  wait(&status);
}


int main(int argc, char** argv) {
  doqueru(argv);
  return 0;
}
