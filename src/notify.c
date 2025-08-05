// src/notify.c
#include "header.h"
#include <sys/stat.h>
#include <string.h>

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