#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "header.h"
#include <sqlite3.h>

void mainMenu(sqlite3 *db, struct User *u)
{
    int option;
    system("clear");
    printf("\n\n\t\t======= ATM =======\n\n");
    printf("\n\t\t-->> Feel free to choose one of the options below <<--\n");
    printf("\n\t\t[1]- Create a new account\n");
    printf("\n\t\t[2]- Update account information\n");
    printf("\n\t\t[3]- Check accounts\n");
    printf("\n\t\t[4]- Check list of owned account\n");
    printf("\n\t\t[5]- Make Transaction\n");
    printf("\n\t\t[6]- Remove existing account\n");
    printf("\n\t\t[7]- Transfer ownership\n");
    printf("\n\t\t[8]- Exit\n");
    printf("\n\t\tYour choice: ");
    scanf("%d", &option);

    switch (option)
    {
    case 1:
        // add record
        recordMenu(db, u);
        break;
    case 2:
        // updateAccountInfo(db, u);
        break;
    case 3:
        // checkAccountDetails(db, u);
        break;
    case 4:
        // listAccounts(db, u);
        break;
    case 5:
        // makeTransaction(db, u);
        break;
    case 6:
        // removeAccount(db, u);
        break;
    case 7:
        // transferOwnership(db, u);
        break;
    case 8:
        sqlite3_close(db); // Close DB before exit
        exit(0);
        break;
    default:
        printf("Invalid operation!\n");
    }
}

void initMenu(sqlite3 *db, struct User *u)
{
    int r = 0;
    int option;
    system("clear");
    printf("\n\n\t\t======= ATM =======\n");
    printf("\n\t\t-->> Feel free to login / register :\n");
    printf("\n\t\t[1]- Login\n");
    printf("\n\t\t[2]- Register\n");
    printf("\n\t\t[3]- Exit\n");
    printf("\n\t\tYour choice: ");

    while (!r)
    {
        scanf("%d", &option);
        switch (option)
        {
        case 1:
            loginMenu(u->username, u->password);
            int n = authenticateUser(db, u);
            if (n != 0)
            {
                printf("\n\nPassword Match!\n");
            }
            else
            {
                printf("\nWrong password!\n");
                exit(1);
            }

            r = 1;
            break;
        case 2:
            createNewAcc(db, u);
            r = 1;
            break;
        case 3:
            sqlite3_close(db); // Close DB before exit
            exit(0);
            break;
        default:
            printf("Insert a valid operation!\n");
        }
    }
}

void promptContinueOrExit(sqlite3 *db, struct User *usr)
{
    int choice;
    int attempts = 0;

    while (attempts < 3)
    {
        printf("\n\nEnter 1 to return to the main menu or 0 to exit: ");
        if (scanf("%d", &choice) == 1 && (choice == 0 || choice == 1))
        {
            if (choice == 1)
            {
                mainMenu(db, usr);
                return;
            }
            else
            {
                printf("Exiting the program. Goodbye!\n");
                exit(0);
            }
        }
        else
        {
            printf("❌ Invalid input. Please enter 1 or 0.\n");
            attempts++;
            while (getchar() != '\n')
                ; // clear input buffer
        }
    }

    printf("❌ Too many invalid attempts. Exiting...\n");
    exit(1);
}



int main()
{
    sqlite3 *db = openDatabase("./data/atm.db");
    if (!db)
    {
        printf("Failed to to open  db.\n");
        return 1; // error opening DB
    }

    if (!createTables(db, "./data/schema.sql"))
    {
        printf("Failed to create tables.\n");
        sqlite3_close(db);
        return 1;
    }

    printf("Tables created successfully.\n");

    struct User *user = malloc(sizeof(struct User));

    initMenu(db, user);
    mainMenu(db, user);

    sqlite3_close(db);
    return 0;
}
