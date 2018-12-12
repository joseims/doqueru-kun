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

#define TRY(x) if (x) fatal_errno(__LINE__)

void fatal_errno(int line)
{
   printf("error at line %d, errno=%d\n", line, errno);
   exit(1);
}

void fork_and_exec_env_unshareutspid_chdir_pivot(char** argv) {
  int status;
  pid_t pid;
  int uns_flags = CLONE_NEWUTS | CLONE_NEWPID | CLONE_NEWNS;

  if ((pid = fork()) == 0) {
    setup_new_env();
  	safe_unshare(uns_flags);
  	safe_sethostname();

    /* Enter the mount and user namespaces. Note that in some cases (e.g., RHEL
       6.8), this will succeed even though the userns is not created. In that
       case, the following mount(2) will fail with EPERM. */
    TRY (unshare(CLONE_NEWNS|CLONE_NEWUSER));



    /* Ensure that our image directory exists. It doesn't really matter what's
       in it. */
    if (mkdir("/tmp/newroot", 0755) && errno != EEXIST)
       TRY (errno);


    /* Claim the image for our namespace by recursively bind-mounting it over
       itself. This standard trick avoids conditions 1 and 2. */
    TRY (mount("/tmp/newroot", "/tmp/newroot", NULL,
               MS_REC | MS_BIND | MS_PRIVATE, NULL));

    /* The next few calls deal with condition 3. The solution is to overmount
       the root filesystem with literally anything else. We use the parent of
       the image, /tmp. This doesn't hurt if / is not a rootfs, so we always do
       it for simplicity. */

    /* Claim /tmp for our namespace. You would think that because /tmp contains
       /tmp/newroot and it's a recursive bind mount, we could claim both in the
       same call. But, this causes pivot_root(2) to fail later with EBUSY. */
    TRY (mount("/tmp", "/tmp", NULL, MS_REC | MS_BIND | MS_PRIVATE, NULL));

    /* chdir to /tmp. This moves the process' special "." pointer to
       the soon-to-be root filesystem. Otherwise, it will keep pointing to the
       overmounted root. See the e-mail at the end of:
       https://git.busybox.net/busybox/tree/util-linux/switch_root.c?h=1_24_2 */
    TRY (chdir("/tmp"));

    /* Move /tmp to /. (One could use this to directly enter the image,
       avoiding pivot_root(2) altogether. However, there are ways to remove all
       active references to the root filesystem. Then, the image could be
       unmounted, exposing the old root filesystem underneath. While
       Charliecloud does not claim a strong isolation boundary, we do want to
       make activating the UDSS irreversible.) */
    TRY (mount("/tmp", "/", NULL, MS_MOVE, NULL));

    /* Move the "/" special pointer to the new root filesystem, for the reasons
       above. (Similar reasoning applies for why we don't use chroot(2) to
       directly activate the UDSS.) */
    TRY (chroot("."));

    /* Make a place for the old (intermediate) root filesystem to land. */
    if (mkdir("/newroot/oldroot", 0755) && errno != EEXIST)
       TRY (errno);

    /* Re-mount the image read-only. */
    TRY (mount(NULL, "/newroot", NULL, MS_REMOUNT | MS_BIND | MS_RDONLY, NULL));

    /* Finally, make our "real" newroot into the root filesystem. */
    TRY (chdir("/newroot"));
    TRY (syscall(SYS_pivot_root, "/newroot", "/newroot/oldroot"));
    TRY (chroot("."));

    /* Unmount the old filesystem and it's gone for good. */
    TRY (umount2("/oldroot", MNT_DETACH));


    printf("===============\n");
    printf("Running child! pid[%d]\n", pid);
    TRY(execvp("/bin/bash", argv + 1));
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
