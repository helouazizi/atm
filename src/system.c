#include "header.h"
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#define MAX_ATTEMPTS 3

struct Date get_currentDate()
{
    time_t now = time(NULL);
    struct tm *local = localtime(&now);
    struct Date Today;
    Today.year = local->tm_year + 1900;
    Today.month = local->tm_mon + 1;
    Today.day = local->tm_mday;
    return Today;
}

double calculateInterest(const char *type, double amount)
{
    if (strcmp(type, "saving") == 0)
    {
        // Annual 7%, shown as monthly interest
        return (amount * 0.07) / 12;
    }
    else if (strcmp(type, "fixed01") == 0)
    {
        // Fixed 1-year interest: 4% for the whole year
        return amount * 0.04;
    }
    else if (strcmp(type, "fixed02") == 0)
    {
        // Fixed 2-year interest: 5% for the whole 2 years
        return amount * 0.05 * 2;
    }
    else if (strcmp(type, "fixed03") == 0)
    {
        // Fixed 3-year interest: 8% for the whole 3 years
        return amount * 0.08 * 3;
    }
    return 0.0;
}

static int accountNumberExists(sqlite3 *db, int accountNbr)
{
    const char *sql = "SELECT 1 FROM records WHERE accountNbr = ? LIMIT 1;";
    sqlite3_stmt *stmt;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK)
    {
        return 1;
    }

    sqlite3_bind_int(stmt, 1, accountNbr);

    int rc = sqlite3_step(stmt);
    int exists = (rc == SQLITE_ROW); /* got at least one row → it exists */

    sqlite3_finalize(stmt);
    return exists;
}

int checkAccount(sqlite3 *db, struct User *user, int accountNbr)
{

    const char *sql = "SELECT accountNbr FROM records WHERE owner = ? AND accountNbr = ?;";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK)
    {
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

int checkAccountType(sqlite3 *db, struct User *user, int accountNbr)
{
    const char *sql = "SELECT accountType FROM records WHERE owner = ? AND accountNbr = ?;";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK)
    {
        return 0;
    }

    sqlite3_bind_text(stmt, 1, user->username, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 2, accountNbr);

    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW)
    {
        const unsigned char *acctype = sqlite3_column_text(stmt, 0);
        if (acctype &&
            (strcmp((const char *)acctype, "saving") == 0 || strcmp((const char *)acctype, "current") == 0))
        {
            sqlite3_finalize(stmt);
            return 1;
        }
    }

    sqlite3_finalize(stmt);
    return 0;
}

int createNewRecord(sqlite3 *db, struct User *user, struct Record *record)
{
    const char *sql = "INSERT INTO records (user_id, owner, country, phone, accountType, accountNbr, amount, deposit, withdraw) "
                      "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?);";

    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK)
    {
        return 0;
    }

    char depositDate[11];
    char phoneStr[20];

    snprintf(depositDate, sizeof(depositDate), "%04d-%02d-%02d", record->deposit.year, record->deposit.month, record->deposit.day);
    snprintf(phoneStr, sizeof(phoneStr), "%d", record->phone);

    sqlite3_bind_int(stmt, 1, user->id);                            // user_id
    sqlite3_bind_text(stmt, 2, user->username, -1, SQLITE_STATIC);  // name
    sqlite3_bind_text(stmt, 3, record->country, -1, SQLITE_STATIC); // country

    sqlite3_bind_text(stmt, 4, phoneStr, -1, SQLITE_STATIC); // phone (as TEXT)

    sqlite3_bind_text(stmt, 5, record->accountType, -1, SQLITE_STATIC); // accountType
    sqlite3_bind_int(stmt, 6, record->accountNbr);                      // accountNbr
    sqlite3_bind_double(stmt, 7, record->amount);                       // amount
    sqlite3_bind_text(stmt, 8, depositDate, -1, SQLITE_STATIC);         // deposit
    sqlite3_bind_null(stmt, 9);                                         // withdraw

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE)
    {
        return 0;
    }
    return 101;
}

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
        return 0;
    }

    sqlite3_bind_text(stmt, 1, newValue, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 2, *id);

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    return rc == SQLITE_DONE;
}

