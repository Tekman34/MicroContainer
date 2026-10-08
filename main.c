#define _GNU_SOURCE // Required to unlock the clone() system call in C
#include <sched.h>  // For clone() and namespace flags
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <linux/capability.h>
#include <sys/syscall.h>
#include <sys/prctl.h>
// Allocate 1 Megabyte for the child's stack
#define STACK_SIZE (1024 * 1024) 
void setup_groups(pid_t child_pid) {
    mkdir("/sys/fs/cgroup/microjail", 0755);
    FILE *f_pids = fopen("/sys/fs/cgroup/microjail/pids.max", "w");
if (f_pids != NULL) {
    fprintf(f_pids, "20");
    fclose(f_pids);
    
}FILE *f_procs = fopen("/sys/fs/cgroup/microjail/cgroup.procs", "w");
if (f_procs != NULL) {
    fprintf(f_procs, "%d", child_pid);
    fclose(f_procs);
}
}
// This is the function the child process will execute when spawned
int child_payload(void *arg) {
    char *new_hostname = "container";
    char *argv[] = {"/bin/sh", NULL};
    char *envp[] = { "PATH=/bin:/usr/bin", NULL};

    // 1. Change the hostname (do this before dropping capabilities!)
    if (sethostname(new_hostname, strlen(new_hostname)) == -1) {
        perror("sethostname failed");
        return -1;
    }

    // 2. Isolate the filesystem
    if (chroot("./rootfs") == -1) {
        perror("chroot failed");
        return -1;
    }
    if (chdir("/") == -1) {
        perror("chdir failed");
        return -1;
    }
    if (mount("proc", "/proc", "proc", 0, NULL) == -1) {
        perror("mount proc failed");
        return -1;
    }

    // 3. Destroy the hidden Bounding Set so execve() can't restore privileges
    for (int i = 0; i <= 63; i++) {
        prctl(PR_CAPBSET_DROP, i, 0, 0, 0);
    }
    struct __user_cap_header_struct capheader;
    struct __user_cap_data_struct capdata[2];
    memset(&capheader, 0, sizeof(capheader));
    memset(&capdata, 0, sizeof(capdata));
    capheader.version = _LINUX_CAPABILITY_VERSION_3;
    capheader.pid =0;
    if(syscall(SYS_capset, &capheader, capdata) < 0) {
        perror("capset failed");
        return -1;
    }
    
    execve("/bin/sh", argv, envp);
    
    perror("execve failed");
    return -1;
}

int main(int argc, char *argv[]) {
    printf("Parent: Starting microjail...\n");

    // 1. Allocate memory for the child process's stack
    char *child_stack = malloc(STACK_SIZE);
    if (child_stack == NULL) {
        perror("malloc failed");
        exit(EXIT_FAILURE);
    }

    // 2. Define our namespace flags. 
    // CLONE_NEWUTS isolates the hostname.
    // SIGCHLD tells the kernel to send a signal to the parent when the child dies, 
    // which is required for waitpid() to work correctly.
    int flags = CLONE_NEWUTS |CLONE_NEWNS| CLONE_NEWPID | SIGCHLD; 

    // 3. Spawn the child process!
    // Notice we pass child_stack + STACK_SIZE. Stacks on Linux grow downwards, 
    // so we must pass a pointer to the TOP of the allocated memory block.
    pid_t child_pid = clone(child_payload, child_stack + STACK_SIZE, flags, NULL);
    printf("Parent: Spawned child process with PID %d\n", child_pid);
    setup_groups(child_pid);

    if (child_pid == -1) {
        perror("clone failed");
        exit(EXIT_FAILURE);
    }

    printf("Parent: Spawned container process with PID %d\n", child_pid);

    // 4. Parent process waits here until the child exits
    waitpid(child_pid, NULL, 0);
    printf("Parent: Container exited. Shutting down.\n");

    free(child_stack);
    return 0;
}