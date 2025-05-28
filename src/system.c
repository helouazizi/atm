#include "header.h"
#include <stdio.h>      // FILE, fopen, fclose, fprintf, fscanf, printf, scanf
#include <stdlib.h>     // exit, system, malloc, free (if used)
#include <string.h>     // strcmp, strcpy

const char *RECORDS = "./data/records.txt";

int getAccountFromFile(FILE *ptr, char name[50], struct Record *r)
{
    return fscanf(ptr, "%d %d %s %d %d/%d/%d %s %d %lf %s",
                  &r->id,
		  &r->userId,
		  name,
                  &r->accountNbr,
                  &r->deposit.month,
                  &r->deposit.day,
                  &r->deposit.year,
                  r->country,
                  &r->phone,
                  &r->amount,
                  r->accountType) != EOF;
}

void saveAccountToFile(FILE *ptr, struct User u, struct Record r)
{
    fprintf(ptr, "%d %d %s %d %d/%d/%d %s %d %.2lf %s\n\n",
            r.id,
            u.id,
            u.name,
            r.accountNbr,
            r.deposit.month,
            r.deposit.day,
            r.deposit.year,
            r.country,
            r.phone,
            r.amount,
            r.accountType);
}


void stayOrReturn(int notGood, void f(struct User u), struct User u)
{
    int option;
    if (notGood == 0)
    {
        system("clear");
        printf("\n✖ Record not found!!\n");
    invalid:
        printf("\nEnter 0 to try again, 1 to return to main menu and 2 to exit:");
        scanf("%d", &option);
        if (option == 0)
            f(u);
        else if (option == 1)
            mainMenu(u);
        else if (option == 2)
            exit(0);
        else
        {
            printf("Insert a valid operation!\n");
            goto invalid;
        }
    }
    else
    {
        printf("\nEnter 1 to go to the main menu and 0 to exit:");
        scanf("%d", &option);
    }
    if (option == 1)
    {
        system("clear");
        mainMenu(u);
    }
    else
    {
        system("clear");
        exit(1);
    }
}

void success(struct User u)
{
    int option;
    printf("\n✔ Success!\n\n");
invalid:
    printf("Enter 1 to go to the main menu and 0 to exit!\n");
    scanf("%d", &option);
    system("clear");
    if (option == 1)
    {
        mainMenu(u);
    }
    else if (option == 0)
    {
        exit(1);
    }
    else
    {
        printf("Insert a valid operation!\n");
        goto invalid;
    }
}

void createNewAcc(struct User u)
{
    struct Record r;
    struct Record cr;
    char userName[50];
    FILE *pf = fopen(RECORDS, "a+");

noAccount:
    system("clear");
    printf("\t\t\t===== New record =====\n");

    printf("\nEnter today's date(mm/dd/yyyy):");
    scanf("%d/%d/%d", &r.deposit.month, &r.deposit.day, &r.deposit.year);
    printf("\nEnter the account number:");
    scanf("%d", &r.accountNbr);

    while (getAccountFromFile(pf, userName, &cr))
    {
        if (strcmp(userName, u.name) == 0 && cr.accountNbr == r.accountNbr)
        {
            printf("✖ This Account already exists for this user\n\n");
            goto noAccount;
        }
    }
    printf("\nEnter the country:");
    scanf("%s", r.country);
    printf("\nEnter the phone number:");
    scanf("%d", &r.phone);
    printf("\nEnter amount to deposit: $");
    scanf("%lf", &r.amount);
    printf("\nChoose the type of account:\n\t-> saving\n\t-> current\n\t-> fixed01(for 1 year)\n\t-> fixed02(for 2 years)\n\t-> fixed03(for 3 years)\n\n\tEnter your choice:");
    scanf("%s", r.accountType);

    saveAccountToFile(pf, u, r);

    fclose(pf);
    success(u);
}