void listAccounts(sqlite3 *db, struct User *user)
{
    const char *sql = "SELECT * FROM records WHERE owner = ?;";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);

    system("clear");
    printf(BOLD CYAN);
    printSeparator('=');
    printCentered(" YOUR ACCOUNTS ");
    printSeparator('=');
    printf(RESET);

    if (rc != SQLITE_OK)
    {
        fprintf(stderr, RED "❌ Failed to prepare statement: %s\n" RESET, sqlite3_errmsg(db));
        return;
    }

    sqlite3_bind_text(stmt, 1, user->username, -1, SQLITE_STATIC);

    int found = 0;

    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW)
    {
        found = 1;
        const unsigned char *name = sqlite3_column_text(stmt, 2);
        const unsigned char *country = sqlite3_column_text(stmt, 3);
        const unsigned char *phone = sqlite3_column_text(stmt, 4);
        const unsigned char *accTyp = sqlite3_column_text(stmt, 5);
        int accNbr = sqlite3_column_int(stmt, 6);
        double amount = sqlite3_column_double(stmt, 7);
        const unsigned char *deposit = sqlite3_column_text(stmt, 8);
        const unsigned char *withdraw = sqlite3_column_text(stmt, 9);

        printf("\n" BOLD GREEN "📄 Account #[%d]\n" RESET, accNbr);
        printf("👤 " BOLD "Name:" RESET "     %s\n", name);
        printf("🌍 " BOLD "Country:" RESET "  %s\n", country);
        printf("📞 " BOLD "Phone:" RESET "    %s\n", phone);
        printf("🏦 " BOLD "Type:" RESET "     %s\n", accTyp);
        printf("💰 " BOLD "Balance:" RESET "  %.2lf $\n", amount);
        printf("📥 " BOLD "Deposit Date:" RESET "  %s\n", deposit ? deposit : "N/A");
        printf("📤 " BOLD "Withdraw Date:" RESET " %s\n", withdraw ? withdraw : "N/A");
        printf(CYAN "----------------------------------------\n" RESET);
    }

    if (!found)
    {
        printf(YELLOW "⚠️  You have no accounts listed under your ownership.\n" RESET);
    }

    sqlite3_finalize(stmt);
    printf("\n");
    promptContinueOrExit(db, user);
    return;
}

