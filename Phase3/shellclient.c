#include <unistd.h>		// / header for unix specic functions declarations : fork(), getpid(), getppid(), sleep()
#include <stdio.h>		// header for input and output from console : printf, perror
#include <sys/socket.h> // header for socket specific functions and macros declarations
#include <stdlib.h>		// header for general functions declarations: exit() and also has exit macros such as EXIT_FAILURE - unsuccessful execution of a program
#include <netinet/in.h> //header for MACROS and structures related to addresses "sockaddr_in", INADDR_ANY
#include <string.h>		// header for string functions declarations: strlen()
#include <arpa/inet.h>	// header for functions related to addresses from text to binary form, inet_pton
#include <signal.h>		// header for signal related functions and macros declarations
#include <sys/time.h>
#include<fcntl.h>	//fcntl

#define CHUNK_SIZE 512

#define PORT 5800

int sock = 0;

int recv_timeout(int s, int timeout);



// function routine of Signal Handler for SIGINT, to send connection termination message to server and terminates the client process
void clientExitHandler(int sig_num )
{
	send(sock, "exit_client", strlen("exit_client"), 0); // sending exit message to server
	close(sock);										 // close the socket/end the conection
	printf("\nExiting client.  \n");
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
		
		send(sock, input, strlen(input), 0);

	
		if (strcmp(input, "exit") == 0)
		{
			printf("Exiting client socket... \n");
			break;
		}
		
		
		// char message[1024] = {0};
		// recv(sock, message, sizeof(message), 0);

		//printf("Client received: \n%s\n", message);

		printf("Cleint received: \n");
		int total_recv = recv_timeout(sock, 1);
		printf("\n\nDone. Received a total of %d bytes\n\n" , total_recv);

		
	}

	printf("All processes terminated. End of session. \n");
	close(sock);

}

// Code used from: https://www.binarytides.com/receive-full-data-with-recv-socket-function-in-c/

int recv_timeout(int s , int timeout)
{
	int size_recv , total_size= 0;
	struct timeval begin , now;
	char chunk[CHUNK_SIZE];
	double timediff;
	
	//make socket non blocking
	fcntl(s, F_SETFL, O_NONBLOCK);
	
	//beginning time
	gettimeofday(&begin , NULL);
	
	while(1)
	{
		gettimeofday(&now , NULL);
		
		//time elapsed in seconds
		timediff = (now.tv_sec - begin.tv_sec) + 1e-6 * (now.tv_usec - begin.tv_usec);
		
		//if you got some data, then break after timeout
		if( total_size > 0 && timediff > timeout )
		{
			break;
		}
		
		//if you got no data at all, wait a little longer, twice the timeout
		else if( timediff > timeout*2)
		{
			break;
		}
		
		memset(chunk ,0 , CHUNK_SIZE);	//clear the variable
		if((size_recv =  recv(s , chunk , CHUNK_SIZE , 0) ) < 0)
		{
			//if nothing was received then we want to wait a little before trying again, 0.1 seconds
			usleep(100000);
		}
		else
		{
			total_size += size_recv;
			printf("%s" , chunk);
			//reset beginning time
			gettimeofday(&begin , NULL);
		}
	}
	
	return total_size;
}