// src/notify.c
#include "header.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

void save_user_pid(struct User *user, SharedData *SharedDataa)
{
    if (SharedDataa->user_count >= MAX_USERS)
    {
        printf("Cannot add more users. Limit reached.\n");
        return;
    }

    SharedDataa->users[SharedDataa->user_count].pid = getpid();
    strcpy(SharedDataa->users[SharedDataa->user_count].username, user->username);
    SharedDataa->user_count++;
}

SharedData *init_shared_memory(void)
{
    int shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1)
    {
        perror("shm_open failed");
        return NULL;
    }

    if (ftruncate(shm_fd, sizeof(SharedData)) == -1)
    {
        perror("ftruncate failed");
        close(shm_fd);
        return NULL;
    }

    SharedData *shared = mmap(NULL, sizeof(SharedData),
                              PROT_READ | PROT_WRITE,
                              MAP_SHARED, shm_fd, 0);
    if (shared == MAP_FAILED)
    {
        perror("mmap failed");
        close(shm_fd);
        return NULL;
    }

    // Optional: simple guard to reinitialize if value is invalid
    if (shared->user_count > MAX_USERS || shared->user_count < 0)
    {
        shared->user_count = 0;
    }

    return shared;
}