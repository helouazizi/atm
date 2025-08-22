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
    // Check if user with the same username already exists
    for (int i = 0; i < SharedDataa->user_count; i++)
    {
        if (strcmp(SharedDataa->users[i].username, user->username) == 0)
        {
            // Overwrite the existing user's pid
            SharedDataa->users[i].pid = getpid();
            printf("Updated user '%s' with new PID %d\n", user->username, SharedDataa->users[i].pid);
            return;
        }
    }

    // If user not found, add a new user
    if (SharedDataa->user_count >= MAX_USERS)
    {
        printf("Cannot add more users. Limit reached.\n");
        return;
    }

    pid_t pid = getpid();
    SharedDataa->users[SharedDataa->user_count].pid = pid;
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

void cleanup_shared_memory()
{
    shm_unlink(SHM_NAME);
}
void *listen_for_notifications(void *arg)
{
    SharedData *SharedDataa = (SharedData *)arg;
    pid_t my_pid = getpid();

    while (1)
    {
        pthread_mutex_lock(&SharedDataa->mutex);
        if (SharedDataa->updated && SharedDataa->target_pid == my_pid)
        {
            printf("\n🔔 Notification: %s\n", SharedDataa->message);
            SharedDataa->updated = 0;
        }
        pthread_mutex_unlock(&SharedDataa->mutex);
        usleep(500000); // 0.5s delay
    }
    return NULL;
}
