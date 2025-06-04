// #include <termios.h>
#include "header.h"


// void loginMenu(char a[50], char pass[50])
// {
//     struct termios oflags, nflags;

//     system("clear");
//     printf("\n\n\n\t\t\t\t   Bank Management System\n\t\t\t\t\t User Login:");
//     scanf("%s", a);

//     // disabling echo
//     tcgetattr(fileno(stdin), &oflags);
//     nflags = oflags;
//     nflags.c_lflag &= ~ECHO;
//     nflags.c_lflag |= ECHONL;

//     if (tcsetattr(fileno(stdin), TCSANOW, &nflags) != 0)
//     {
//         perror("tcsetattr");
//         return exit(1);
//     }
//     printf("\n\n\n\n\n\t\t\t\tEnter the password to login:");
//     scanf("%s", pass);

//     // restore terminal
//     if (tcsetattr(fileno(stdin), TCSANOW, &oflags) != 0)
//     {
//         perror("tcsetattr");
//         return exit(1);
//     }
// };

// const char *getPassword(struct User u)
// {
//     FILE *fp;
//     struct User userChecker;

//     if ((fp = fopen("./data/users.txt", "r")) == NULL)
//     {
//         printf("Error! opening file");
//         exit(1);
//     }

//     while (fscanf(fp, "%s %s", userChecker.name, userChecker.password) != EOF)
//     {
//         if (strcmp(userChecker.name, u.name) == 0)
//         {
//             fclose(fp);
//             char *buff = userChecker.password;
//             return buff;
//         }
//     }

//     fclose(fp);
//     return "no user found";
// }


// void registerMenu(char name[50], char password[50]) {
//     FILE *fp;
//     struct User temp;
//     int exists = 0;

//     printf("\n\nRegister a new user.\n");
//     printf("Enter a username: ");
//     scanf("%s", name);

//     fp = fopen(USERS, "a+");
//     if (fp == NULL) {
//         printf("Unable to open user file.\n");
//         exit(1);
//     }

//     // Check if user already exists
//     while (fscanf(fp, "%s %s", temp.name, temp.password) != EOF) {
//         if (strcmp(temp.name, name) == 0) {
//             printf("User already exists!\n");
//             fclose(fp);
//             exit(1);
//         }
//     }

//     printf("Enter a password: ");
//     scanf("%s", password);

//     fprintf(fp, "%s %s\n", name, password);
//     fclose(fp);

//     printf("\n✔ User registered successfully!\n");
// }
