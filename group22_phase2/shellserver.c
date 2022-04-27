#include <signal.h>
#include <unistd.h>     // / header for unix specic functions declarations : fork(), getpid(), getppid(), sleep()
#include <stdio.h>      // header for input and output from console : printf, perror
#include <stdlib.h>     // header for general functions declarations: exit() and also has exit macros such as EXIT_FAILURE - unsuccessful execution of a program
#include <sys/socket.h> // header for socket specific functions and macros declarations
#include <netinet/in.h> //header for MACROS and structures related to addresses "sockaddr_in", INADDR_ANY
#include <string.h>     // header for string functions declarations: strlen()
#include <signal.h>     // header for signal related functions and macros declarations

#define PORT 5800
#define NUM_CLIENTS 10

// function routine of Signal Handler for SIGINT, to terminate all the threads which will all be terminated as we are calling exit of a process instead of pthread_exit
void serverExitHandler(int sig_num)
{
    printf("\n server exit  \n");
    fflush(stdout);// force to flush any data in buffers to the file descriptor of standard output,, a pretty convinent function
    exit(0);
}

// parsing the pipes and finding number of pipes and putting the string into commands
int parse_pipes(char input[], char *commands[])
{
    const char delimeter[2] = "|"; // the delimeter is the pipe
    char *token;                   // the string we got before | - delimeter

    /* get the first token */
    token = strtok(input, delimeter);
    int count = 0;

    /* walk through other tokens and putting them inside the commands */
    while (token != NULL)
    {
        commands[count] = token;
        token = strtok(NULL, delimeter);
        count++;
    }

    return count;
}

/*
void parseSpecialChars(char *token) {
    int w = 0, r = 0;
    for (r = 0; r < strlen(token); r++) {
        if (token[r] != '\'' && token[r] != '\"' && token[r] != '\\')  {
            token[w] = token[r];
            w++;
        }
        else if (r+1 < strlen(token) && token[r+1] == '\'' || token[r+1] == '\"' && token[r] == '\\' ) {
            token[w] = token[r+1];
            w++;
        }
        else if (r+1 < strlen(token) && token[r+1] != '\'' && token[r+1] != '\"' && token[r] == '\\') {
            token[w] = token[r];
            w++;
        }
    }
}*/

// parsing the commands, same as in parse_pipes, but adding the "NULL" at the end for exec
void parse_spaces(char pipe[], char *command[])
{
    const char delimeter[2] = " ";
    char *token;

    /* get the first token */
    token = strtok(pipe, delimeter);
    int count = 0;
    // parseSpecialChars(token);
    command[count] = token;

    /* walk through other tokens */
    while (token != NULL)
    {

        count++;
        token = strtok(NULL, delimeter);
        // parseSpecialChars(token);
        command[count] = token;
    }
}

// the following four functions are processing the commands and pipes based on the amount of pipes
void processing_one_command(char *firstcommand[]) // 0 pipes
{

    execvp(firstcommand[0], firstcommand);
    fprintf(stderr, "Execvp failed while executing %s \n", firstcommand[0]);
    exit(EXIT_FAILURE);
}

void processing_two_commands(char *firstcommand[], char *secondcommand[]) // 1 pipe
{
    int fd1[2]; // pipe 1 for getting output from child 1 and giving it to child 2
    int pid;

    if (pipe(fd1) < 0)
        exit(EXIT_FAILURE);

    pid = fork();
    if (pid < 0)
        exit(EXIT_FAILURE);
    else if (pid == 0) // first command
    {
        dup2(fd1[1], 1); // write by redirecting standard output to pipe 1
        close(fd1[1]);
        close(fd1[0]);
        execvp(firstcommand[0], firstcommand);
        fprintf(stderr, "Execvp failed while executing %s \n", firstcommand[0]);
        exit(EXIT_FAILURE);
    }
    else
    {
        dup2(fd1[0], 0); // reading redirected ouput of ls through pipe 1
        close(fd1[1]);
        close(fd1[0]);
        execvp(secondcommand[0], secondcommand);
        fprintf(stderr, "Execvp failed while executing  %s \n", secondcommand[0]);
        exit(EXIT_FAILURE);
    }
    fprintf(stderr, "Reached unexpectedly\n");
    exit(EXIT_FAILURE);
}

