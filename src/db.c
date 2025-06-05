#include <stdio.h>
#include <string.h>
#include <sqlite3.h>
#include <header.h>

// Open DB connection
sqlite3 *openDatabase(const char *filename)
{
    sqlite3 *db;
    int rc = sqlite3_open(filename, &db);
    if (rc)
    {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        return NULL;
    }
    return db;
}

int createTables(sqlite3 *db)
{
    const char *Users =
        "CREATE TABLE IF NOT EXISTS users ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "username TEXT UNIQUE NOT NULL, "
        "password TEXT NOT NULL"
        ");";

    const char *Records =
        "CREATE TABLE IF NOT EXISTS records ("
        "id INTEGER PRIMARY KEY, "
        "user_id INTEGER NOT NULL, "
        "name TEXT, "
        "country TEXT, "
        "phone TEXT, "
        "accountType TEXT, "
        "accountNbr INTEGER, "
        "amount REAL, "
        "deposit TEXT, "
        "withdraw TEXT, "
        "FOREIGN KEY (userId) REFERENCES users(id) ON DELETE CASCADE"
        ");";

    char *errMsg = NULL;

    // Execute create users table
    int rc = sqlite3_exec(db, Users, NULL, NULL, &errMsg);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "SQL error creating users table: %s\n", errMsg);
        sqlite3_free(errMsg);
        return 0;
    }

    // Execute create accounts table
    rc = sqlite3_exec(db, Records, NULL, NULL, &errMsg);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "SQL error creating accounts table: %s\n", errMsg);
        sqlite3_free(errMsg);
        return 0;
    }

    return 1; // Success
}



// // Calculate and display interest for given account type and deposit date
// void displayInterest(const char *accountType, double balance, const char *depositDate)
// {
//     double interestRate = 0.0;
//     if (strcmp(accountType, "savings") == 0)
//         interestRate = 0.07;
//     else if (strcmp(accountType, "fixed01") == 0)
//         interestRate = 0.04;
//     else if (strcmp(accountType, "fixed02") == 0)
//         interestRate = 0.05;
//     else if (strcmp(accountType, "fixed03") == 0)
//         interestRate = 0.08;

//     if (strcmp(accountType, "current") == 0)
//     {
//         printf("You will not get interests because the account is of type current.\n");
//         return;
//     }

//     if (interestRate > 0.0)
//     {
//         double interestMonthly = balance * interestRate / 12.0;
//         // Extract day from depositDate (expected format dd/mm/yyyy or yyyy-mm-dd)
//         int day = 1;
//         if (strlen(depositDate) >= 2)
//         {
//             // Try to parse first 2 chars as day
//             char dayStr[3] = {depositDate[0], depositDate[1], 0};
//             day = atoi(dayStr);
//             if (day <= 0 || day > 31)
//                 day = 1;
//         }
//         printf("You will get $%.2f as interest on day %d of every month.\n", interestMonthly, day);
//     }
//     else
//     {
//         printf("Account type '%s' is unknown for interest calculation.\n", accountType);
//     }
// }

// // Make a transaction: deposit or withdraw (positive or negative amount)
// int makeTransaction(sqlite3 *db, struct User *user, int accountNbr, double amount)
// {
//     // Check account type first
//     const char *sqlCheckType = "SELECT accountType, balance FROM accounts WHERE accountNbr = ? AND ownerUsername = ?;";
//     sqlite3_stmt *stmt;
//     int rc = sqlite3_prepare_v2(db, sqlCheckType, -1, &stmt, NULL);
//     if (rc != SQLITE_OK)
//     {
//         fprintf(stderr, "Failed to prepare select account for transaction\n");
//         return 0;
//     }

//     sqlite3_bind_int(stmt, 1, accountNbr);
//     sqlite3_bind_text(stmt, 2, user->username, -1, SQLITE_STATIC);

