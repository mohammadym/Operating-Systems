//
// Created by amin on 12/3/20.
//

#ifndef Q1_SHARED_MEMORY_H
#define Q1_SHARED_MEMORY_H

#include <stdbool.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#include "shared_memory.h"

// Constants
#define BLOCK_SIZE 1024 * 5
#define SHARED_FILE_NAME ".shared_file"
#define PROCESS_FILE_NAME ".process_file"
#define NUMBER_OF_BLOCKS 8
#define SEM_FILE_PRODUCER ".producer"
#define SEM_FILE_CONSUMER ".consumer"
#define IPC_ERROR (-1)

// link functions
static int get_shared_block_id(char *filename, int size, int block_index) {
    key_t key;

    key = ftok(filename, block_index);
    if (key == IPC_ERROR) {
        return IPC_ERROR;
    }

    return shmget(key, size, IPC_CREAT | 0644);
}

void *attach_memory_block(char *filename, int size, int block_index) {
    int share_block_id;
    void *result;

    share_block_id = get_shared_block_id(filename, size, block_index);
    if (share_block_id == IPC_ERROR) {
        return NULL;
    }

    result = shmat(share_block_id, NULL, 0);
    if (result == (void *) IPC_ERROR) {
        return NULL;
    }

    return result;
}

bool detach_memory_block(void *block) {
    return shmdt(block) != IPC_ERROR;
}

bool destroy_memory_block(char *filename, int block_index) {
    int shared_memory_id;

    shared_memory_id = get_shared_block_id(filename, 0, block_index);
    if (shared_memory_id == IPC_ERROR) {
        return NULL;
    }

    return shmctl(shared_memory_id, IPC_RMID, NULL) != IPC_ERROR;
}

#endif //Q1_SHARED_MEMORY_H
