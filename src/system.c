#include "header.h"
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>

#define MAX_ATTEMPTS 3
// Create new record for a user
int createNewRecord(sqlite3 *db, struct User *user, struct Record *record)
{
    // Default balance 0, and depositDate is current date (for simplicity, pass NULL or set externally)
    const char *sql = "INSERT INTO records (user_id,name, country, phone, accountType,accountNbr,amount,deposit,withdraw) VALUES (?,?,?,?,?,?,?,?,?);";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "Failed to prepare insert account statement\n");
        return 0;
    }

    // Bind parameters
    sqlite3_bind_int(stmt, 1, user->id);                                  // userId
    sqlite3_bind_text(stmt, 2, record->name, -1, SQLITE_STATIC);          // name
    sqlite3_bind_text(stmt, 3, record->country, -1, SQLITE_STATIC);       // country
    sqlite3_bind_int(stmt, 4, record->phone);                             // phone
    sqlite3_bind_text(stmt, 5, record->accountType, -1, SQLITE_STATIC);   // accountType
    sqlite3_bind_int(stmt, 6, record->accountNbr);                        // accountNbr
    sqlite3_bind_double(stmt, 7, record->amount);                         // amount
    sqlite3_bind_text(stmt, 8, record->deposit.year, -1, SQLITE_STATIC);  // deposit (as TEXT)
    sqlite3_bind_text(stmt, 9, record->withdraw.year, -1, SQLITE_STATIC); // withdraw (as TEXT)

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    return (rc == SQLITE_DONE);
}

// Update phone or country for user by acount id
int updateUserInfo(sqlite3 *db, const int *id, const char *field, const char *newValue)
{
    if (strcmp(field, "phone") != 0 && strcmp(field, "country") != 0)
    {
        printf("Can only update 'phone' or 'country' fields.\n");
        return 0;
    }

    char sql[128];
    snprintf(sql, sizeof(sql), "UPDATE records SET %s = ? WHERE id = ?;", field);

    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "Failed to prepare update user info statement\n");
        return 0;
    }

    sqlite3_bind_text(stmt, 1, newValue, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, id, -1, SQLITE_STATIC);

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    return (rc == SQLITE_DONE);
}

// List all accounts of a user
void listAccounts(sqlite3 *db, struct User *user)
{
    const char *sql = "SELECT * FROM records WHERE name = ?;";
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
        int accId = sqlite3_column_int(stmt, 0);
        const unsigned char *name = sqlite3_column_text(stmt, 2);
        const unsigned char *country = sqlite3_column_text(stmt, 3);
        const unsigned char *phone = sqlite3_column_text(stmt, 4);
        const unsigned char *accTyp = sqlite3_column_text(stmt, 5);
        int accNbr = sqlite3_column_int(stmt, 6);
        double amount = sqlite3_column_int(stmt, 7);
        const unsigned char *desposit = sqlite3_column_text(stmt, 8);
        const unsigned char *withdraw = sqlite3_column_text(stmt, 9);

        printf("Account #[%d] %s, %s, %s, %s, %d, %lf,%s,%s\n", accId, name, country, phone, accTyp, accNbr, amount, desposit, withdraw);
    }

    sqlite3_finalize(stmt);
}

// Check specific account details with interest info
void checkAccountDetails(sqlite3 *db, struct User *user, int accountNbr)
{
    const char *sql = "SELECT * FROM records WHERE accountNbr = ? AND name = ? ;";
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
        int accId = sqlite3_column_int(stmt, 0);
        const unsigned char *name = sqlite3_column_text(stmt, 2);
        const unsigned char *country = sqlite3_column_text(stmt, 3);
        const unsigned char *phone = sqlite3_column_text(stmt, 4);
        const unsigned char *accTyp = sqlite3_column_text(stmt, 5);
        int accNbr = sqlite3_column_int(stmt, 6);
        double amount = sqlite3_column_int(stmt, 7);
        const unsigned char *desposit = sqlite3_column_text(stmt, 8);
        const unsigned char *withdraw = sqlite3_column_text(stmt, 9);

        printf("Account #[%d] %s, %s, %s, %s, %d, %lf,%s,%s\n", accId, name, country, phone, accTyp, accNbr, amount, desposit, withdraw);
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
        printf("\nEnter date (MM/DD/YYYY): ");
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

    // Account Number
    attempts = 0;
    while (attempts < MAX_ATTEMPTS)
    {
        printf("Enter account number: ");
        if (scanf("%d", &r->accountNbr) == 1 && r->accountNbr > 0)
            break;

        printf("❌ Invalid account number. Try again.\n");
        attempts++;
        while (getchar() != '\n')
            ;
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
        printf("\n✅ Account not saved please try later\n");
        promptContinueOrExit(db, user);
    }

    free(r);
    printf("\n✅ Acccount created successfully!\n");
    promptContinueOrExit(db, user);

    // Optional: Print summary
    // printf("\n--- Record Summary ---\n");
    // printf("User ID: %d\n", r->userId);
    // printf("Username: %s\n", r->name);
    // printf("Deposit Date: %02d/%02d/%d\n", r->deposit.month, r->deposit.day, r->deposit.year);
    // printf("Country: %s\n", r->country);
    // printf("Phone Number: %d\n", r->phone);
    // printf("Account Type: %s\n", r->accountType);
    // printf("Account Number: %d\n", r->accountNbr);
    // printf("Deposit Amount: %.2lf\n", r->amount);
}
