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
    printf(BOLD CYAN);
    printSeparator('=');
    printCentered(" LOGIN ");
    printSeparator('=');
    printf(RESET);

    printf(MAGENTA BOLD "\nEnter your credentials:\n\n" RESET);
    printf("  %sUsername:%s", CYAN, RESET);
    scanf("%49s", username);
    printf("  %sPassword:%s", CYAN, RESET);
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
    /* 1. Reject duplicate usernames */
    if (usernameExists(db, user->username))
    {
        printf("Username '%s' already exists. Choose another.\n", user->username);
        return 0;
    }

    /* 2. Prepare the INSERT */
    const char *sql =
        "INSERT INTO users (username, password) VALUES (?, ?);";
    /* If you’re on SQLite ≥ 3.35 you could do:
       "INSERT INTO users (username, password) VALUES (?, ?)
        RETURNING id;"  and read the id from sqlite3_column_… */
    sqlite3_stmt *stmt;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK)
    {
        fprintf(stderr, "Error preparing insert: %s\n", sqlite3_errmsg(db));
        return 0;
    }

    /* 3. Bind parameters */
    sqlite3_bind_text(stmt, 1, user->username, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, user->password, -1, SQLITE_TRANSIENT);

    /* 4. Execute */
    int rc = sqlite3_step(stmt);

    /* 5. If it worked, grab the auto‑generated rowid */
    if (rc == SQLITE_DONE)
    {
        /* sqlite3_last_insert_rowid returns a 64‑bit value.       */
        /* Cast only if you _know_ your ids fit into 32 bits.      */
        sqlite3_int64 newId = sqlite3_last_insert_rowid(db);
        user->id = (int)newId; /* or keep it as sqlite3_int64   */
    }

    /* 6. Clean up */
    sqlite3_finalize(stmt);
    return rc == SQLITE_DONE;
}

// Create a new user account (register flow)
void register_user(sqlite3 *db, struct User *user)
{
    system("clear");
    printf(BOLD CYAN);
    printSeparator('=');
    printCentered(" REGISTER ");
    printSeparator('=');
    printf(RESET);

    printf(MAGENTA BOLD "\nEnter your credentials:\n\n" RESET);

    int attempts = 0;
    while (attempts < 3)
    {
        printf("  %sUsername:%s ", CYAN, RESET);
        scanf("%49s", user->username);
        while (getchar() != '\n' && getchar() != EOF)
            ;

        if (strlen(user->username) < 3 || strlen(user->username) > 15)
        {
            printf(RED BOLD "\n❌ Invalid username! " RESET);
            printf(RED "Username must be between 3 and 15 characters.\n\n" RESET);
            attempts++;
            continue;
        }

        if (usernameExists(db, user->username))
        {
            printf(RED BOLD "\n❌ Username already exists! " RESET);
            printf(RED "Please try a different one.\n\n" RESET);
            attempts++;
            continue;
        }

        break; // Valid username
    }

    if (attempts == 3)
    {
        printf(RED BOLD "\n❌ Too many invalid attempts. Exiting...\n\n" RESET);
        sleep(2);
        exit(0);
    }

    struct termios oldFlags;
    disableEcho(&oldFlags);

    attempts = 0;
    while (attempts < 3)
    {
        printf("  %sPassword:%s ", CYAN, RESET);
        scanf("%49s", user->password);
        while (getchar() != '\n' && getchar() != EOF)
            ;

        if (strlen(user->password) < 8 || strlen(user->password) > 15)
        {
            printf(RED BOLD "\n❌ Invalid password! " RESET);
            printf(RED "Password must be between 8 and 15 characters.\n\n" RESET);
            attempts++;
            continue;
        }

        break; // Valid password
    }

    restoreEcho(&oldFlags);

    if (attempts == 3)
    {
        printf(RED BOLD "\n❌ Too many invalid  attempts. Exiting...\n\n" RESET);
        sleep(2);
        exit(0);
    }

    if (registerUser(db, user))
    {

        printf(GREEN BOLD "\n✅ Successfully created your account!\n\n" RESET);
        sleep(2);
    }
    else
    {
        printf(RED BOLD "\n❌ Failed to create your account. Please try again later.\n\n" RESET);
        sleep(2);
        exit(0);
    }
}

int check_credentials(sqlite3 *db, struct User *user)
{
    const char *sql = "SELECT id, password FROM users WHERE username = ?;";
    sqlite3_stmt *stmt;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK)
    {
        return 0;
    }

    sqlite3_bind_text(stmt, 1, user->username, -1, SQLITE_TRANSIENT);

    int rc = sqlite3_step(stmt);
    int authenticated = 0;

    if (rc == SQLITE_ROW)
    {
        const unsigned char *dbPassword = sqlite3_column_text(stmt, 1);
        int id = sqlite3_column_int(stmt, 0);

        if (strcmp((const char *)dbPassword, user->password) == 0)
        {
            authenticated = 1;
            user->id = id;
        }
    }

    sqlite3_finalize(stmt);
    return authenticated;
}