void checkAccountDetails(sqlite3 *db, struct User *user)
{
    int accountNbr;
    system("clear");
    printf(BOLD CYAN);
    printSeparator('=');
    printCentered(" ACCOUNT DETAILS ");
    printSeparator('=');
    printf(RESET);

    printf("\nEnter the %saccount number%s to view: ", YELLOW, RESET);
    if (scanf("%d", &accountNbr) != 1)
    {
        printf(RED "❌ Invalid input. Please enter a valid number.\n" RESET);
        while (getchar() != '\n')
            ;
        sleep(2);
        return;
    }
    while (getchar() != '\n')
        ;

    if (accountNbr < 1)
    {
        printf(RED "❌ Account number must be greater than zero.\n" RESET);
        sleep(2);
        return;
    }

    if (checkAccount(db, user, accountNbr) == 0)
    {
        printf(RED "❌ Account not found under your ownership.\n" RESET);
        sleep(2);
        return;
    }

    const char *sql = "SELECT * FROM records WHERE accountNbr = ? AND owner = ?;";
    sqlite3_stmt *stmt;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK)
    {
        fprintf(stderr, RED "❌ Failed to prepare SQL statement.\n" RESET);
        return;
    }

    sqlite3_bind_int(stmt, 1, accountNbr);
    sqlite3_bind_text(stmt, 2, user->username, -1, SQLITE_STATIC);

    int rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW)
    {
        const unsigned char *name = sqlite3_column_text(stmt, 2);
        const unsigned char *country = sqlite3_column_text(stmt, 3);
        const unsigned char *phone = sqlite3_column_text(stmt, 4);
        const unsigned char *accType = sqlite3_column_text(stmt, 5);
        int accNbr = sqlite3_column_int(stmt, 6);
        double amount = sqlite3_column_double(stmt, 7);
        const unsigned char *depositDate = sqlite3_column_text(stmt, 8);

        printf(BOLD GREEN "\n📄 Account #[%d]\n" RESET, accNbr);
        printf("👤 " BOLD "Name:" RESET "    %s\n", name);
        printf("🌍 " BOLD "Country:" RESET " %s\n", country);
        printf("📞 " BOLD "Phone:" RESET "   %s\n", phone);
        printf("🏦 " BOLD "Type:" RESET "    %s\n", accType);
        printf("💰 " BOLD "Balance:" RESET " %.2lf$\n", amount);

        if (strcmp(accType, "current") == 0)
        {
            printf("ℹ️  Note: No interest applied for 'current' account type.\n");
        }
        else if (strcmp(accType, "saving") == 0)
        {
            double interest = calculateInterest(accType, amount);
            int year = 0, month = 0, day = 1;
            if (depositDate && sscanf(depositDate, "%d-%d-%d", &year, &month, &day) == 3)
            {
                printf("💸 Interest: You will get $%.2lf of interest on day %d of every month.\n", interest, day);
            }
        }
        else if (strcmp(accType, "fixed01") == 0 || strcmp(accType, "fixed02") == 0 || strcmp(accType, "fixed03") == 0)
        {
            double interest = calculateInterest(accType, amount);
            int year = 0, month = 0, day = 0;
            int duration = 0;

            if (strcmp(accType, "fixed01") == 0)
                duration = 1;
            else if (strcmp(accType, "fixed02") == 0)
                duration = 2;
            else if (strcmp(accType, "fixed03") == 0)
                duration = 3;

            if (depositDate && sscanf((const char *)depositDate, "%d-%d-%d", &year, &month, &day) == 3)
            {
                int maturityYear = year + duration;
                printf("💸 Interest: You will get $%.2lf on %02d/%02d/%04d.\n",
                       interest, day, month, maturityYear);
            }
            else
            {
                // fallback if depositDate is missing or invalid
                printf("💸 Interest: You will get $%.2lf at the end of the %d-year period.\n",
                       interest, duration);
            }
        }
    }
    else
    {
        printf(RED "\n⚠️  No account found with that number under your ownership.\n" RESET);
    }

    sqlite3_finalize(stmt);
    printf("\n");
    promptContinueOrExit(db, user);
    return;
}

