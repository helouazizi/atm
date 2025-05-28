#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "header.h"
#include <sqlite3.h>


// void mainMenu(sqlite3 *db, struct User *u)
// {
//     int option;
//     system("clear");
//     printf("\n\n\t\t======= ATM =======\n\n");
//     printf("\n\t\t-->> Feel free to choose one of the options below <<--\n");
//     printf("\n\t\t[1]- Create a new account\n");
//     printf("\n\t\t[2]- Update account information\n");
//     printf("\n\t\t[3]- Check accounts\n");
//     printf("\n\t\t[4]- Check list of owned account\n");
//     printf("\n\t\t[5]- Make Transaction\n");
//     printf("\n\t\t[6]- Remove existing account\n");
//     printf("\n\t\t[7]- Transfer ownership\n");
//     printf("\n\t\t[8]- Exit\n");
//     printf("\n\t\tYour choice: ");
//     scanf("%d", &option);

//     switch (option)
//     {
//     case 1:
//         createNewAcc(db, u);
//         break;
//     case 2:
//         updateAccountInfo(db, u);
//         break;
//     case 3:
//         checkAccountDetails(db, u);
//         break;
//     case 4:
//         listAccounts(db, u);
//         break;
//     case 5:
//         makeTransaction(db, u);
//         break;
//     case 6:
//         removeAccount(db, u);
//         break;
//     case 7:
//         transferOwnership(db, u);
//         break;
//     case 8:
//         sqlite3_close(db);  // Close DB before exit
//         exit(0);
//         break;
//     default:
//         printf("Invalid operation!\n");
//     }
// }

// void initMenu(sqlite3 *db, struct User *u)
// {
//     int r = 0;
//     int option;
//     system("clear");
//     printf("\n\n\t\t======= ATM =======\n");
//     printf("\n\t\t-->> Feel free to login / register :\n");
//     printf("\n\t\t[1]- Login\n");
//     printf("\n\t\t[2]- Register\n");
//     printf("\n\t\t[3]- Exit\n");
//     printf("\n\t\tYour choice: ");

//     while (!r)
//     {
//         scanf("%d", &option);
//         switch (option)
//         {
//         case 1:
//             loginMenu(u->username, u->password);
//             if (loadUserFromDB(db, u->username, u)) // You must pass db here
//             {
//                 if (strcmp(u->password, getPassword(*u)) == 0)
//                 {
//                     printf("\n\nPassword Match!\n");
//                 }
//                 else
//                 {
//                     printf("\nWrong password!\n");
//                     exit(1);
//                 }
//             }
//             else
//             {
//                 printf("\nUser not found!\n");
//                 exit(1);
//             }
//             r = 1;
//             break;
//         case 2:
//             registerMenu(u->username, u->password);
//             saveUserToDB(db, u);  // Also pass db here
//             r = 1;
//             break;
//         case 3:
//             sqlite3_close(db); // Close DB before exit
//             exit(0);
//             break;
//         default:
//             printf("Insert a valid operation!\n");
//         }
//     }
// }

int main() {
    sqlite3 *db = openDatabase("./data/db.db");
    if (!db) {
        return 1; // error opening DB
    }

    if (!createTables(db)) {
        printf("Failed to create tables.\n");
        sqlite3_close(db);
        return 1;
    }

    printf("Tables created successfully.\n");

    sqlite3_close(db);
    return 0;
}

