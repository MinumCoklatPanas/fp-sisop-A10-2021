// Server side C/C++ program to demonstrate Socket programming
#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#define PORT 8080
#define REGISTER 0
#define LOGIN 1
#define ADD 0
#define DOWNLOAD 1
#define DELETE 2
#define SEE 3
#define FIND 4

typedef struct login_creds
{
    char id[110];
    char password[110];
    int status;
}creds;

char* respond;
int is_root = 0;
creds login;
char* current_database;

char* getQueryType(char query[])
{
	char* token;
	token = strtok(query," ");
	return token;
}

void create_user(char query[])
{
	char trash[110];
	char uname[110];
	char pass[110];
	sscanf(query,"%s %s %s %s %s %s",trash,trash,uname,trash,trash,pass);
	int ix = strlen(pass) - 1;
	memmove(&pass[ix],&pass[ix+1],strlen(pass)-ix);
	FILE* f;
	f = fopen("database/tables/user.csv","r");
	char buffer[1010];
	int ada = 0;
	while (fscanf(f,"%s",buffer) != EOF)
	{
		// puts("MASUK");
		printf("%s -> buffer\n",buffer);
		const char s[2] = ";";
		char *token;
		
		/* get the first token */
		token = strtok(buffer, s);
		char id[1010];
		char pw[1010];
		strcpy(id,token);
		
		/* walk through other tokens */
		while( token != NULL ) {
			printf( " %s\n", token );
			token = strtok(NULL, s);
			if (strlen(pw) == 0)
				strcpy(pw,token);
		}
		if (strcmp(id,uname) == 0)
		{
			ada = 1;
			break;
		}
	}   
	fclose(f);
	if (ada)
	{
		respond = "User Already Exist!";
	}
	else
	{
		printf("Masuk lagi\n");
		respond = "User Registered!";
		f = fopen("database/tables/user.csv","a");
		fprintf(f,"%s;%s\n",uname,pass);
		fclose(f);
	}
}

void create_database(char query[])
{
	char database[510];
	char trash[510];
	sscanf(query,"%s %s %[^;]",trash,trash,database);

	FILE* f;
	f = fopen("database/tables/available_database.txt","r");
	int ada = 0;
	char buffer[510];
	while (fscanf(f,"%s",buffer) != EOF)
	{
		if (strcmp(buffer,database) == 0)
		{
			ada = 1;
			break;
		}
	}
	fclose(f);
	if (ada)
	{
		respond = "Database already exist";
	}
	else
	{
		// printf("Masuk");
		respond = "Database created";
		f = fopen("database/tables/available_database.txt","a");
		fprintf(f,"%s\n",database);
		fclose(f);
		f = fopen("database/tables/permission.csv","a");
		fprintf(f,"%s;%s\n",database,login.id);
		fclose(f);
		char path[510];
		sprintf(path,"database/tables/%s",database);
		mkdir(path,0777);
	}
}

void create_handler(char query[])
{
	char* tmp;
	//CREATE USER
	tmp = strstr(query,"USER");
	if (tmp != NULL)
	{
		if (!is_root)
		{
			respond = "Cannot execute the command";
			return;
		}
		create_user(query);
		return;
	}

	//CREATE DATABASE
	tmp = strstr(query,"DATABASE");
	if (tmp != NULL)
	{
		create_database(query);
	}
}

void grant_handler(char query[])
{
	if (!is_root)
	{
		respond = "Cannot execute the command";
		return;
	}
	char database[510];
	char uname[510];
	char trash[510];
	sscanf(query,"%s %s %s %s %s;",trash,trash,database,trash,uname);
	int ix = strlen(uname) - 1;
	memmove(&uname[ix],&uname[ix+1],strlen(uname)-ix);
	FILE* f;
	f = fopen("database/tables/permission.csv","a");
	fprintf(f,"%s;%s\n",database,uname);
	fclose(f);
	respond = "Permission Granted";
}

void use_handler(char query[])
{
	char database[510];
	char uname[510];
	char trash[510];
	strcpy(uname,login.id);
	sscanf(query,"%s %s",trash,database);
	int ix = strlen(database) - 1;
	memmove(&database[ix],&database[ix+1],strlen(database)-ix);
	FILE* f;
	f = fopen("database/tables/available_database.txt","r");
	int ada = 0;
	char buffer[510];
	while (fscanf(f,"%s",buffer) != EOF)
	{
		if (strcmp(buffer,database) == 0)
		{
			ada = 1;
			break;
		}
	}
	fclose(f);
	if (ada)
	{
		f = fopen("database/tables/permission.csv","r");
		int bisa = 0;
		while (fscanf(f,"%s",buffer) != EOF)
		{
			char db[510];
			char id[510];
			char cc;
			sscanf(buffer,"%[^;] %c %[^\n]",db,&cc,id);
			if (strcmp(db,database) == 0 && strcmp(id,uname) == 0)
			{
				bisa = 1;
				break;
			}
		}
		fclose(f);
		if (bisa)
		{
			current_database = database;
			respond = "Command executed succesfully";
			return;
		}
		else
		{
			respond = "User doesnt have privilege";
			return;
		}
	}
	else
	{
		respond = "Database doesn't exist";
	}
}