void recordMenu(sqlite3 *db, struct User *user)
{
    system("clear");
    printf(BOLD CYAN);
    printSeparator('=');
    printCentered(" CREATE ACCOUNT ");
    printSeparator('=');
    printf(RESET);

    struct Record *r = malloc(sizeof(struct Record));
    if (!r)
    {
        return;
    }

    r->userId = user->id;
    strcpy(r->name, user->username);

    int attempts = 0;
    while (attempts < MAX_ATTEMPTS)
    {
        struct Date today = get_currentDate();
        printf(CYAN "\n\nEnter date (MM DD YYYY): " RESET);

        if (scanf("%d %d %d", &r->deposit.month, &r->deposit.day, &r->deposit.year) == 3 &&
            r->deposit.month >= 1 && r->deposit.month <= 12 &&
            r->deposit.day >= 1 && r->deposit.day <= 31 &&
            r->deposit.year >= 1900)
        {
            break;
        }
        else
        {
            printf(RED "❌ Invalid date. Try again.\n" RESET);
            attempts++;
            while (getchar() != '\n')
                ; // clear input
        }
        if (attempts == MAX_ATTEMPTS)
        {
            printf(RED "❌ Too many invalid attempts. Exiting...\n" RESET);
            sleep(1);
            exit(0);
        }
    }
    getchar(); // clear newline

    // Country
    attempts = 0;
    while (attempts < MAX_ATTEMPTS)
    {
        printf(CYAN "Enter country: " RESET);
        fgets(r->country, sizeof(r->country), stdin);
        r->country[strcspn(r->country, "\n")] = 0;
        if (strlen(r->country) > 0)
            break;

        printf(RED "❌ Country cannot be empty. Try again.\n" RESET);
        attempts++;
        if (attempts == MAX_ATTEMPTS)
        {
            printf(RED "❌ Too many invalid attempts. Exiting...\n" RESET);
            sleep(1);
            exit(0);
        }
    }

    // Phone
    attempts = 0;
    while (attempts < MAX_ATTEMPTS)
    {
        printf(CYAN "Enter phone number: " RESET);
        if (scanf("%d", &r->phone) == 1 && r->phone > 0)
            break;

        printf(RED "❌ Invalid phone number. Try again.\n" RESET);
        attempts++;
        while (getchar() != '\n')
            ;
        if (attempts == MAX_ATTEMPTS)
        {
            printf(RED "❌ Too many invalid attempts. Exiting...\n" RESET);
            sleep(1);
            exit(0);
        }
    }

    // Account Type
    attempts = 0;
    while (attempts < MAX_ATTEMPTS)
    {
        printf(CYAN "Enter account type (saving/current/fixed01/fixed02/fixed03): " RESET);
        scanf("%s", r->accountType);
        if (
            strcmp(r->accountType, "saving") == 0 ||
            strcmp(r->accountType, "current") == 0 ||
            strcmp(r->accountType, "fixed01") == 0 ||
            strcmp(r->accountType, "fixed02") == 0 ||
            strcmp(r->accountType, "fixed03") == 0)
            break;

        printf(RED "❌ Invalid account type. Try again.\n" RESET);
        attempts++;
        if (attempts == MAX_ATTEMPTS)
        {
            printf(RED "❌ Too many invalid attempts. Exiting...\n" RESET);
            sleep(1);
            exit(0);
        }
    }

    attempts = 0;
    while (attempts < MAX_ATTEMPTS)
    {
        printf(CYAN "Enter account number: " RESET);
        if (scanf("%d", &r->accountNbr) == 1 && r->accountNbr > 0)
        {
            /* Does it already exist? */
            if (!accountNumberExists(db, r->accountNbr))
            {
                break;
            }

            printf(RED "❌ Account number already exists. Choose another.\n" RESET);
        }
        else
        {
            printf(RED "❌ Invalid account number. Try again.\n" RESET);
        }

        attempts++;
        while (getchar() != '\n')
            ;

        if (attempts == MAX_ATTEMPTS)
        {
            printf(RED "❌ Too many invalid attempts. Exiting...\n" RESET);
            sleep(1);
            exit(0);
        }
    }

    // Amount
    attempts = 0;
    while (attempts < MAX_ATTEMPTS)
    {
        printf(CYAN "Enter amount to deposit: " RESET);
        if (scanf("%lf", &r->amount) == 1 && r->amount > 0)
            break;

        printf(RED "❌ Invalid amount. Try again.\n" RESET);
        attempts++;
        while (getchar() != '\n')
            ;
        if (attempts == MAX_ATTEMPTS)
        {
            printf(RED "❌ Too many invalid attempts. Exiting...\n" RESET);
            sleep(1);
            exit(0);
        }
    }

    int i = createNewRecord(db, user, r);
    if (i != SQLITE_DONE)
    {
        printf(RED "\n❌ Account not saved, please try later.\n" RESET);
        promptContinueOrExit(db, user);
    }
    free(r);

    printf(GREEN "\n✅ Account created successfully!\n" RESET);

    promptContinueOrExit(db, user);
    return;
}

