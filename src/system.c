#include "header.h"
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>

#define MAX_ATTEMPTS 3
/* Return 1 if accountNbr is found in the records table, 0 otherwise */
static int accountNumberExists(sqlite3 *db, int accountNbr)
{
    const char *sql = "SELECT 1 FROM records WHERE accountNbr = ? LIMIT 1;";
    sqlite3_stmt *stmt;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK)
    {
        /* Preparation failed – be conservative and pretend it exists */
        fprintf(stderr, "SQLite error: %s\n", sqlite3_errmsg(db));
        return 1;
    }

    sqlite3_bind_int(stmt, 1, accountNbr);

    int rc = sqlite3_step(stmt);
    int exists = (rc == SQLITE_ROW); /* got at least one row → it exists */

    sqlite3_finalize(stmt);
    return exists;
}

// lets create a function that check if the acount is under the  current user is exist should accept db and user and accnumber
int checkAccount(sqlite3 *db, struct User *user, int accountNbr)
{
    // select the accnumber from db where the username  = user.usernme
    // open the db and check
    const char *sql = "SELECT accountNbr FROM records WHERE owner = ? AND accountNbr = ?;";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(db));
        return 0;
    }
    sqlite3_bind_text(stmt, 1, user->username, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 2, accountNbr);
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW)
    {
        int acc = sqlite3_column_int(stmt, 0);
        if (acc == accountNbr)
        {
            return 1;
        }
    }
    sqlite3_finalize(stmt);
    return 0;
}

// Create new record for a user
int createNewRecord(sqlite3 *db, struct User *user, struct Record *record)
{
    const char *sql = "INSERT INTO records (user_id, owner, country, phone, accountType, accountNbr, amount, deposit, withdraw) "
                      "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?);";

    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "❌ Failed to prepare statement: %s\n", sqlite3_errmsg(db));
        return 0;
    }

    char depositDate[11];
    char withdrawDate[11];

    snprintf(depositDate, sizeof(depositDate), "%04d-%02d-%02d",
             record->deposit.year, record->deposit.month, record->deposit.day);
    snprintf(withdrawDate, sizeof(withdrawDate), "%04d-%02d-%02d",
             record->withdraw.year, record->withdraw.month, record->withdraw.day);

    sqlite3_bind_int(stmt, 1, user->id);                            // user_id
    sqlite3_bind_text(stmt, 2, user->username, -1, SQLITE_STATIC);  // name
    sqlite3_bind_text(stmt, 3, record->country, -1, SQLITE_STATIC); // country

    char phoneStr[20];
    snprintf(phoneStr, sizeof(phoneStr), "%d", record->phone);
    sqlite3_bind_text(stmt, 4, phoneStr, -1, SQLITE_TRANSIENT); // phone (as TEXT)

    sqlite3_bind_text(stmt, 5, record->accountType, -1, SQLITE_STATIC); // accountType
    sqlite3_bind_int(stmt, 6, record->accountNbr);                      // accountNbr
    sqlite3_bind_double(stmt, 7, record->amount);                       // amount
    sqlite3_bind_text(stmt, 8, depositDate, -1, SQLITE_STATIC);         // deposit
    sqlite3_bind_null(stmt, 9);                                         // withdraw

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE)
    {
        fprintf(stderr, "❌ Failed to insert record: %s\n", sqlite3_errmsg(db));
        return 0;
    }

    printf("✅ Record successfully saved to database.\n");
    return 101;
}

// Update phone or country for user by acount id
int updateUserInfo(sqlite3 *db, const int *id, const char *field, const char *newValue)
{
    const char *sql = NULL;

    if (strcmp(field, "phone") == 0)
    {
        sql = "UPDATE records SET phone = ? WHERE id = ?;";
    }
    else if (strcmp(field, "country") == 0)
    {
        sql = "UPDATE records SET country = ? WHERE id = ?;";
    }
    else
    {
        printf("Can only update 'phone' or 'country' fields.\n");
        return 0;
    }

    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "❌ Failed to prepare statement: %s\n", sqlite3_errmsg(db));
        return 0;
    }

    sqlite3_bind_text(stmt, 1, newValue, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 2, *id);

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    return rc == SQLITE_DONE;
}

