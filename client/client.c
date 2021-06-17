// Client side C/C++ program to demonstrate Socket programming
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <zconf.h>
#define PORT 8080
#define ADD 0
#define DOWNLOAD 1
#define DELETE 2
#define SEE 3
#define FIND 4

typedef struct login_creds
{
    char id[110];
    char password[110];
}creds;


int main(int argc, char const *argv[])
{

	creds login;
	
	int sock = 0, valread;
	struct sockaddr_in serv_addr;
	char *hello = "Hello from client";
	char buffer[1024] = {0};
	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		printf("\n Socket creation error \n");
		return -1;
	}

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(PORT);
	
	// Convert IPv4 and IPv6 addresses from text to binary form
	if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0)
	{
		printf("\nInvalid address/ Address not supported \n");
		return -1;
	}

	if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
	{
		printf("\nConnection Failed \n");
		return -1;
	}
    int logged_in = 0;
	int not_root = getuid();
	if (not_root)
	{
		if (argc != 5)
		{
			printf("Missing Login Argument\n");
			return 0;
		}
		if (strcmp(argv[1],"-u") != 0 || strcmp(argv[3],"-p") != 0)
		{
			printf("Wrong Arguments\n");
			return 0;
		}
		strcpy(login.id,argv[2]);
		strcpy(login.password,argv[4]);
	}
	else
	{
		strcpy(login.id,"root");
		strcpy(login.password,"");
	}
	while (1)
	{
		int tmp;
		if (!logged_in)
		{
			if (tmp = send(sock,(void*)&login,sizeof(login),0) < 0)
			{
				perror("Failed Sending Login");
				exit(EXIT_FAILURE);
			} 
			char message[110];
			if (tmp = recv(sock,(void*)&message,sizeof(message),0) < 0)
			{
				perror("Failed Receiving Login Message");
				exit(EXIT_FAILURE);
			}
			printf("%s\n",message);
			if (strcmp(message,"Login Success!") != 0) break;
			logged_in = 1;
		}
		else
		{
			printf("Masuk\n");
			char query[5010];
			gets(query);
			if (tmp = send(sock,(void*)&query,sizeof(query),0) < 0)
			{
				perror("Failed to Send Query");
				exit(EXIT_FAILURE);
			}
			char respond[5010];
			if (tmp = recv(sock,(void*)&respond,sizeof(respond),0) < 0)
			{
				perror("Failed Retrieving Query Respond");
				exit(EXIT_FAILURE);
			}
			printf("%s\n",respond);
		}
	}
}