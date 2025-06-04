#ifndef HEADER_H
#define HEADER_H

#include <stdio.h>
#include<sqlite3.h> 

// Structs
struct Date {
    int month, day, year;
};

struct Record {
    int id;
    int userId;
    char name[100];
    char country[100];
    int phone;
    char accountType[10];
    int accountNbr;
    double amount;
    struct Date deposit;
    struct Date withdraw;
};

struct User {
    int id;
    char username[50];
    char password[50];
};


// ========================== database functions ================================//

sqlite3 *openDatabase(const char *filename);
int createTables(sqlite3 *db);

//=========================== Authentication functions ==========================//
void loginMenu(char a[50], char pass[50]);
void registerMenu(sqlite3 *db,char name[50], char password[50]);
int usernameExists(sqlite3 *db, const char *username);
int authenticateUser(sqlite3 *db, struct User *user);
int registerUser(sqlite3 *db, struct User *user);

//=========================== system functions ==========================//
int updateUserInfo(sqlite3 *db, const int *id, const char *field, const char *newValue);


// void mainMenu(sqlite3 *db, struct User *user);

// int createNewAccount(sqlite3 *db, struct User *user);
// void listAccounts(sqlite3 *db, struct User *user);
// void checkAccountDetails(sqlite3 *db, struct User *user, int accountNbr);
// int updateAccountInfo(sqlite3 *db, struct User *user, int accountNbr, double newBalance);
// int makeTransaction(sqlite3 *db, struct User *user, int accountNbr, double amount);
// int removeAccount(sqlite3 *db, struct User *user, int accountNbr);
// int transferOwnership(sqlite3 *db, struct User *user, int accountNbr, const char *newOwnerUsername);

// Helpers (if still relevant, or adapt to DB if needed)
// void stayOrReturn(int notGood, void (*f)(sqlite3 *, struct User *), sqlite3 *db, struct User *user);
// void success(struct User *user);

#endif