// List all accounts of a user
void listAccounts(sqlite3 *db, struct User *user)
{
    const char *sql = "SELECT * FROM records WHERE owner = ?;";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK)
    {
        printf("Failed to prepare select accounts statement\n");
        return;
    }

    sqlite3_bind_text(stmt, 1, user->username, -1, SQLITE_STATIC);

    printf("Your accounts:\n");
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW)
    {
        const unsigned char *name = sqlite3_column_text(stmt, 2);
        const unsigned char *country = sqlite3_column_text(stmt, 3);
        const unsigned char *phone = sqlite3_column_text(stmt, 4);
        const unsigned char *accTyp = sqlite3_column_text(stmt, 5);
        int accNbr = sqlite3_column_int(stmt, 6);
        double amount = sqlite3_column_int(stmt, 7);
        const unsigned char *desposit = sqlite3_column_text(stmt, 8);
        const unsigned char *withdraw = sqlite3_column_text(stmt, 9);

        printf("Account #[%d] %s, %s, %s, %s, %lf$\n", accNbr, name, country, phone, accTyp, amount);
    }

    sqlite3_finalize(stmt);
}

// Check specific account details with interest info
void checkAccountDetails(sqlite3 *db, struct User *user, int accountNbr)
{
    const char *sql = "SELECT * FROM records WHERE accountNbr = ? AND owner = ? ;";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK)
    {
        printf("Failed to prepare select account statement\n");
        return;
    }

    sqlite3_bind_int(stmt, 1, accountNbr);
    sqlite3_bind_text(stmt, 2, user->username, -1, SQLITE_STATIC);

    rc = sqlite3_step(stmt);

    if (rc == SQLITE_ROW)
    {

        const unsigned char *name = sqlite3_column_text(stmt, 2);
        const unsigned char *country = sqlite3_column_text(stmt, 3);
        const unsigned char *phone = sqlite3_column_text(stmt, 4);
        const unsigned char *accTyp = sqlite3_column_text(stmt, 5);
        int accNbr = sqlite3_column_int(stmt, 6);
        double amount = sqlite3_column_int(stmt, 7);
        printf("Account #[%d] %s, %s, %s, %s, %lf$\n", accNbr, name, country, phone, accTyp, amount);
        // displayInterest((const char *)accType, balance, (const char *)depositDate);
    }
    else
    {
        printf("No account found with that number under your ownership.\n");
    }

    sqlite3_finalize(stmt);
}