void processing_three_commands(char *firstcommand[], char *secondcommand[], char *thirdcommand[]) // 2 pipes
{
    int fd1[2]; // pipe 1 for getting output from child 1 and giving it to child 2
    int fd2[2]; // pipe 2 for getting output from child 2 and giving it to child 3
    int pid;

    if (pipe(fd1) < 0)
        exit(EXIT_FAILURE);
    if (pipe(fd2) < 0)
        exit(EXIT_FAILURE);

    pid = fork();
    if (pid < 0)
        exit(EXIT_FAILURE);
    else if (pid == 0) // first command
    {
        dup2(fd1[1], 1); // write by redirecting standard output to pipe 1
        close(fd1[1]);
        close(fd1[0]);
        close(fd2[0]);
        close(fd2[1]);
        execvp(firstcommand[0], firstcommand);
        fprintf(stderr, "Execvp failed while executing %s \n", firstcommand[0]);
        exit(EXIT_FAILURE);
    }
    else
    {
        pid = fork(); // second command
        if (pid == 0)
        {
            dup2(fd1[0], 0); // reading redirected ouput of ls through pipe 1
            dup2(fd2[1], 1); // write by redirecting standard output to pipe 2
            close(fd1[1]);
            close(fd1[0]);
            close(fd2[1]);
            close(fd2[0]);
            execvp(secondcommand[0], secondcommand);
            fprintf(stderr, "Execvp failed while executing  %s \n", secondcommand[0]);
            exit(EXIT_FAILURE);
        }
        else
        {
            dup2(fd2[0], 0); // third command
            close(fd1[1]);
            close(fd1[0]);
            close(fd2[1]);
            close(fd2[0]);
            execvp(thirdcommand[0], thirdcommand);
            fprintf(stderr, "Execvp failed while executing  %s \n", thirdcommand[0]);
            exit(EXIT_FAILURE);
        }
    }
    fprintf(stderr, "Reached unexpectedly\n");
    exit(EXIT_FAILURE);
}

void processing_four_commands(char *firstcommand[], char *secondcommand[], char *thirdcommand[], char *forthcommand[]) // 4 pipes
{
    int fd1[2]; // pipe 1 for getting output from child 1 and giving it to child 2
    int fd2[2]; // pipe 2 for getting output from child 2 and giving it to child 3
    int fd3[2]; // pipe 3 fro getting output from child 3 and giving it to child 4
    int pid;

    if (pipe(fd1) < 0)
        exit(EXIT_FAILURE);
    if (pipe(fd2) < 0)
        exit(EXIT_FAILURE);
    if (pipe(fd3) < 0)
        exit(EXIT_FAILURE);

    pid = fork();
    if (pid < 0)
        exit(EXIT_FAILURE);
    else if (pid == 0) // first command
    {
        dup2(fd1[1], 1); // write by redirecting standard output to pipe 1
        close(fd1[1]);
        close(fd1[0]);
        close(fd2[0]);
        close(fd2[1]);
        close(fd3[0]);
        close(fd3[1]);
        execvp(firstcommand[0], firstcommand);
        fprintf(stderr, "Execvp failed while executing %s \n", firstcommand[0]);
        exit(EXIT_FAILURE);
    }
    else
    {
        pid = fork(); // second command
        if (pid == 0)
        {
            dup2(fd1[0], 0); // reading redirected ouput of ls through pipe 1
            dup2(fd2[1], 1); // write by redirecting standard output to pipe 2
            close(fd1[1]);
            close(fd1[0]);
            close(fd2[1]);
            close(fd2[0]);
            close(fd3[0]);
            close(fd3[1]);
            execvp(secondcommand[0], secondcommand);
            fprintf(stderr, "Execvp failed while executing  %s \n", secondcommand[0]);
            exit(EXIT_FAILURE);
        }
        else
        {
            pid = fork(); // third command
            if (pid == 0)
            {
                dup2(fd2[0], 0); // reading redirected ouput through pipe 2
                dup2(fd3[1], 1); // write by redirecting standard output to pipe 3
                close(fd1[1]);
                close(fd1[0]);
                close(fd2[1]);
                close(fd2[0]);
                close(fd3[0]);
                close(fd3[1]);
                execvp(thirdcommand[0], thirdcommand);
                fprintf(stderr, "Execvp failed while executing  %s \n", thirdcommand[0]);
                exit(EXIT_FAILURE);
            }
            else // forth command
            {
                dup2(fd3[0], 0); // reading redirected output through pipe 3
                close(fd1[1]);
                close(fd1[0]);
                close(fd2[1]);
                close(fd2[0]);
                close(fd3[0]);
                close(fd3[1]);
                execvp(forthcommand[0], forthcommand);
                fprintf(stderr, "Execvp failed while executing  %s \n", forthcommand[0]);
                exit(EXIT_FAILURE);
            }
        }
    }
    fprintf(stderr, "Reached unexpectedly\n");
    exit(EXIT_FAILURE);
}

