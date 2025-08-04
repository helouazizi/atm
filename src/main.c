#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>
#include "header.h"

void mainMenu(sqlite3 *db, struct User *u)
{
    int option;

    while (1) {
        system("clear");
        printf(BOLD CYAN);
        printSeparator('=');
        printCentered("ATM MAIN MENU");
        printSeparator('=');
        printf(RESET);

        printf(MAGENTA BOLD "\nChoose an option:\n\n" RESET);
        printf("  %s[1]%s - Create a new account\n", CYAN, RESET);
        printf("  %s[2]%s - Update account information\n", CYAN, RESET);
        printf("  %s[3]%s - Check accounts\n", CYAN, RESET);
        printf("  %s[4]%s - List owned accounts\n", CYAN, RESET);
        printf("  %s[5]%s - Make transaction\n", CYAN, RESET);
        printf("  %s[6]%s - Remove account\n", CYAN, RESET);
        printf("  %s[7]%s - Transfer ownership\n", CYAN, RESET);
        printf("  %s[8]%s - Exit\n\n", CYAN, RESET);

        printf(BOLD "Your choice: " RESET);
        if (scanf(" %d", &option) != 1) {
            while (getchar() != '\n'); // clear input buffer
            printf(RED "❌ Invalid input! Please enter a number between 1 and 8.\n" RESET);
            sleep(1);
            continue;
        }

        switch (option)
        {
            case 1:
                recordMenu(db, u);
                break;
            case 2:
                updateAccountInfo(db, u);
                break;
            case 3:
                checkAccountDetails(db, u);
                break;
            case 4:
                listAccounts(db, u);
                break;
            case 5:
                makeTransaction(db, u);
                break;
            case 6:
                removeAccount(db, u);
                break;
            case 7:
                transferOwnership(db, u);
                break;
            case 8:
                sqlite3_close(db);
                printf(GREEN "\nThank you for using the ATM. Goodbye!\n" RESET);
                exit(0);
            default:
                printf(RED "❌ Invalid option! Please try again.\n" RESET);
                sleep(1);
                continue;
        }

        // After an action, prompt to continue or exit
        promptContinueOrExit(db, u);
    }
}

void initMenu(sqlite3 *db, struct User *u)
{
    int option;

    while (1)
    {
        system("clear");
        printf(BOLD CYAN);
        printSeparator('=');
        printCentered("ATM LOGIN / REGISTER");
        printSeparator('=');
        printf(RESET);

        printf(MAGENTA BOLD "\nPlease choose an option:\n\n" RESET);
        printf("  %s[1]%s - Login\n", CYAN, RESET);
        printf("  %s[2]%s - Register\n", CYAN, RESET);
        printf("  %s[3]%s - Exit\n\n", CYAN, RESET);

        printf(BOLD "Your choice: " RESET);

        if (scanf(" %d", &option) != 1) {
            while (getchar() != '\n');
            printf(RED "❌ Invalid input! Please enter 1, 2, or 3.\n" RESET);
            sleep(1);
            continue;
        }

        switch (option)
        {
            case 1:
                loginMenu(u->username, u->password);
                if (check_credentials(db, u) != 0)
                {
                    printf(GREEN "\nLogged in successfully.\n" RESET);
                    sleep(1);
                    return; 
                }
                else
                {
                    printf(RED "\nInvalid Credentials! Try again.\n" RESET);
                    sleep(2);
                    continue;
                }
            case 2:
                register_user(db, u);
                return;
            case 3:
                sqlite3_close(db);
                printf(GREEN "Goodbye!\n" RESET);
                exit(0);
            default:
                printf(RED "❌ Insert a valid operation!\n" RESET);
                sleep(1);
        }
    }
}

void promptContinueOrExit(sqlite3 *db, struct User *usr)
{
    int choice;
    int attempts = 0;

    while (attempts < 3)
    {
        printf(BOLD "\nEnter %s1%s to return to the main menu or %s0%s to exit: " RESET, GREEN, RESET, RED, RESET);
        if (scanf(" %d", &choice) == 1 && (choice == 0 || choice == 1))
        {
            if (choice == 1)
            {
                return;  // return control to mainMenu loop
            }
            else
            {
                printf(GREEN "Exiting the program. Goodbye!\n" RESET);
                sqlite3_close(db);
                exit(0);
            }
        }
        else
        {
            printf(RED "❌ Invalid option. Please enter 1 or 0.\n" RESET);
            attempts++;
            while (getchar() != '\n') ; // clear input buffer
        }
    }

    printf(RED "❌ Too many invalid attempts. Exiting...\n" RESET);
    sqlite3_close(db);
    exit(1);
}

int main()
{
    sqlite3 *db = openDatabase("./data/atm.db");
    if (!db)
    {
        return 1;
    }

    if (!createTables(db, "./data/schema.sql"))
    {
        sqlite3_close(db);
        return 1;
    }

    struct User *user = malloc(sizeof(struct User));
    if (!user) {
        sqlite3_close(db);
        return 1;
    }
    memset(user, 0, sizeof(struct User));

    initMenu(db, user);
    mainMenu(db, user);

    sqlite3_close(db);
    free(user);

    return 0;
}
