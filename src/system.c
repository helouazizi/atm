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
    sqlite3_bind_int(stmt, 1, user->id);                                // userId
    sqlite3_bind_text(stmt, 2, record->name, -1, SQLITE_STATIC);        // name
    sqlite3_bind_text(stmt, 3, record->country, -1, SQLITE_STATIC);     // country
    sqlite3_bind_int(stmt, 4, record->phone);                           // phone
    sqlite3_bind_text(stmt, 5, record->accountType, -1, SQLITE_STATIC); // accountType
    sqlite3_bind_int(stmt, 6, record->accountNbr);                      // accountNbr
    sqlite3_bind_double(stmt, 7, record->amount);                       // amount
    // sqlite3_bind_text(stmt, 8, record->deposit, -1, SQLITE_STATIC);     // deposit (as TEXT)
    // sqlite3_bind_text(stmt, 9, record->withdraw, -1, SQLITE_STATIC);    // withdraw (as TEXT)

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