void checkAllAccounts(struct User u)
{
    char userName[100];
    struct Record r;

    FILE *pf = fopen(RECORDS, "r");

    system("clear");
    printf("\t\t====== All accounts from user, %s =====\n\n", u.name);
    while (getAccountFromFile(pf, userName, &r))
    {
        if (strcmp(userName, u.name) == 0)
        {
            printf("_____________________\n");
            printf("\nAccount number:%d\nDeposit Date:%d/%d/%d \ncountry:%s \nPhone number:%d \nAmount deposited: $%.2f \nType Of Account:%s\n",
                   r.accountNbr,
                   r.deposit.day,
                   r.deposit.month,
                   r.deposit.year,
                   r.country,
                   r.phone,
                   r.amount,
                   r.accountType);
        }
    }
    fclose(pf);
    success(u);
}
void updateAccountInfo(struct User u) {
    FILE *fp = fopen(RECORDS, "r");
    FILE *temp = fopen("temp.txt", "w");
    struct Record r;
    char userName[50];
    int accNbr, found = 0;

    printf("\nEnter account number to update: ");
    scanf("%d", &accNbr);

    while (getAccountFromFile(fp, userName, &r)) {
        if (strcmp(userName, u.name) == 0 && r.accountNbr == accNbr) {
            printf("Updating country, phone, and amount...\n");
            printf("New country: ");
            scanf("%s", r.country);
            printf("New phone: ");
            scanf("%d", &r.phone);
            printf("New amount: ");
            scanf("%lf", &r.amount);
            found = 1;
        }
        saveAccountToFile(temp, u, r);
    }

    fclose(fp);
    fclose(temp);
    remove(RECORDS);
    rename("temp.txt", RECORDS);

    stayOrReturn(found, updateAccountInfo, u);
}
void checkAccount(struct User u) {
    FILE *fp = fopen(RECORDS, "r");
    struct Record r;
    char userName[50];
    int accNbr, found = 0;

    printf("Enter account number to view: ");
    scanf("%d", &accNbr);

    while (getAccountFromFile(fp, userName, &r)) {
        if (strcmp(userName, u.name) == 0 && r.accountNbr == accNbr) {
            printf("Account number: %d\nCountry: %s\nPhone: %d\nAmount: $%.2f\nType: %s\n",
                   r.accountNbr, r.country, r.phone, r.amount, r.accountType);
            found = 1;
            break;
        }
    }
    fclose(fp);
    stayOrReturn(found, checkAccount, u);
}
void makeTransaction(struct User u) {
    FILE *fp = fopen(RECORDS, "r");
    FILE *temp = fopen("temp.txt", "w");
    struct Record r;
    char userName[50];
    int accNbr, found = 0, option;
    double amt;

    printf("Enter account number: ");
    scanf("%d", &accNbr);

    while (getAccountFromFile(fp, userName, &r)) {
        if (strcmp(userName, u.name) == 0 && r.accountNbr == accNbr) {
            printf("Choose: 1) Deposit 2) Withdraw: ");
            scanf("%d", &option);
            printf("Enter amount: ");
            scanf("%lf", &amt);

            if (option == 1) r.amount += amt;
            else if (option == 2) {
                if (amt > r.amount) {
                    printf("✖ Insufficient funds.\n");
                    fclose(fp);
                    fclose(temp);
                    remove("temp.txt");
                    stayOrReturn(0, makeTransaction, u);
                    return;
                }
                r.amount -= amt;
            }
            found = 1;
        }
        saveAccountToFile(temp, u, r);
    }

    fclose(fp);
    fclose(temp);
    remove(RECORDS);
    rename("temp.txt", RECORDS);

    stayOrReturn(found, makeTransaction, u);
}
void removeAccount(struct User u) {
    FILE *fp = fopen(RECORDS, "r");
    FILE *temp = fopen("temp.txt", "w");
    struct Record r;
    char userName[50];
    int accNbr, found = 0;

    printf("Enter account number to remove: ");
    scanf("%d", &accNbr);

    while (getAccountFromFile(fp, userName, &r)) {
        if (strcmp(userName, u.name) == 0 && r.accountNbr == accNbr) {
            found = 1;
            continue;  // skip writing this record
        }
        saveAccountToFile(temp, u, r);
    }

    fclose(fp);
    fclose(temp);
    remove(RECORDS);
    rename("temp.txt", RECORDS);

    stayOrReturn(found, removeAccount, u);
}
void transferOwnership(struct User u) {
    FILE *fp = fopen(RECORDS, "r");
    FILE *temp = fopen("temp.txt", "w");
    struct Record r;
    char userName[50], newOwner[50];
    int accNbr, found = 0;

    printf("Enter account number to transfer: ");
    scanf("%d", &accNbr);
    printf("Enter the new owner's username: ");
    scanf("%s", newOwner);

    while (getAccountFromFile(fp, userName, &r)) {
        if (strcmp(userName, u.name) == 0 && r.accountNbr == accNbr) {
            found = 1;
            strcpy(userName, newOwner);
        }
        saveAccountToFile(temp, (struct User){.name = userName}, r);
    }

    fclose(fp);
    fclose(temp);
    remove(RECORDS);
    rename("temp.txt", RECORDS);

    stayOrReturn(found, transferOwnership, u);
}