//     rc = sqlite3_step(stmt);
//     if (rc != SQLITE_ROW)
//     {
//         printf("Account not found or not owned by you.\n");
//         sqlite3_finalize(stmt);
//         return 0;
//     }

//     const unsigned char *accType = sqlite3_column_text(stmt, 0);
//     double currentBalance = sqlite3_column_double(stmt, 1);
//     sqlite3_finalize(stmt);

//     // Reject transactions on fixed accounts
//     if (strncmp((const char *)accType, "fixed", 5) == 0)
//     {
//         printf("Transactions are not allowed on fixed accounts (%s).\n", accType);
//         return 0;
//     }

//     double newBalance = currentBalance + amount;
//     if (newBalance < 0)
//     {
//         printf("Insufficient funds for this transaction.\n");
//         return 0;
//     }

//     // Update balance
//     const char *sqlUpdate = "UPDATE accounts SET balance = ? WHERE accountNbr = ? AND ownerUsername = ?;";
//     rc = sqlite3_prepare_v2(db, sqlUpdate, -1, &stmt, NULL);
//     if (rc != SQLITE_OK)
//     {
//         fprintf(stderr, "Failed to prepare update balance statement\n");
//         return 0;
//     }

//     sqlite3_bind_double(stmt, 1, newBalance);
//     sqlite3_bind_int(stmt, 2, accountNbr);
//     sqlite3_bind_text(stmt, 3, user->username, -1, SQLITE_STATIC);

//     rc = sqlite3_step(stmt);
//     sqlite3_finalize(stmt);

//     if (rc == SQLITE_DONE)
//     {
//         printf("Transaction successful. New balance: %.2f\n", newBalance);
//         return 1;
//     }
//     else
//     {
//         printf("Transaction failed.\n");
//         return 0;
//     }
// }

// // Remove an account owned by user
// int removeAccount(sqlite3 *db, struct User *user, int accountNbr)
// {
//     const char *sql = "DELETE FROM accounts WHERE accountNbr = ? AND ownerUsername = ?;";
//     sqlite3_stmt *stmt;
//     int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
//     if (rc != SQLITE_OK)
//     {
//         fprintf(stderr, "Failed to prepare delete account statement\n");
//         return 0;
//     }

//     sqlite3_bind_int(stmt, 1, accountNbr);
//     sqlite3_bind_text(stmt, 2, user->username, -1, SQLITE_STATIC);

//     rc = sqlite3_step(stmt);
//     sqlite3_finalize(stmt);

//     if (rc == SQLITE_DONE)
//     {
//         printf("Account #%d removed successfully.\n", accountNbr);
//         return 1;
//     }
//     else
//     {
//         printf("Failed to remove account.\n");
//         return 0;
//     }
// }

// // Transfer ownership of an account to another user
// int transferOwnership(sqlite3 *db, struct User *user, int accountNbr, const char *newOwner)
// {
//     // Check new owner exists
//     if (!usernameExists(db, newOwner))
//     {
//         printf("New owner username '%s' does not exist.\n", newOwner);
//         return 0;
//     }

//     // Check current ownership
//     const char *sqlCheck = "SELECT 1 FROM accounts WHERE accountNbr = ? AND ownerUsername = ?;";
//     sqlite3_stmt *stmt;
//     int rc = sqlite3_prepare_v2(db, sqlCheck, -1, &stmt, NULL);
//     if (rc != SQLITE_OK)
//     {
//         fprintf(stderr, "Failed to prepare ownership check\n");
//         return 0;
//     }
//     sqlite3_bind_int(stmt, 1, accountNbr);
//     sqlite3_bind_text(stmt, 2, user->username, -1, SQLITE_STATIC);
//     rc = sqlite3_step(stmt);
//     sqlite3_finalize(stmt);

//     if (rc != SQLITE_ROW)
//     {
//         printf("Account #%d is not owned by you.\n", accountNbr);
//         return 0;
//     }