void recordMenu(sqlite3 *db, struct User *user)
{
    system("clear");
    printf("\n\n\n\t\t\t\tBank Management System");
    printf("\n\t\t\t\tCreate Account");

    struct Record *r = malloc(sizeof(struct Record));
    if (!r)
    {
        printf("Memory allocation failed.\n");
        return;
    }

    r->userId = user->id;
    strcpy(r->name, user->username);

    // Date
    int attempts = 0;
    while (attempts < MAX_ATTEMPTS)
    {
        printf("\nEnter date (MM DD YYYY): ");
        if (scanf("%d %d %d", &r->deposit.month, &r->deposit.day, &r->deposit.year) == 3 &&
            r->deposit.month >= 1 && r->deposit.month <= 12 &&
            r->deposit.day >= 1 && r->deposit.day <= 31 &&
            r->deposit.year >= 1900)
        {
            break;
        }
        else
        {
            printf("❌ Invalid date. Try again.\n");
            attempts++;
            while (getchar() != '\n')
                ; // clear input
        }
        if (attempts == MAX_ATTEMPTS)
        {
            printf("❌ Too many invalid attempts. Exiting...\n");
            exit(1);
        }
    }
    getchar(); // clear newline

    // Country
    attempts = 0;
    while (attempts < MAX_ATTEMPTS)
    {
        printf("Enter country: ");
        fgets(r->country, sizeof(r->country), stdin);
        r->country[strcspn(r->country, "\n")] = 0;
        if (strlen(r->country) > 0)
            break;

        printf("❌ Country cannot be empty. Try again.\n");
        attempts++;
        if (attempts == MAX_ATTEMPTS)
        {
            printf("❌ Too many invalid attempts. Exiting...\n");
            exit(1);
        }
    }

    // Phone
    attempts = 0;
    while (attempts < MAX_ATTEMPTS)
    {
        printf("Enter phone number: ");
        if (scanf("%d", &r->phone) == 1 && r->phone > 0)
            break;

        printf("❌ Invalid phone number. Try again.\n");
        attempts++;
        while (getchar() != '\n')
            ;
        if (attempts == MAX_ATTEMPTS)
        {
            printf("❌ Too many invalid attempts. Exiting...\n");
            exit(1);
        }
    }

    // Account Type
    attempts = 0;
    while (attempts < MAX_ATTEMPTS)
    {
        printf("Enter account type (savings/current/fixed01/fixed02/fixed03): ");
        scanf("%s", r->accountType);
        if (
            strcmp(r->accountType, "savings") == 0 ||
            strcmp(r->accountType, "current") == 0 ||
            strcmp(r->accountType, "fixed01") == 0 ||
            strcmp(r->accountType, "fixed02") == 0 ||
            strcmp(r->accountType, "fixed03") == 0)
            break;

        printf("❌ Invalid account type. Try again.\n");
        attempts++;
        if (attempts == MAX_ATTEMPTS)
        {
            printf("❌ Too many invalid attempts. Exiting...\n");
            exit(1);
        }
    }

    /* ── Account Number ───────────────────────────────────── */
    attempts = 0;
    while (attempts < MAX_ATTEMPTS)
    {
        printf("Enter account number: ");
        if (scanf("%d", &r->accountNbr) == 1 && r->accountNbr > 0)
        {
            /* Does it already exist? */
            if (!accountNumberExists(db, r->accountNbr))
            {
                break; /* ⇢ good, continue */
            }

            printf("❌ Account number already exists. Choose another.\n");
        }
        else
        {
            printf("❌ Invalid account number. Try again.\n");
        }

        attempts++;
        while (getchar() != '\n')
            ; /* clear leftover input */

        if (attempts == MAX_ATTEMPTS)
        {
            printf("❌ Too many invalid attempts. Exiting...\n");
            exit(1);
        }
    }

    // Amount
    attempts = 0;
    while (attempts < MAX_ATTEMPTS)
    {
        printf("Enter amount to deposit: ");
        if (scanf("%lf", &r->amount) == 1 && r->amount > 0)
            break;

        printf("❌ Invalid amount. Try again.\n");
        attempts++;
        while (getchar() != '\n')
            ;
        if (attempts == MAX_ATTEMPTS)
        {
            printf("❌ Too many invalid attempts. Exiting...\n");
            exit(1);
        }
    }

    int i = createNewRecord(db, user, r);
    if (i != SQLITE_DONE)
    {
        printf("\n Account not saved please try later\n");
        promptContinueOrExit(db, user);
    }

    free(r);
    printf("\n✅ Acccount created successfully!\n");
    promptContinueOrExit(db, user);
}

// Update account information (phone or country)
void updateAccountInfo(sqlite3 *db, struct User *user)
{
    int accId;
    char field[16];
    char newValue[64];

    printf("Enter account number to update: ");
    if (scanf("%d", &accId) != 1)
    {
        printf("Invalid input.\n");
        while (getchar() != '\n')
            ;
        return;
    }
    while (getchar() != '\n')
        ; // clear input
    if (accId < 1)
    {
        printf("Invalid account number.\n");
        return;
    }
    // check account id
    if (checkAccount(db, user, accId) == 0)
    {
        printf("Account not found under your informations.\n");
        return;
    }

    printf("Which field do you want to update? (phone/country): ");
    fgets(field, sizeof(field), stdin);
    field[strcspn(field, "\n")] = 0; // Remove newline

    // Extra validation (optional but helpful)
    if (strcmp(field, "phone") != 0 && strcmp(field, "country") != 0)
    {
        printf("Invalid field. Only 'phone' or 'country' are allowed.\n");
        return;
    }

    printf("Enter new value: ");
    fgets(newValue, sizeof(newValue), stdin);
    newValue[strcspn(newValue, "\n")] = 0;

    if (updateUserInfo(db, &accId, field, newValue))
    {
        printf("✅ Account info updated successfully.\n");
    }
    else
    {
        printf("❌ Failed to update account info.\n");
    }
}