int main(int argc, char const *argv[])
{
	int server_fd, new_socket, valread;
	struct sockaddr_in address;
	int opt = 1;
	int addrlen = sizeof(address);
	char buffer[1024] = {0};
	char *hello = "Hello from server";
	
	// Creating socket file descriptor
	if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
	{
		perror("socket failed");
		exit(EXIT_FAILURE);
	}
	
	// Forcefully attaching socket to the port 8080
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
												&opt, sizeof(opt)))
	{
		perror("setsockopt");
		exit(EXIT_FAILURE);
	}
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons( PORT );
	
	// Forcefully attaching socket to the port 8080
	if (bind(server_fd, (struct sockaddr *)&address,
								sizeof(address))<0)
	{
		perror("bind failed");
		exit(EXIT_FAILURE);
	}
    if (listen(server_fd, 3) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    if ((new_socket = accept(server_fd, (struct sockaddr *)&address,
                    (socklen_t*)&addrlen))<0)
    {
        perror("accept");
        exit(EXIT_FAILURE);
    }
	int logged_in = 0;
	while (1)
	{
		int tmp;
		if (!logged_in)
		{
			if (recv(new_socket,(void*)&login,sizeof(login),0) < 0)
			{
				perror("Login Attempt Failed");
				exit(EXIT_FAILURE);
			}
			printf("Masuk\n");
			if (strcmp(login.id,"root") != 0)
			{
				FILE *f;
				f = fopen("database/tables/user.csv","a+");
				char buffer[1010];
				int ada = 0;
				int benar = 0;
				while (fscanf(f,"%s",buffer) != EOF)
				{
					printf("%s\n",buffer);
					const char s[2] = ";";
					char *token;
					
					/* get the first token */
					token = strtok(buffer, s);
					char id[1010];
					char pw[1010];
					strcpy(id,token);
					strcpy(pw,"");
					
					/* walk through other tokens */
					while( token != NULL ) {
						token = strtok(NULL, s);
						if (strlen(pw) == 0)
							strcpy(pw,token);
					}
					if (strcmp(id,login.id) == 0)
					{
						ada = 1;
						if (strcmp(pw,login.password) == 0)
							benar = 1;
						break;
					}
				}
				fclose(f);
				char message[110];
				if (!ada)
				{
					strcpy(message,"User Not Found!");
				}
				else
				if (!benar)
				{
					strcpy(message,"Password Incorrect!");
				}
				else
				{
					strcpy(message,"Login Success!");
				}
				if (tmp = send(new_socket, (void*)&message,strlen(message),0) < 0)
				{
					perror("Login Process Failed");
					exit(EXIT_FAILURE);
				}
			}
			else
			{
				is_root = 1;
				char message[110];
				strcpy(message,"Login Success!");
				if (tmp = send(new_socket, (void*)&message,strlen(message),0) < 0)
				{
					perror("Login Process Failed");
					exit(EXIT_FAILURE);
				}
			}
			logged_in = 1;
		}
		else
		{
			printf("Masuk2\n");
			char queryBuffer[5010];
			if (tmp = recv(new_socket,(void*)&queryBuffer,sizeof(queryBuffer),0) < 0)
			{
				perror("Failed to Receive Query");
				exit(EXIT_FAILURE);
			}
			printf("%s -> query\n",queryBuffer);
			char query[5010];
			strcpy(query,queryBuffer);
			char* queryType = getQueryType(queryBuffer);
			if (strcmp(queryType,"CREATE") == 0)
			{
				create_handler(query);
				printf("%s\n",respond);
				if (tmp = send(new_socket,(void*)respond,sizeof(respond),0) < 0)
				{
					perror("Failed Sending Create User Message");
					exit(EXIT_FAILURE);
				}
			}
			else
			if (strcmp(queryType,"GRANT") == 0)
			{
				grant_handler(query);
				printf("%s\n",respond);
				if (tmp = send(new_socket,(void*)respond,sizeof(respond),0) < 0)
				{
					perror("Failed Sending Create User Message");
					exit(EXIT_FAILURE);
				}
			}
			else
			if (strcmp(queryType,"USE") == 0)
			{
				use_handler(query);
				printf("%s\n",respond);
				if (tmp = send(new_socket,(void*)respond,sizeof(respond),0) < 0)
				{
					perror("Failed Sending Create User Message");
					exit(EXIT_FAILURE);
				}
			}
		}
	}
}