#include <string.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>       // nanosleep()
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include "common.h"

// definizione struttura memoria
struct shared_memory {
    int buf [BUFFER_SIZE];
    int read_index;
    int write_index;
};

//definizione shared memory
struct shared_memory *myshm_ptr;
int fd_shm;

void initMemory() {
    /** COMPLETE THE FOLLOWING CODE BLOCK
     *
     * Request the kernel to creare a shared memory, set its size to the size of
     * struct shared_memory, and map the shared memory in the shared_mem_ptr variable.
     * Initialize the shared memory to 0.
     **/
    fd_shm = shm_open(SH_MEM_NAME, O_CREAT | O_RDWR, 0666);
    if(fd_shm == -1) handle_error("initMemory: shm_open");

    if(ftruncate(fd_shm, sizeof(struct shared_memory)) == -1)
        handle_error("initMemory: ftruncate");
    
    myshm_ptr = mmap(0, sizeof(struct shared_memory), PROT_READ | PROT_WRITE, MAP_SHARED, fd_shm, 0);
    if(myshm_ptr == MAP_FAILED)
        handle_error("initMemory: mmap");

    memset(myshm_ptr, 0, sizeof(struct shared_memory));
}

void closeMemory() {
    /** COMPLETE THE FOLLOWING CODE BLOCK
     *
     * unmap the shared memory, unlink the shared memory and close its descriptor
     **/
    if (munmap(myshm_ptr, sizeof(struct shared_memory)) == -1) {
        handle_error("closeMemory: munmap");
    }

    // Close the shared memory file descriptor
    if (close(fd_shm) == -1) {
        handle_error("closeMemory: close");
    }

}


// generates a number between -MAX_TRANSACTION and +MAX_TRANSACTION
static inline int performRandomTransaction() {
    struct timespec pause = {0};
    pause.tv_nsec = 10000000; // 10 ms (100*10^6 ns)
    nanosleep(&pause, NULL);

    int amount = rand() % (2 * MAX_TRANSACTION); // {0, ..., 2*MAX_TRANSACTION - 1}
    if (amount >= MAX_TRANSACTION) {
        return MAX_TRANSACTION - (amount+1); // {-MAX_TRANSACTION, ..., -1}
    } else {
        return amount + 1; // {1, ..., MAX_TRANSACTION}
    }
}

void produce(int id, int numOps) {
    int localSum = 0;

    while (numOps > 0) {
        int value = performRandomTransaction();

        // Busy wait until there is space in the buffer
        while ((myshm_ptr->write_index + 1) % BUFFER_SIZE == myshm_ptr->read_index) {
            // Buffer full, busy wait
        }

        // Write the value to the buffer and update the write_index
        myshm_ptr->buf[myshm_ptr->write_index] = value;
        myshm_ptr->write_index = (myshm_ptr->write_index + 1) % BUFFER_SIZE;

        localSum += value;
        numOps--;
    }

    printf("Producer %d ended. Local sum is %d\n", id, localSum);
}

int main(int argc, char** argv) {
    srand(PRNG_SEED);
    initMemory();

    produce(0, OPS_PER_PRODUCER);

    printf("Producer has terminated. Exiting...\n");
    closeMemory();

    exit(EXIT_SUCCESS);
}

