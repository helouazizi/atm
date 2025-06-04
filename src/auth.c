#include <termios.h>
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
    printf("\n\n\n\n\n\t\t\t\tEnter the password to login:");
    scanf("%s", pass);

    // restore terminal
    if (tcsetattr(fileno(stdin), TCSANOW, &oflags) != 0)
    {
        perror("tcsetattr");
        return exit(1);
    }
};

void registerMenu(sqlite3 *db,char name[50], char password[50])
{
    struct User temp;
    int exists = 0;

    printf("\n\n\n\t\t\t\t   Bank Management System\n\t\t\t\t\t User Register:");

    printf("\n\n\n\n\n\t\t\t\tEnter a username: ");

    scanf("%s", name);

    // check username  existance

    printf("\n\n\n\n\n\t\t\t\tEnter a password: ");
    scanf("%s", password);

    // save to db

    printf("\n✔ User registered successfully!\n");
}