void updateAccountInfo(sqlite3 *db, struct User *user)
{
    int accId, attempts = 3;
    char field[16];
    char newValue[64];

    system("clear");
    printf(BOLD CYAN);
    printSeparator('=');
    printCentered(" UPDATE ACCOUNT INFO ");
    printSeparator('=');
    printf(RESET);

    while (attempts--)
    {
        printf("\nEnter %saccount number%s to update: ", YELLOW, RESET);
        if (scanf("%d", &accId) == 1 && accId > 0)
        {
            while (getchar() != '\n')
                ;
            break;
        }
        else
        {
            printf(RED "❌ Invalid account number. Please enter a valid positive number.\n" RESET);
            while (getchar() != '\n')
                ;
        }
        if (attempts == 0)
        {
            printf(RED "\n❌ Too many invalid attempts.\n" RESET);
            sleep(1);
            exit(0);
        }
    }

    if (!checkAccount(db, user, accId))
    {
        printf(RED "\n❌ Account not found under your ownership.\n" RESET);
        promptContinueOrExit(db, user);
        return;
    }

    attempts = 3;
    while (attempts--)
    {
        printf("\nWhich field do you want to update? " BOLD "(phone/country): " RESET);
        if (fgets(field, sizeof(field), stdin) != NULL)
        {
            field[strcspn(field, "\n")] = 0;
            if (strcmp(field, "phone") == 0 || strcmp(field, "country") == 0)
                break;
        }
        printf(RED "❌ Invalid input. Only 'phone' or 'country' are allowed.\n" RESET);
        if (attempts == 0)
        {
            printf(RED "\n❌ Too many invalid attempts.\n" RESET);
            sleep(1);
            exit(0);
        }
    }

    attempts = 3;
    while (attempts--)
    {
        printf("Enter new value for %s%s%s: ", CYAN, field, RESET);
        if (fgets(newValue, sizeof(newValue), stdin) != NULL)
        {
            newValue[strcspn(newValue, "\n")] = 0;
            if (strlen(newValue) > 3)
                break;
        }
        printf(RED "❌ Value cannot be empty.\n" RESET);
        if (attempts == 0)
        {
            printf(RED "\n❌ Too many invalid attempts.\n" RESET);
            sleep(1);
            exit(0);
        }
    }

    if (updateUserInfo(db, &accId, field, newValue))
    {
        printf(GREEN "\n✅ Account info updated successfully.\n" RESET);
    }
    else
    {
        printf(RED "\n❌ Failed to update account info.\n" RESET);
    }

    promptContinueOrExit(db, user);
    return;
}

void removeAccount(sqlite3 *db, struct User *user)
{
    int accNbr;
    int attempts = 3;
    char confirm;

    system("clear");
    printf(BOLD CYAN);
    printSeparator('=');
    printCentered(" REMOVE AN ACCOUNT ");
    printSeparator('=');
    printf(RESET);

    // Ask for account number with validation
    while (attempts--)
    {
        printf(CYAN "\n\n [🔢] Enter account number to remove: " RESET);
        if (scanf("%d", &accNbr) == 1)
        {
            while (getchar() != '\n')
                ;
            break;
        }
        else
        {
            printf(RED "❌ Invalid input. Please enter a valid number.\n" RESET);
            while (getchar() != '\n')
                ;
        }

        if (attempts == 0)
        {
            printf(RED "\n❌ Too many invalid attempts.\n" RESET);
            sleep(1);
            exit(0);
        }
    }

    // Check account ownership
    if (!checkAccount(db, user, accNbr))
    {
        printf(RED "\n❌ Account not found under your ownership.\n" RESET);
        promptContinueOrExit(db, user);
        return;
    }

    // Ask for confirmation (y/n)
    attempts = 3;
    while (attempts--)
    {
        printf(YELLOW "\n ⚠️  Are you sure you want to remove account #%d? [y/n]: " RESET, accNbr);
        if (scanf(" %c", &confirm) == 1)
        {
            while (getchar() != '\n')
                ; // flush buffer
            confirm = tolower(confirm);
            if (confirm == 'y' || confirm == 'n')
                break;
        }
        printf(RED "❌ Invalid input. Please type 'y' or 'n'.\n" RESET);
        while (getchar() != '\n')
            ;

        if (attempts == 0)
        {
            printf(RED "\n❌ Too many invalid confirmation attempts.\n" RESET);
            sleep(1);
            exit(0);
        }
    }

    if (confirm == 'n')
    {
        printf(YELLOW "\nℹ️  Account removal cancelled.\n" RESET);
        promptContinueOrExit(db, user);
        return;
    }

    // Prepare and execute delete query
    const char *sql = "DELETE FROM records WHERE accountNbr = ? AND owner = ?;";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, RED "❌ Failed to prepare delete statement: %s\n" RESET, sqlite3_errmsg(db));
        return;
    }

    sqlite3_bind_int(stmt, 1, accNbr);
    sqlite3_bind_text(stmt, 2, user->username, -1, SQLITE_STATIC);

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc == SQLITE_DONE)
    {
        printf(GREEN "\n✅ Account #%d removed successfully.\n" RESET, accNbr);
    }
    else
    {
        printf(RED "\n❌ Failed to remove account. Please try again later.\n" RESET);
    }

    promptContinueOrExit(db, user);
}

