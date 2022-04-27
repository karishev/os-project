#include <unistd.h>		// / header for unix specic functions declarations : fork(), getpid(), getppid(), sleep()
#include <stdio.h>		// header for input and output from console : printf, perror
#include <sys/socket.h> // header for socket specific functions and macros declarations
#include <stdlib.h>		// header for general functions declarations: exit() and also has exit macros such as EXIT_FAILURE - unsuccessful execution of a program
#include <netinet/in.h> //header for MACROS and structures related to addresses "sockaddr_in", INADDR_ANY
#include <string.h>		// header for string functions declarations: strlen()
#include <arpa/inet.h>	// header for functions related to addresses from text to binary form, inet_pton
#include <signal.h>		// header for signal related functions and macros declarations

#define PORT 5800

// function routine of Signal Handler for SIGINT, to send connection termination message to server and terminates the client process
void clientExitHandler(int sig_num, void *socket)
{
	int sock = *(int *)socket;
	send(sock, "exit_client", strlen("exit_client"), 0); // sending exit message to server
	close(sock);										 // close the socket/end the conection
	printf("Exiting client.  \n");
	fflush(stdout); // force to flush any data in buffers to the file descriptor of standard output,, a pretty convinent function
	exit(0);
}

// to print pwd, current directory adn the user name
void print_current_directory()
{
	char cwd[1024];
	char *username = getenv("USER");
	getcwd(cwd, sizeof(cwd));
	printf("\n\n%s: %s$ ", username, cwd);
}

int main()
{
	signal(SIGINT, clientExitHandler);
	int sock = 0;
	struct sockaddr_in serv_addr;

	// Creating socket file descriptor with communication: domain of internet protocol version 4, type of SOCK_STREAM for reliable/conneciton oriented communication, protocol of internet
	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) // checking if socket creation fail
	{
		printf("\n Socket creation error \n");
		return -1;
	}

	// setting the address to connect socket to server
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(PORT);

	// Convert IPv4 and IPv6 addresses from text to binary form and set the ip
	// This function converts the character string 127.0.0.1 into a network
	// address structure in the af address family, then copies the
	// network address structure to serv_addr.sin_addr
	if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) // check if conversion failed
	{
		printf("\nInvalid address/ Address not supported \n");
		return -1;
	}

	// connect the socket with the adddress and establish connnection with the server
	if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
	{
		printf("\nConnection Failed \n");
		return -1;
	}

	printf("\nConnected to the server at the port: %d \n", PORT);

	// receive shell initialization message from server
	char shell_init[1024] = {0};
	recv(sock, shell_init, sizeof(shell_init), 0);
	printf("%s", shell_init);

	while (1)
	{

		print_current_directory();

		char input[1024];
		fgets(input, sizeof(input), stdin);
		input[strcspn(input, "\n")] = 0;

		if (strlen(input) == 0)
		{
			continue;
		}

		// If user enters exit command, we send it first to the server, so that server process terminates and then we check the command on client and terminate
		if (strcmp(input, "exit") == 0)
		{
			printf("Exiting client socket... \n");
			break;
		}

		send(sock, input, strlen(input), 0);
		
		char message[1024] = {0};
		recv(sock, message, sizeof(message), 0);

		printf("Client received: \n%s\n", message);
	}

	printf("All processes terminated. End of session. \n");
	close(sock);
}