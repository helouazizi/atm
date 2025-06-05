#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <sqlite3.h>
#include "header.h"

// Disable terminal echo (for password input)
void disableEcho(struct termios *oldFlags)
{
    struct termios newFlags;
    tcgetattr(fileno(stdin), oldFlags);
    newFlags = *oldFlags;
    newFlags.c_lflag &= ~ECHO;
    newFlags.c_lflag |= ECHONL;
    tcsetattr(fileno(stdin), TCSANOW, &newFlags);
}

// Restore terminal settings
void restoreEcho(const struct termios *oldFlags)
{
    tcsetattr(fileno(stdin), TCSANOW, oldFlags);
}

// Show login prompt and collect credentials
void loginMenu(char username[50], char password[50])
{
    struct termios oldFlags;

    system("clear");
    printf("\n\n\n\t\t\t\tBank Management System");
    printf("\n\t\t\t\tEnter Username: ");
    scanf("%49s", username);

    printf("\n\t\t\t\tEnter Password: ");
    disableEcho(&oldFlags);
    scanf("%49s", password);
    restoreEcho(&oldFlags);
}

// Check if username already exists
int usernameExists(sqlite3 *db, const char *username)
{
    const char *sql = "SELECT 1 FROM users WHERE username = ?;";
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK)
        return 1; // assume exists to prevent duplicate

    sqlite3_bind_text(stmt, 1, username, -1, SQLITE_STATIC);
    int exists = (sqlite3_step(stmt) == SQLITE_ROW);
    sqlite3_finalize(stmt);
    return exists;
}

// Register a new user
int registerUser(sqlite3 *db, struct User *user)
{
    if (usernameExists(db, user->username))
    {
        printf("Username '%s' already exists. Choose another.\n", user->username);
        return 0;
    }

    const char *sql = "INSERT INTO users (username, password) VALUES (?, ?);";
    sqlite3_stmt *stmt;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK)
    {
        fprintf(stderr, "Error preparing insert: %s\n", sqlite3_errmsg(db));
        return 0;
    }

    sqlite3_bind_text(stmt, 1, user->username, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, user->password, -1, SQLITE_STATIC);

    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return rc == SQLITE_DONE;
}

// Create a new user account (register flow)
void createNewAcc(sqlite3 *db, struct User *user)
{
    system("clear");
    printf("\n\n\n\t\t\t\tBank Management System");
    printf("\n\t\t\t\t\t Register\n");

    while (1)
    {
        printf("\n\t\t\t\tEnter a Username: ");
        scanf("%49s", user->username);
        if (strlen(user->username) < 3 || strlen(user->username) > 15)
        {
            printf("\n❌ Please enter a valid username {3 -> 15} characters\n");
            continue;
        }

        if (!usernameExists(db, user->username))
            break;

        printf("\n❌ Username already exists. Try again.\n");
    }

    struct termios oldFlags;
    disableEcho(&oldFlags);
    while (1)
    {
        printf("\n\t\t\t\tEnter a Password: ");
        scanf("%49s", user->password);
        if (strlen(user->password) < 8 || strlen(user->password) > 15)
        {
            printf("\n❌ Please enter a valid password {8 -> 15} characters\n");
            continue;
        }
        else
        {
            break;
        }
    }

    restoreEcho(&oldFlags);
    if (registerUser(db, user))
    {
        printf("\n✔ User registered successfully!\n");
    }
    else
    {
        printf("\n❌ Failed to register user.\n");
    }
}

// Authenticate user
int authenticateUser(sqlite3 *db, struct User *user)
{
    const char *sql = "SELECT password FROM users WHERE username = ?;";
    sqlite3_stmt *stmt;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK)
    {
        fprintf(stderr, "Failed to prepare authentication query\n");
        return 0;
    }

    sqlite3_bind_text(stmt, 1, user->username, -1, SQLITE_STATIC);
    int rc = sqlite3_step(stmt);

    int authenticated = 0;

    if (rc == SQLITE_ROW)
    {
        const unsigned char *dbPassword = sqlite3_column_text(stmt, 0);
        authenticated = strcmp((const char *)dbPassword, user->password) == 0;
    }

    sqlite3_finalize(stmt);
    return authenticated;
}

// Remove an account owned by the user
int removeAccount(sqlite3 *db, struct User *user, int accountNbr)
{
    const char *sql = "DELETE FROM records WHERE accountNbr = ? AND name = ?;";
    sqlite3_stmt *stmt;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK)
    {
        fprintf(stderr, "Failed to prepare delete statement\n");
        return 0;
    }

    sqlite3_bind_int(stmt, 1, accountNbr);
    sqlite3_bind_text(stmt, 2, user->username, -1, SQLITE_STATIC);

    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc == SQLITE_DONE)
    {
        printf("✔ Account #%d removed successfully.\n", accountNbr);
        return 1;
    }
    else
    {
        printf("❌ Failed to remove account.\n");
        return 0;
    }
}