void reading_input(char *input) // reading teh input and working with it
{
    // declaring the commands
    char *firstcommand[5];
    char *secondcommand[5];
    char *thirdcommand[5];
    char *fourthcommand[5];

    char *commands[4];
    int amount_of_pipes = 0;

    if (strlen(input) == 0)
    {
        return;
    }

    // getting amount of pipes
    amount_of_pipes = parse_pipes(input, commands);

    // based on the amount of pipes we are doing the corresponding processing function
    switch (amount_of_pipes)
    {
    case 1:
        parse_spaces(commands[0], firstcommand);
        if (strcmp(firstcommand[0], "exit") == 0) // if the user writes exit, we are terminating teh parent pid and exit the program
        {                                         // handle exit command
            printf("Exit program. Terminating... \n");
            kill(getppid(), SIGUSR1); // kill the parent process within a child
            exit(0);
        }
        // if not, processing the commands
        processing_one_command(firstcommand);
        break;
    case 2:
        parse_spaces(commands[0], firstcommand);
        parse_spaces(commands[1], secondcommand);
        processing_two_commands(firstcommand, secondcommand);
        break;
    case 3:
        parse_spaces(commands[0], firstcommand);
        parse_spaces(commands[1], secondcommand);
        parse_spaces(commands[2], thirdcommand);
        processing_three_commands(firstcommand, secondcommand, thirdcommand);
        break;
    case 4:
        parse_spaces(commands[0], firstcommand);
        parse_spaces(commands[1], secondcommand);
        parse_spaces(commands[2], thirdcommand);
        parse_spaces(commands[3], fourthcommand);
        processing_four_commands(firstcommand, secondcommand, thirdcommand, fourthcommand);
        break;
    }
}

int main() // main function
{
    signal(SIGINT, serverExitHandler);

    int sock1, sock2, valread;
    struct sockaddr_in address; // structure for storing addres; local interface and port
    int addrlen = sizeof(address);

    // Creating socket file descriptor with communication: domain of internet protocol version 4, type of SOCK_STREAM for reliable/conneciton oriented communication, protocol of internet
    if ((sock1 = socket(AF_INET, SOCK_STREAM, 0)) == 0) // checking if socket creation fail
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // setting the address to be bind to socket
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    address.sin_port = htons(PORT);

    // attaching socket to addresses (any/all local ip with port 5564)
    if (bind(sock1, (struct sockaddr *)&address, sizeof(address)) < 0) // checking if bind fails
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    while (1)
    {
        if (listen(sock1, NUM_CLIENTS) < 0) // defining for socket length of queue for pending client connections
        {
            perror("Listen Failed");
            exit(EXIT_FAILURE);
        }
        if ((sock2 = accept(sock1, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0) // accepting the client connection with creation/return of a new socket for the established connection to enable dedicated communication (active communication on a new socket) with the client
        {
            perror("accept");
            exit(EXIT_FAILURE);
        }

        char *shell_init_msg =
            "\n\n******************"
            "*********************************"
            "\n\n  **Remote CLI Shell G22 by Adilet and Shyngys**  "
            "\n\n\t- Start by typing your commands! -"
            "\n\n*******************"
            "********************************\n";
        send(sock2, shell_init_msg, strlen(shell_init_msg), 0);

        while (1)
        {

            pid_t pid = fork();
            if (pid < 0)
            {
                printf("Child fork failure\n");
                exit(EXIT_FAILURE);
            }
            else if (pid == 0)
            {

                char message[1024] = {0};
                recv(sock2, message, sizeof(message), 0);
                printf("Server Received: %s\n", message);
                message[strcspn(message, "\n")] = 0;

                if (strcmp(message, "exit") == 0)
                {
                    printf("Exiting server socket... \n");
                    close(sock2);
                    break;
                }

                // redirecting stdout to sock2
                //  from: https://stackoverflow.com/questions/8100817/redirect-stdout-and-stderr-to-socket-in-c
                dup2(sock2, STDOUT_FILENO);
                dup2(sock2, STDERR_FILENO);
                close(sock2);

                reading_input(message);
            }
            else
            {
                wait(NULL);

                // continue;
            }
        }
    }
    printf("All processes terminated. End of session. \n");

    close(sock1);

    return 0;
}