void transferOwnership(sqlite3 *db, struct User *user)
{
    int accNbr;
    char newOwner[64];
    char confirm;
    int attempts = 3;

    system("clear");
    printf(BOLD CYAN);
    printSeparator('=');
    printCentered(" TRANSFER OWNERSHIP ");
    printSeparator('=');
    printf(RESET);

    while (attempts--)
    {
        printf(CYAN "\n\n [🔢] Enter account number to transfer: " RESET);
        if (scanf("%d", &accNbr) == 1)
        {
            while (getchar() != '\n')
                ;
            break;
        }
        else
        {
            printf(RED "❌ Invalid input. Please enter a valid number.\n" RESET);
            while (getchar() != '\n')
                ;
        }

        if (attempts == 0)
        {
            printf(RED "\n❌ Too many invalid attempts.\n" RESET);
            sleep(1);
            exit(0);
        }
    }

    if (!checkAccount(db, user, accNbr))
    {
        printf(RED "\n❌ Account not found under your ownership.\n" RESET);
        promptContinueOrExit(db, user);
        return;
    }

    attempts = 3;
    while (attempts--)
    {
        printf(CYAN " 👤 Enter new owner's username: " RESET);
        if (fgets(newOwner, sizeof(newOwner), stdin) != NULL)
        {
            newOwner[strcspn(newOwner, "\n")] = 0;
            if (strlen(newOwner) > 0)
                break;
        }
        printf(RED "❌ Invalid username. Please try again.\n" RESET);
        if (attempts == 0)
        {
            printf(RED "\n❌ Too many invalid attempts.\n" RESET);
            sleep(1);
            exit(0);
        }
    }

    attempts = 3;
    while (attempts--)
    {
        printf(YELLOW "⚠️  Confirm transfer to '%s'? [y/n]: " RESET, newOwner);
        if (scanf(" %c", &confirm) == 1)
        {
            while (getchar() != '\n')
                ;
            confirm = tolower(confirm);
            if (confirm == 'y' || confirm == 'n')
                break;
        }
        printf(RED "❌ Invalid input. Please type 'y' or 'n'.\n" RESET);
        while (getchar() != '\n')
            ;

        if (attempts == 0)
        {
            printf(RED "\n❌ Too many invalid confirmation attempts.\n" RESET);
            sleep(1);
            exit(0);
        }
    }

    if (confirm == 'n')
    {
        printf(YELLOW "\nℹ️  Ownership transfer cancelled.\n" RESET);
        promptContinueOrExit(db, user);
        return;
    }

    const char *sql = "UPDATE records SET owner = ? WHERE accountNbr = ? AND owner = ?;";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK)
    {
        return;
    }

    sqlite3_bind_text(stmt, 1, newOwner, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 2, accNbr);
    sqlite3_bind_text(stmt, 3, user->username, -1, SQLITE_STATIC);

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc == SQLITE_DONE)
    {
        printf(GREEN "\n✅ Ownership transferred to '%s' successfully.\n" RESET, newOwner);
    }
    else
    {
        printf(RED "\n❌ Failed to transfer ownership.\n" RESET);
    }

    promptContinueOrExit(db, user);
    return;
}

