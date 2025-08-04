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


// ========================== ui functions ================================//
// ANSI color codes
#define RESET "\033[0m"
#define BOLD "\033[1m"
#define UNDERLINE "\033[4m"
#define RED "\033[31m"
#define GREEN "\033[32m"
#define CYAN "\033[36m"
#define MAGENTA "\033[35m"
#define YELLOW "\033[33m"

int getTerminalWidth();
void printCentered(const char *text);
void printSeparator(char ch);

void promptContinueOrExit(sqlite3 *db, struct User *usr);
// ========================== database functions ================================//

sqlite3 *openDatabase(const char *filename);
int createTables(sqlite3 *db, char *schemaFile);

//=========================== Authentication functions ==========================//
void loginMenu(char a[50], char pass[50]);
void createNewAcc(sqlite3 *db,struct User *user);
int usernameExists(sqlite3 *db, const char *username);
int check_credentials(sqlite3 *db, struct User *user);
int registerUser(sqlite3 *db, struct User *user);

//=========================== system functions ==========================//
int updateUserInfo(sqlite3 *db, const int *id, const char *field, const char *newValue);
void recordMenu(sqlite3 *db, struct User *user);
void updateAccountInfo(sqlite3 *db, struct User *u);
void checkAccountDetails(sqlite3 *db, struct User *user);
void listAccounts(sqlite3 *db, struct User *u);
void makeTransaction(sqlite3 *db, struct User *u);
void removeAccount(sqlite3 *db, struct User *u);
void transferOwnership(sqlite3 *db, struct User *u);


#endif
