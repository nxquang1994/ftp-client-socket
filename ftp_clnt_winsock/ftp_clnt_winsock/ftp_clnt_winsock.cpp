// ftp_clnt_winsock.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "Header-ftp-client.h"
#pragma comment(lib, "Ws2_32.lib")

using namespace std;

void errexit(const char *, ...);
void pause(void);
void replylogcode(int code)
{
	switch(code){
		case 200:
			printf("Command okay");
			break;
		case 500:
			printf("Syntax error, command unrecognized.");
			printf("This may include errors such as command line too long.");
			break;
		case 501:
			printf("Syntax error in parameters or arguments.");
			break;
		case 202:
			printf("Command not implemented, superfluous at this site.");
			break;
		case 502:
			printf("Command not implemented.");
			break;
		case 503:
			printf("Bad sequence of commands.");
			break;
		case 530:
			printf("Not logged in.");
			break;
	}
	printf("\n");
}

char* sendCommand(char str[100])
{
	//sprintf(buf,"USER %s\r\n",info);
	return NULL;
}


int _tmain(int argc, char* argv[])
{
	// ====================================
	// CREATE SOCKET
	WSADATA wsa;
	SOCKET socketClient;
	cout << "Initilize Winsock ... " << endl;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		cout << "Failed, error code: " << WSAGetLastError() << endl;
		return 1;
	}
	cout << "Initialized" << endl;
	// create a socket
	if ((socketClient = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
	{
		cout << "Socket could not created: " << WSAGetLastError() << endl;
	}
	cout << "Socket created" << endl;

	// ====================================
	// CONNECT TO SERVER
	struct sockaddr_in server;
	server.sin_addr.S_un.S_addr = inet_addr(SERVER_IP);
	server.sin_family = AF_INET;
	server.sin_port = htons(SERVER_FTP_PORT);
	if (connect(socketClient, (struct sockaddr*)&server, sizeof(server)) < 0)
	{
		cout << "Connect error" << endl;
		return 1;
	}
	pause();

	// ====================================
	// LOGIN
	char buf[BUFSIZ + 1];
	int tmpres, size, status;
	char * str;
	int codeftp;
	printf("Connection established, waiting for welcome message...\n");
	memset(buf, 0, sizeof buf);
	while ((tmpres = recv(socketClient, buf, BUFSIZ, 0)) > 0) {
		sscanf(buf, "%d", &codeftp);
		printf("%s", buf);
		if (codeftp != 220) //120, 240, 421: something wrong
		{
			replylogcode(codeftp);
			exit(1);
		}

		str = strstr(buf, "220");
		if (str != NULL) {
			break;
		}
		memset(buf, 0, tmpres);
	}
	//Send Username
	char info[50];
	printf("Name (%s): ", inet_ntoa(server.sin_addr));
	memset(buf, 0, sizeof buf);
	scanf("%s", info);

	sprintf(buf, "USER %s\r\n", info);
	tmpres = send(socketClient, buf, strlen(buf), 0);

	memset(buf, 0, sizeof buf);
	tmpres = recv(socketClient, buf, BUFSIZ, 0);

	sscanf(buf, "%d", &codeftp);
	if (codeftp != 331)
	{
		replylogcode(codeftp);
		exit(1);
	}
	printf("%s", buf);

	//Send Password
	memset(info, 0, sizeof info);
	printf("Password: ");
	memset(buf, 0, sizeof buf);
	scanf("%s", info);

	sprintf(buf, "PASS %s\r\n", info);
	tmpres = send(socketClient, buf, strlen(buf), 0);

	memset(buf, 0, sizeof buf);
	tmpres = recv(socketClient, buf, BUFSIZ, 0);

	sscanf(buf, "%d", &codeftp);
	if (codeftp != 230)
	{
		replylogcode(codeftp);
		exit(1);
	}
	printf("%s", buf);

	// ====================================
	// Working process
	while (1) {
		char cmd[50];
		printf("ftp> ");
		scanf("%s", cmd);
		if (strcmp(cmd, "exit") == 0) {
			break;
		}
	}

	return 0;
}


void pause(void)
{
	char c;

	printf("Press Enter to continue\n");
	scanf("%c", &c);
}