void makeTransaction(sqlite3 *db, struct User *user)
{
    int accNbr = -1, choice = -1;
    double amount = -1;
    int attempts = 3;

    system("clear");
    printf(BOLD CYAN);
    printSeparator('=');
    printCentered(" MAKE A TRANSACTION ");
    printSeparator('=');
    printf(RESET);

    // === Attempt to get valid account number ===
    while (attempts--)
    {
        printf(CYAN "\n\n [🔢] Enter account number: " RESET);
        if (scanf("%d", &accNbr) == 1)
        {
            while (getchar() != '\n')
                ; // clear input buffer
            break;
        }
        printf(RED "❌ Invalid account number input. Try again.\n" RESET);
        sleep(1);
        while (getchar() != '\n')
            ;
        if (attempts == 0)
            return;
    }

    // === Verify ownership ===
    if (!checkAccount(db, user, accNbr))
    {
        printf(RED "❌ Account not found under your ownership.\n" RESET);
        sleep(1);
        return;
    }

    // === Check account type (must not be 'fixed') ===
    if (!checkAccountType(db, user, accNbr))
    {
        printf(YELLOW "⚠️  Transactions are not allowed for 'fixed' accounts.\n" RESET);
        sleep(1);
        return;
    }

    // === Attempt to get valid transaction type ===
    attempts = 3;
    while (attempts--)
    {
        printf(CYAN " [💳]Enter %s1%s to deposit, %s2%s to withdraw: " RESET, GREEN, CYAN, GREEN, CYAN);
        if (scanf("%d", &choice) == 1 && (choice == 1 || choice == 2))
        {
            while (getchar() != '\n')
                ;
            break;
        }
        printf(RED "❌ Invalid transaction type. Try again.\n" RESET);
        sleep(1);
        while (getchar() != '\n')
            ;
        if (attempts == 0)
            return;
    }

    // === Attempt to get valid amount ===
    attempts = 3;
    while (attempts--)
    {
        printf(CYAN " 💵 Enter amount: " RESET);
        if (scanf("%lf", &amount) == 1 && amount > 0)
        {
            while (getchar() != '\n')
                ;
            break;
        }
        printf(RED "❌ Invalid amount. Must be greater than 0. Try again.\n" RESET);
        sleep(1);
        while (getchar() != '\n')
            ;
        if (attempts == 0)
            return;
    }

    // === SQL query selection ===
    const char *sql = (choice == 1)
                          ? "UPDATE records SET amount = amount + ? WHERE accountNbr = ? AND owner = ?;"
                          : "UPDATE records SET amount = amount - ? WHERE accountNbr = ? AND owner = ? AND amount >= ?;";

    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK)
    {
        return;
    }

    sqlite3_bind_double(stmt, 1, amount);
    sqlite3_bind_int(stmt, 2, accNbr);
    sqlite3_bind_text(stmt, 3, user->username, -1, SQLITE_STATIC);
    if (choice == 2)
        sqlite3_bind_double(stmt, 4, amount);

    rc = sqlite3_step(stmt);
    int changes = sqlite3_changes(db);
    sqlite3_finalize(stmt);

    // === Report result ===
    if (rc == SQLITE_DONE && changes > 0)
    {
        printf(GREEN "✅ %s successful! Amount: $%.2lf\n" RESET,
               (choice == 1) ? "Deposit" : "Withdrawal", amount);
    }
    else
    {
        if (choice == 2)
            printf(RED "❌ Withdrawal failed: Insufficient funds \n" RESET);
            
        else
            printf(RED "❌ Deposit failed.\n" RESET);
    }

    promptContinueOrExit(db, user);
    return;
}
