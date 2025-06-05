#include <termios.h>
#include <stdlib.h>
#include <string.h>
#include "header.h"

void loginMenu(char a[50], char pass[50])
{
    struct termios oflags, nflags;

    system("clear");
    printf("\n\n\n\t\t\t\t   Bank Management System\n\t\t\t\t\t User Login:");
    scanf("%s", a);
    // disabling echo
    tcgetattr(fileno(stdin), &oflags);
    nflags = oflags;
    nflags.c_lflag &= ~ECHO;
    nflags.c_lflag |= ECHONL;

    if (tcsetattr(fileno(stdin), TCSANOW, &nflags) != 0)
    {
        perror("tcsetattr");
        return exit(1);
    }
    printf("\n\t\t\t\t\t Enter the password to login:");
    scanf("%s", pass);

    // restore terminal
    if (tcsetattr(fileno(stdin), TCSANOW, &oflags) != 0)
    {
        perror("tcsetattr");
     exit(1);
    }
};

void registerMenu(sqlite3 *db,char name[50], char password[50])
{

    printf("\n\n\n\t\t\t\t   Bank Management System\n\t\t\t\t\t User Register:");

    printf("\n\n\n\n\n\t\t\t\tEnter a username: ");

    scanf("%s", name);

    // check username  existance

    printf("\n\n\n\n\n\t\t\t\tEnter a password: ");
    scanf("%s", password);

    // save to db

    printf("\n✔ User registered successfully!\n");
}
// // Check if username exists (for registration uniqueness)
int usernameExists(sqlite3 *db, const char *username)
{
    const char *sql = "SELECT 1 FROM users WHERE username = ?;";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK)
    {
        // fprintf(stderr, "Failed to prepare username exists statement\n");/
        return 1; // assume exists to prevent duplicates on error
    }
    sqlite3_bind_text(stmt, 1, username, -1, SQLITE_STATIC);
    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    return (rc == SQLITE_ROW);
}

// Register a new user - prevent duplicates
int registerUser(sqlite3 *db, struct User *user)
{
        if (usernameExists(db, user->username))
        {
            printf("Username '%s' already exists. Choose another.\n", user->username);
            return 0;
        }

        const char *sql = "INSERT INTO users (username, password) VALUES (?, ?);";
        sqlite3_stmt *stmt;
        int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
        if (rc != SQLITE_OK)
        {
            // fprintf(stderr, "Failed to prepare insert user statement\n");
            return 0;
        }

        sqlite3_bind_text(stmt, 1, user->username, -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, user->password, -1, SQLITE_STATIC);

        rc = sqlite3_step(stmt);
        sqlite3_finalize(stmt);

        return (rc == SQLITE_DONE);
}
// Authenticate user
int authenticateUser(sqlite3 *db, struct User *user)
{
    const char *sql = "SELECT password FROM users WHERE username = ?;";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK)
    {
        // fprintf(stderr, "Failed to prepare auth statement\n");
        return 0;
    }

    sqlite3_bind_text(stmt, 1, user->username, -1, SQLITE_STATIC);
    rc = sqlite3_step(stmt);

    if (rc == SQLITE_ROW)
    {
        const unsigned char *db_password = sqlite3_column_text(stmt, 0);
        int result = (strcmp((const char *)db_password, user->password) == 0);
        sqlite3_finalize(stmt);
        return result;
    }

    sqlite3_finalize(stmt);
    return 0;
}


// Remove an account owned by user
int removeAccount(sqlite3 *db, struct User *user, int accountNbr)
{
    const char *sql = "DELETE FROM records WHERE accountNbr = ? AND name = ?;";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "Failed to prepare delete account statement\n");
        return 0;
    }

    sqlite3_bind_int(stmt, 1, accountNbr);
    sqlite3_bind_text(stmt, 2, user->username, -1, SQLITE_STATIC);

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc == SQLITE_DONE)
    {
        printf("Account #%d removed successfully.\n", accountNbr);
        return 1;
    }
    else
    {
        printf("Failed to remove account.\n");
        return 0;
    }
}
