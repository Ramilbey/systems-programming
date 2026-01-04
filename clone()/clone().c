#include <stdio.h>
#include <stdlib.h>
#include <sched.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>

#define STACK_SIZE (1024 * 1024)  // 1MB stack per process/thread

// ========== PART 3: Thread in Child 2 (using clone) ==========
int thread_func(void* arg) {
    char* message = (char*)arg;
    printf("Thread in Child 2: Message = '%s'\n", message);
    printf("Thread PID: %d, Thread PPID: %d\n", getpid(), getppid());
    return 0;
}

// ========== PART 2: Child 2 Process ==========
int child2_func(void* arg) {
    char* message = (char*)arg;
    
    printf("\n=== Child 2 Process ===\n");
    printf("Child 2 PID: %d, Parent PID (Child 1): %d\n", getpid(), getppid());
    printf("Child 2 received: '%s'\n", message);
    
    // Create thread using clone() - SHARE memory with Child 2!
    char* thread_stack = malloc(STACK_SIZE);
    if (!thread_stack) {
        perror("malloc failed for thread");
        exit(1);
    }
    
    // Create THREAD (shares memory) using clone()
    // CLONE_VM = Share memory space (makes it a thread!)
    // CLONE_FILES = Share file descriptors
    // CLONE_FS = Share filesystem info
    // CLONE_SIGHAND = Share signal handlers
    // CLONE_THREAD = Part of same thread group
    int thread_pid = clone(thread_func,
                          thread_stack + STACK_SIZE,
                          CLONE_VM | CLONE_FILES | CLONE_FS | 
                          CLONE_SIGHAND | CLONE_THREAD | SIGCHLD,
                          message);
    
    if (thread_pid == -1) {
        perror("clone failed for thread");
        free(thread_stack);
        exit(1);
    }
    
    waitpid(thread_pid, NULL, 0);  // Wait for thread to finish
    free(thread_stack);
    
    printf("Child 2 exiting...\n");
    return 0;
}

// ========== PART 1: Child 1 Process ==========
int child1_func(void* arg) {
    char* message = (char*)arg;
    
    printf("\n=== Child 1 Process ===\n");
    printf("Child 1 PID: %d, Parent PID: %d\n", getpid(), getppid());
    printf("Child 1 received: '%s'\n", message);
    
    // Create Child 2 using clone() - SEPARATE process
    char* child2_stack = malloc(STACK_SIZE);
    if (!child2_stack) {
        perror("malloc failed for Child 2");
        exit(1);
    }
    
    // Create PROCESS (separate) using clone()
    // SIGCHLD only = regular process (no sharing)
    int child2_pid = clone(child2_func, 
                          child2_stack + STACK_SIZE,
                          SIGCHLD,  // Regular process (no sharing)
                          message);
    
    if (child2_pid == -1) {
        perror("clone failed for Child 2");
        free(child2_stack);
        exit(1);
    }
    
    waitpid(child2_pid, NULL, 0);  // Wait for Child 2
    free(child2_stack);
    
    printf("Child 1 exiting...\n");
    return 0;
}

// ========== MAIN: Parent Process ==========
int main() {
    char message[] = "Hello from Parent!";
    
    printf("=== Parent Process ===\n");
    printf("Parent PID: %d\n", getpid());
    printf("Original message: '%s'\n", message);
    
    // Create Child 1 using clone() - SEPARATE process
    char* child1_stack = malloc(STACK_SIZE);
    if (!child1_stack) {
        perror("malloc failed");
        return 1;
    }
    
    int child1_pid = clone(child1_func,
                          child1_stack + STACK_SIZE,
                          SIGCHLD,  // Regular process (no sharing)
                          message);
    
    if (child1_pid == -1) {
        perror("clone failed");
        free(child1_stack);
        return 1;
    }
    
    waitpid(child1_pid, NULL, 0);  // Wait for Child 1
    free(child1_stack);
    
    printf("\nParent exiting...\n");
    return 0;
}
