// src/auth.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <sqlite3.h>
#include "header.h"

void disableEcho(struct termios *oldFlags)
{
    struct termios newFlags;
    tcgetattr(fileno(stdin), oldFlags);
    newFlags = *oldFlags;
    newFlags.c_lflag &= ~ECHO;
    newFlags.c_lflag |= ECHONL;
    tcsetattr(fileno(stdin), TCSANOW, &newFlags);
}

void restoreEcho(const struct termios *oldFlags)
{
    tcsetattr(fileno(stdin), TCSANOW, oldFlags);
}

void login(sqlite3 *db, struct User *user)
{
    struct termios oldFlags;
    int attempts = 0;

    system("clear");
    printf(BOLD CYAN);
    printSeparator('=');
    printCentered(" LOGIN ");
    printSeparator('=');
    printf(RESET);

    printf(MAGENTA BOLD "\nEnter your credentials:\n\n" RESET);
    while (attempts < 3)
    {

        printf("  %sUsername:%s ", CYAN, RESET);
        scanf("%49s", user->username);
        while (getchar() != '\n' && getchar() != EOF)
            ;

        disableEcho(&oldFlags);
        printf("  %sPassword:%s ", CYAN, RESET);
        scanf("%49s", user->password);
        while (getchar() != '\n' && getchar() != EOF)
            ;
        restoreEcho(&oldFlags);

        if (check_credentials(db, user) != 0)
        {
            printf(GREEN "\n✅ Logged in successfully.\n\n" RESET);
            sleep(1);
            return;
        }
        else
        {
            printf(RED "\n❌ Invalid credentials! Try again.\n\n" RESET);
            attempts++;
        }
    }

    printf(RED BOLD "\n❌ Too many failed login attempts. Exiting.\n\n" RESET);
    sleep(2);
    exit(0);
}

int usernameExists(sqlite3 *db, const char *username)
{
    const char *sql = "SELECT 1 FROM users WHERE username = ?;";
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK)

        sqlite3_bind_text(stmt, 1, username, -1, SQLITE_STATIC);
    int exists = (sqlite3_step(stmt) == SQLITE_ROW);
    sqlite3_finalize(stmt);
    return exists;
}

int registerUser(sqlite3 *db, struct User *user)
{
    if (usernameExists(db, user->username))
    {
        printf("Username '%s' already exists. Choose another.\n", user->username);
        return 0;
    }

    const char *sql =
        "INSERT INTO users (username, password) VALUES (?, ?);";

    sqlite3_stmt *stmt;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK)
    {
        fprintf(stderr, "Error preparing insert: %s\n", sqlite3_errmsg(db));
        return 0;
    }

    sqlite3_bind_text(stmt, 1, user->username, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, user->password, -1, SQLITE_TRANSIENT);

    int rc = sqlite3_step(stmt);

    if (rc == SQLITE_DONE)
    {

        sqlite3_int64 newId = sqlite3_last_insert_rowid(db);
        user->id = (int)newId;
    }

    sqlite3_finalize(stmt);
    return rc == SQLITE_DONE;
}

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

        break; 
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

        break; 
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
