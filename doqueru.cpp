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


void safe_unshare(int flags) {
	if (0 == unshare(flags)) {
		printf("unshare worked!\n");
	} else {
		printf("unshare has failed! :( [errno=%d]\n", errno);
		exit(1);
	}
}

void safe_sethostname() {
	char new_name[] = "carl";
	if (0 == sethostname(new_name, strlen(new_name))) {
		printf("sethostname worked!\n");
	} else {
		printf("sethostname has failed! :( [errno=%d]\n", errno);
		exit(1);
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


void fatal_errno(int line)
{
   printf("error at line %d, errno=%d\n", line, errno);
   exit(1);
}

#define TRY(x) if (x) fatal_errno(__LINE__)

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

void doqueru(char** argv) {
  int status;
  pid_t pid_dad,pid_grandad;
  int uns_flags = CLONE_NEWUTS | CLONE_NEWPID | CLONE_NEWNS;

   if ((pid_dad = fork()) == 0) { // AVO PARA O PID FUNCIONAR
      setup_new_env();
      safe_unshare(uns_flags);
      safe_sethostname();
      if (pid_grandad = fork() == 0) {
      TRY(mount("none", "/", NULL, MS_REC|MS_PRIVATE, NULL));

      TRY(mount(MOUNT_DIR, MOUNT_DIR, NULL,
                  MS_BIND | MS_PRIVATE | MS_REC, NULL));

      chdir(MOUNT_DIR);

      /* Make a place for the old (intermediate) root filesystem to land. */
      if (mkdir("oldroot", 0755) && errno != EEXIST)
        TRY (errno);


      TRY(pivot_root(".", "./oldroot"));

 


      //MOUNT PROC ETC
      TRY(mount("proc","/proc","proc",NULL,NULL));


      /* Unmount the old filesystem and it's gone for good. */
      TRY (umount2("./oldroot", MNT_DETACH));
      rmdir("./oldroot");

        printf("===============\n");
        printf("Running child! pid[%d]\n", pid_grandad);

        int a = system("/bin/sh");
        if ( -1 != a) {
          printf("child keep running error[%d]\n", errno);
        }
   exit(1);
   }
   wait(&status); 
   exit(1);
   }
  wait(&status);
  printf("===============\n");
  printf("Now the father! pid[%d]\n", pid_dad);
  printf("===============\n");
  execvp("./hello_worldo", argv + 1);
}


int main(int argc, char** argv) {
  doqueru(argv);
  return 0;
}
