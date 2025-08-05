// learn/main.c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#define SHM_NAME "/atm_notify"
#define MAX_MSG 256
#define MAX_USERS 10

typedef struct
{
    pthread_mutex_t mutex;
    char message[MAX_MSG];
    int updated;
    Sender *sender;
    Reciver *reciver;
} SharedData;

typedef struct
{
    pid_t sender_pid;
    char username[50];

} Sender;
typedef struct
{
    pid_t reciver_pid;
    char username[50];

} Reciver;

SharedData *shared;

void *listen_for_notifications(void *arg)
{
    pid_t my_pid = getpid();

    while (1)
    {
        pthread_mutex_lock(&shared->mutex);
        if (shared->updated && shared->reciver->reciver_pid == my_pid)

        {
            printf("\n🔔 Notification: %s\n", shared->message);
            shared->updated = 0;
        }
        pthread_mutex_unlock(&shared->mutex);
        usleep(500000); // 0.5s delay
    }
    return NULL;
}

int main()
{
    int shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1)
    {
        perror("shm_open");
        exit(1);
    }

    if (ftruncate(shm_fd, sizeof(SharedData)) == -1)
    {
        perror("ftruncate");
        exit(1);
    }

    shared = mmap(NULL, sizeof(SharedData), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shared == MAP_FAILED)
    {
        perror("mmap");
        exit(1);
    }

    // Initialize mutex only once
    static int initialized = 0;
    if (!initialized)
    {
        pthread_mutexattr_t attr;
        pthread_mutexattr_init(&attr);
        pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
        pthread_mutex_init(&shared->mutex, &attr);

        // Clear shared memory fields
        pthread_mutex_lock(&shared->mutex);
        shared->updated = 0;
        shared->sender_pid = 0;
        shared->message[0] = '\0';
        pthread_mutex_unlock(&shared->mutex);

        initialized = 1;
    }

    // Start listener thread
    pthread_t tid;
    pthread_create(&tid, NULL, listen_for_notifications, NULL);

    // ATM actions loop
    char input[MAX_MSG];
    while (1)
    {
        printf("1. Register User\n2. Exit\nChoice: ");
        fflush(stdout);
        if (fgets(input, sizeof(input), stdin) == NULL)
            break;

        int choice = atoi(input);
        if (choice == 1)
        {
            printf("Enter username: ");
            fflush(stdout);
            if (fgets(input, sizeof(input), stdin) == NULL)
                break;

            input[strcspn(input, "\n")] = 0;

            pthread_mutex_lock(&shared->mutex);
            snprintf(shared->message, MAX_MSG, "New user registered: %s", input);
            shared->sender_pid = getpid();
            shared->updated = 1;
            pthread_mutex_unlock(&shared->mutex);
        }
        else
        {
            break;
        }
    }

    return 0;
}
