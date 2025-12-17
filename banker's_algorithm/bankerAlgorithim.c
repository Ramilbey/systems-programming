//the current state lead to a deadlock and banker's lagorithms detects the deadlock and prevents it from happening by changing the state to unsafe (denying the request by the processes because there are not enough reouserces for all processes to finish)

#include <stdio.h>
#include <stdbool.h>
#define P 5
#define R 3

void calculateRequest(int request[P][R], int claim[P][R], int alloc[P][R]) {
    printf("The request matrix\n");
    for (int i = 0; i < P; i++) {
        for (int j = 0; j < R; j++) {
            request[i][j] = claim[i][j] - alloc[i][j];
            printf("\t%d", request[i][j]);
        }
        printf("\n");
    }
}

bool safe(int processes[P], int available[R], int claim[P][R], int alloc[P][R]) {
    int request[P][R];
    calculateRequest(request, claim, alloc);
    bool finish[P] = {0};
    int safeSeq[P];
    int currentavail[R];
    
    for (int i = 0; i < R; i++) {
        currentavail[i] = available[i];
    }
    
    printf("current avail. resource\n");
    for (int i = 0; i < R; i++) {
        printf("%d ", currentavail[i]);
    }
    printf("\n");
    
    int count = 0;
    while (count < P) {
        bool found = false;
        for (int i = 0; i < P; i++) {
            if (finish[i] == 0) {
                int j;
                for (j = 0; j < R; j++) {
                    if (request[i][j] > currentavail[j]) {
                        printf("Process %d failed: resource %d < request %d\n", i, j, request[i][j]);
                        break;
                    }
                }
                if (j == R) {
                    printf("Process %d can run to complete\n", i);
                    for (j = 0; j < R; j++) {
                        printf("%d ", alloc[i][j]);
                    }
                    printf(" released\n");
                    for (int k = 0; k < R; k++) {
                        currentavail[k] += alloc[i][k];
                    }
                    printf("current avail. resources changed as\n");
                    for (int j = 0; j < R; j++) {
                        printf("%d ", currentavail[j]);
                    }
                    printf("\n");
                    safeSeq[count++] = i;
                    finish[i] = 1;
                    found = true;
                }
            }
        }
        if (found == false) {
            printf("System is not in safe state\n");
            return false;
        }
    }
    
    printf("System is in safe state. Safe path is: ");
    for (int i = 0; i < P; i++) {
        printf("%d ", safeSeq[i]);
    }
    printf("\n");
    return true;
}

int main() {
    int processes[P] = {0, 1, 2, 3, 4};
    int available[R] = {0, 0, 1};
    int claim[P][R] = {
        {7, 5, 3},
        {3, 2, 2},
        {9, 0, 2},
        {2, 2, 2},
        {4, 3, 3}
    };
    int alloc[P][R] = {
    {5, 3, 2},
    {2, 1, 0},
    {3, 0, 2},
    {2, 1, 1},
    {0, 0, 2}
    };
    safe(processes, available, claim, alloc);
    return 0;
}