// Remove an account by account number
void removeAccount(sqlite3 *db, struct User *user)
{
    int accNbr;
    printf("Enter account number to remove: ");
    if (scanf("%d", &accNbr) != 1)
    {
        printf("Invalid input.\n");
        while (getchar() != '\n')
            ;
        return;
    }
    // check account id
    if (checkAccount(db, user, accNbr) == 0)
    {
        printf("Account not found under your informations.\n");
        return;
    }

    const char *sql = "DELETE FROM records WHERE accountNbr = ? AND owner = ?;";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK)
    {
        printf("Failed to prepare delete statement.\n");
        return;
    }

    sqlite3_bind_int(stmt, 1, accNbr);
    sqlite3_bind_text(stmt, 2, user->username, -1, SQLITE_STATIC);

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc == SQLITE_DONE)
    {
        printf("✅ Account removed successfully.\n");
    }
    else
    {
        printf("❌ Failed to remove account.\n");
    }
}

// Transfer ownership of an account
void transferOwnership(sqlite3 *db, struct User *user)
{
    int accNbr;
    char newOwner[64];
    printf("Enter account number to transfer: ");
    if (scanf("%d", &accNbr) != 1)
    {
        printf("Invalid input.\n");
        while (getchar() != '\n')
            ;
        return;
    }
    while (getchar() != '\n')
        ; // clear input

    // check account id
    if (checkAccount(db, user, accNbr) == 0)
    {
        printf("Account not found under your informations.\n");
        return;
    }

    printf("Enter new owner's username: ");
    fgets(newOwner, sizeof(newOwner), stdin);
    newOwner[strcspn(newOwner, "\n")] = 0;

    const char *sql = "UPDATE records SET owner = ? WHERE accountNbr = ? AND owner = ?;";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK)
    {
        printf("Failed to prepare transfer statement.\n");
        return;
    }

    sqlite3_bind_text(stmt, 1, newOwner, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 2, accNbr);
    sqlite3_bind_text(stmt, 3, user->username, -1, SQLITE_STATIC);

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc == SQLITE_DONE)
    {
        printf("✅ Ownership transferred successfully.\n");
    }
    else
    {
        printf("❌ Failed to transfer ownership.\n");
    }
}

// Make a transaction (deposit or withdraw)
void makeTransaction(sqlite3 *db, struct User *user)
{
    int accNbr;
    double amount;
    int choice;

    printf("Enter account number: ");
    if (scanf("%d", &accNbr) != 1)
    {
        printf("Invalid input.\n");
        while (getchar() != '\n')
            ;
        return;
    }

    if (checkAccount(db, user, accNbr) == 0)
    {
        printf("Account not found under your informations.\n");
        return;
    }

    printf("Enter 1 to deposit, 2 to withdraw: ");
    if (scanf("%d", &choice) != 1 || (choice != 1 && choice != 2))
    {
        printf("Invalid choice.\n");
        while (getchar() != '\n')
            ;
        return;
    }

    printf("Enter amount: ");
    if (scanf("%lf", &amount) != 1 || amount <= 0)
    {
        printf("Invalid amount.\n");
        while (getchar() != '\n')
            ;
        return;
    }

    const char *sql = (choice == 1)
                          ? "UPDATE records SET amount = amount + ? WHERE accountNbr = ? AND owner = ?;"
                          : "UPDATE records SET amount = amount - ? WHERE accountNbr = ? AND owner = ? AND amount >= ?;";

    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK)
    {
        printf("❌ Failed to prepare transaction statement.\n");
        return;
    }

    sqlite3_bind_double(stmt, 1, amount);
    sqlite3_bind_int(stmt, 2, accNbr);
    sqlite3_bind_text(stmt, 3, user->username, -1, SQLITE_STATIC);
    if (choice == 2)
    {
        sqlite3_bind_double(stmt, 4, amount); // for amount >= ?
    }

    rc = sqlite3_step(stmt);
    int changes = sqlite3_changes(db);
    sqlite3_finalize(stmt);

    if (rc == SQLITE_DONE && changes > 0)
    {
        printf("✅ Transaction successful.\n");
    }
    else
    {
        if (choice == 2)
        {
            printf("❌ Withdrawal failed: Insufficient funds.\n");
        }
        else
        {
            printf("❌ Transaction failed.\n");
        }
    }
}
