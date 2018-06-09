// ftp_clnt_winsock.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "Header-ftp-client.h"
#pragma comment(lib, "Ws2_32.lib")
#include <iostream>
#include <string>
#include <fstream>.
using namespace std;
#define MAXLINE 4096

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

int createDTPSocket(char* info) {
	int serverDTP;
	//SOCKET serverDTP
	// create a socket
	if ((serverDTP = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
	{
		cout << "Socket could not created: " << WSAGetLastError() << endl;
	}
	return(serverDTP);
}

HostInfo extractPassivePacket(char* buf) {
	// 227 Entering Passive Mode (127,0,0,1,195,47)
	char* rawInfo = strchr(buf, 40); // 40 correspomd character '('
	int len = strlen(rawInfo);
	HostInfo currentHost;
	// extract IP
	int i = 1;
	int currentCommaIndex = 0;
	while (currentCommaIndex < LAST_COMMA_IP_PART)
	{
		if (rawInfo[i] == ',')
		{
			currentHost.ip[i - 1] = '.'; // i start from 1
			currentCommaIndex++;
		}
		else {
			currentHost.ip[i - 1] = rawInfo[i];
		}
		i++;
	}
	currentHost.ip[i - 2] = '\0';

	// extract port
	bool isAfterComma = false;
	int j = 0;
	int k = 0;
	char h1[6];
	char h2[6];
	while (rawInfo[i] != ')') {
		if (!isAfterComma && rawInfo[i] != ',')
		{
			h1[j] = rawInfo[i];
			j++;
		}
		else if (rawInfo[i] == ',')
		{
			isAfterComma = true;
		}
		else
		{
			h2[k] = rawInfo[i];
			k++;
		}
		i++;
	}
	currentHost.h1 = atoi(h1);
	currentHost.h2 = atoi(h2);
	currentHost.port = currentHost.h1 * 256 + currentHost.h2;
	return currentHost;
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
		// TODO: process each command here
		char tokenCmd[10];
		sscanf(cmd, "%s", tokenCmd);
		if (strcmp(tokenCmd, "ls")==0)
		{
			// Create passive mode
			char passive[5];
			strcpy(passive, "PASV\n");
			/// Get DTP passive information
			tmpres = send(socketClient, passive, strlen(passive), 0);
			memset(buf, 0, sizeof buf);
			tmpres = recv(socketClient, buf, BUFSIZ, 0); // 227 Entering Passive Mode (127,0,0,1,195,47)
			/// Create DTP socket
			SOCKET socketDTP = createDTPSocket(buf);
			/// Bind DTP socket
			struct sockaddr_in serverDTP;
			HostInfo serverDTPInfo = extractPassivePacket(buf);
			serverDTP.sin_addr.S_un.S_addr = inet_addr(serverDTPInfo.ip);
			serverDTP.sin_family = AF_INET;
			serverDTP.sin_port = htons(serverDTPInfo.port);
			if (connect(socketDTP, (struct sockaddr*)&serverDTP, sizeof(serverDTP)) < 0)
			{
				cout << "Connect to server DTP error" << endl;
				return 1;
			}

			// process list files/folders
			char cmdSend[5];
			strcpy(cmdSend, "LIST\n");
			
			// send command
			tmpres = send(socketDTP, cmdSend, strlen(cmdSend), 0);

			// Receive information
			memset(buf, 0, sizeof buf);
			while ((tmpres = recv(socketDTP, buf, BUFSIZ, 0)) > 0)
			{
				sscanf(buf, "%d", &codeftp);
				printf("%s", buf);
				if (codeftp != 220)
				{
					printf("ERROR! There is error when receive data from server. Please try again\n");
					break;
				}
				else {
					printf("%s\n", buf);
				}
				memset(buf, 0, tmpres);
			}
		}
		else if (strcmp(tokenCmd, "put") == 0)
		{
			// process upload file
			int port = rand() % (65535 - 49152) + 49152;
			int port1 = port / 256;
			int port2 = port % 256;
			sprintf(buf, "PORT 127,0,0,1,%d,%d\r\n", port1, port2);
			if (send(socketClient, buf, strlen(buf), 0) < 0)
			{
				cout << "Sending PORT command failed!!!";
				exit(1);
			}
			tmpres = recv(socketClient, buf, BUFSIZ, 0);
			printf("%s", buf);

			
			fflush(stdin);
			string filename = "C:\New folder\MMT\ftp-client-socket\ftp_clnt_winsock\ftp_clnt_winsock\text.txt";
			//getline(cin, filename);
			ifstream file;
			file.open(filename, ios::in || ios::binary);
			
			string command = "STOR text.txt\r\n";
			tmpres = send(socketClient, command.c_str(), command.length(), 0);
			tmpres = recv(socketClient, buf, BUFSIZ, 0);
			printf("%s", buf);

			//HostInfo host = extractPassivePacket(buf);
			struct sockaddr_in server_upload;
			server_upload.sin_addr.S_un.S_addr = inet_addr(SERVER_IP);
			server_upload.sin_family = AF_INET;
			server_upload.sin_port = htons(port);
			SOCKET ClientUpload = createDTPSocket(buf);
			if (connect(ClientUpload, (struct sockaddr*)&server_upload, sizeof(server_upload)) < 0)
			{
				cout << "Connect error" << endl;
				return 1;
			}
			string data;
			getline(file, data);
			file.close();
			tmpres = send(ClientUpload, data.c_str(), data.length(), 0);
			tmpres = recv(ClientUpload, buf, BUFSIZ, 0);
			printf("%s", buf);
/*
			char port[MAXLINE], buffer[MAXLINE], char_num_blks[MAXLINE], char_num_last_blk[MAXLINE];
			int data_port, datasock, lSize, num_blks, num_last_blk, i;
			*/
		}
		else if (strcmp(tokenCmd, "get") == 0)
		{
			// Process download file
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