//     // Update ownerUsername
//     const char *sqlUpdate = "UPDATE accounts SET ownerUsername = ? WHERE accountNbr = ?;";
//     rc = sqlite3_prepare_v2(db, sqlUpdate, -1, &stmt, NULL);
//     if (rc != SQLITE_OK)
//     {
//         fprintf(stderr, "Failed to prepare ownership update\n");
//         return 0;
//     }

//     sqlite3_bind_text(stmt, 1, newOwner, -1, SQLITE_STATIC);
//     sqlite3_bind_int(stmt, 2, accountNbr);

//     rc = sqlite3_step(stmt);
//     sqlite3_finalize(stmt);

//     if (rc == SQLITE_DONE)
//     {
//         printf("Ownership of account #%d transferred to '%s'.\n", accountNbr, newOwner);
//         return 1;
//     }
//     else
//     {
//         printf("Failed to transfer ownership.\n");
//         return 0;
//     }
// }

// // Example usage inside a function that drives menu
// void menu(sqlite3 *db, struct User *user)
// {
//     int choice;
//     while (1)
//     {
//         printf("\nMenu:\n");
//         printf("1. Register\n");
//         printf("2. Update user info\n");
//         printf("3. Create new account\n");
//         printf("4. List accounts\n");
//         printf("5. Check account details\n");
//         printf("6. Make transaction\n");
//         printf("7. Remove account\n");
//         printf("8. Transfer ownership\n");
//         printf("9. Exit\n");
//         printf("Enter your choice: ");
//         scanf("%d", &choice);

//         if (choice == 9) break;

//         if (choice == 1)
//         {
//             // registration (user input example)
//             struct User newUser;
//             printf("Enter username: "); scanf("%s", newUser.username);
//             printf("Enter password: "); scanf("%s", newUser.password);
//             printf("Enter phone: "); scanf("%s", newUser.phone);
//             printf("Enter country: "); scanf("%s", newUser.country);

//             if (registerUser(db, &newUser))
//                 printf("Registration successful.\n");
//             else
//                 printf("Registration failed.\n");
//         }
//         else if (choice == 2)
//         {
//             char field[16], value[64];
//             printf("Enter field to update (phone/country): ");
//             scanf("%s", field);
//             printf("Enter new value: ");
//             scanf("%s", value);

//             if (updateUserInfo(db, user->username, field, value))
//                 printf("User info updated successfully.\n");
//             else
//                 printf("Failed to update user info.\n");
//         }
//         else if (choice == 3)
//         {
//             char accType[16];
//             printf("Enter account type (current, savings, fixed01, fixed02, fixed03): ");
//             scanf("%s", accType);
//             if (createNewAccount(db, user, accType))
//                 printf("Account created.\n");
//             else
//                 printf("Failed to create account.\n");
//         }
//         else if (choice == 4)
//         {
//             listAccounts(db, user);
//         }
//         else if (choice == 5)
//         {
//             int accNbr;
//             printf("Enter account number: ");
//             scanf("%d", &accNbr);
//             checkAccountDetails(db, user, accNbr);
//         }
//         else if (choice == 6)
//         {
//             int accNbr;
//             double amount;
//             printf("Enter account number: ");
//             scanf("%d", &accNbr);
//             printf("Enter transaction amount (+deposit / -withdrawal): ");
//             scanf("%lf", &amount);
//             makeTransaction(db, user, accNbr, amount);
//         }
//         else if (choice == 7)
//         {
//             int accNbr;
//             printf("Enter account number to remove: ");
//             scanf("%d", &accNbr);
//             removeAccount(db, user, accNbr);
//         }
//         else if (choice == 8)
//         {
//             int accNbr;
//             char newOwner[32];
//             printf("Enter account number to transfer: ");
//             scanf("%d", &accNbr);
//             printf("Enter new owner username: ");
//             scanf("%s", newOwner);
//             transferOwnership(db, user, accNbr, newOwner);
//         }
//         else
//         {
//             printf("Invalid choice.\n");
//         }
//     }
// }
