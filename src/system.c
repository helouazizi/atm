#include "header.h"

// Create new record for a user
int createNewAccount(sqlite3 *db, struct User *user, struct Record *record)
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

        printf("Account #[%d] %s, %s, %s, %s, %d, %lf,%s,%s\n", accId, name, country, phone, accTyp, accNbr, amount,desposit,withdraw);
    }

    sqlite3_finalize(stmt);
}



// Check specific account details with interest info
void checkAccountDetails(sqlite3 *db, struct User *user, int accountNbr)
{
    const char *sql = "SELECT * FROM records WHERE accountNbr = ? ;";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK)
    {
        printf("Failed to prepare select account statement\n");
        return;
    }

    sqlite3_bind_int(stmt, 1, accountNbr);

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

        printf("Account #[%d] %s, %s, %s, %s, %d, %lf,%s,%s\n", accId, name, country, phone, accTyp, accNbr, amount,desposit,withdraw);
        // displayInterest((const char *)accType, balance, (const char *)depositDate);
    }
    else
    {
        printf("No account found with that number under your ownership.\n");
    }

    sqlite3_finalize(stmt);
}
