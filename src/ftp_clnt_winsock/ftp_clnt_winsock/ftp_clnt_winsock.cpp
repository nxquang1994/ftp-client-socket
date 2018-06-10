// ftp_clnt_winsock.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "Header-ftp-client.h"
#pragma comment(lib, "Ws2_32.lib")
#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <vector>
using namespace std;
#define MAXLINE 4096

void replylogcode(int code)
{
	switch (code) {
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
	case 250:
		printf("Request completely.");
		break;
	case 421:
		printf("Service unavailable, too many requests.");
		break;
	case 550:
		printf("Directory/File is not exist");
		break;
	}
	printf("\n");
}

char* sendCommand(char str[100])
{
	//sprintf(buf,"USER %s\r\n",info);
	return NULL;
}

int createDTPSocket() {
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

int uploadFile(string filename, SOCKET socketClient, char* str_port) {
	int tmpres, codeftp;
	char buf[BUFSIZ + 1];

	SOCKET socketDTP = createDTPSocket();
	/// Bind DTP socket
	struct sockaddr_in serverDTP;
	HostInfo serverDTPInfo = extractPassivePacket(str_port);
	serverDTP.sin_addr.S_un.S_addr = inet_addr(serverDTPInfo.ip);
	serverDTP.sin_family = AF_INET;
	serverDTP.sin_port = htons(serverDTPInfo.port);
	if (connect(socketDTP, (struct sockaddr*)&serverDTP, sizeof(serverDTP)) < 0)
	{
		cout << "Connect to server DTP error" << endl;
		return 0;
	}

	char cmdSend[COMMAND_LEN];
	string cmdUpload = "STOR " + filename + "\n";
	//strcpy(cmdSend, );

	// send command
	tmpres = send(socketClient, cmdUpload.c_str(), cmdUpload.length(), 0);
	tmpres = recv(socketClient, buf, BUFSIZ, 0);
	sscanf(buf, "%d", &codeftp);
	//printf("%s\n", buf);


	if (codeftp == 150) {
		// Receive information
		memset(buf, 0, sizeof buf);
		//Đọc file
		fflush(stdin);
		//filename = "text.txt";
		//getline(cin, filename);
		ifstream file(filename, ios::in || ios::binary);
		string data;
		string line;
		getline(file, line);
		tmpres = send(socketDTP, line.c_str(), line.length(), 0);
		if (file)
		{
			// get length of file:
			int length;
			file.seekg(0, file.end);
			int tmpLen = file.tellg();
			if (tmpLen)
			{
				length = tmpLen;
			}

			file.seekg(0, file.beg);
			int numBlock = length / BUFSIZ;
			int size_lastBlock = length % BUFSIZ;
			int curBlock = 0;
			char * buffer = new char[length];
			file.read(buffer, length);

			tmpres = send(socketDTP, buffer, strlen(buffer), 0);
			//while (!file.eof())
			//{
			//	/*getline(file, line);
			//	data = "\n" + line;*/
			//	if (curBlock < numBlock)
			//	{
			//		char * buffer = new char[BUFSIZ];
			//		file.read(buffer, BUFSIZ);

			//		tmpres = send(socketDTP, buffer, strlen(buffer), 0);
			//		printf("%s", buffer);
			//	}
			//	else
			//	{
			//		char * buffer = new char[size_lastBlock];
			//		file.read(buffer, size_lastBlock);
			//		tmpres = send(socketDTP, buffer, strlen(buffer), 0);
			//	}
			//	curBlock++;
			//}
			file.close();
		}

		int retCode = closesocket(socketDTP);
		tmpres = recv(socketClient, buf, BUFSIZ, 0);

		sscanf(buf, "%d", &codeftp);
		//printf("%s\n", buf);
		memset(buf, 0, tmpres);
		return codeftp;

	}
	else {
		string err = "Không thể mở kênh truyền để upload!\n";
		printf("%s\n", err.c_str());
		return 0;
	}
}

int downloadFile(string filename, SOCKET socketClient, char* str_port) {
	int tmpres, codeftp;
	char buf[BUFSIZ + 1];
	// Create passive mode
	SOCKET socketDTP = createDTPSocket();
	/// Bind DTP socket
	struct sockaddr_in serverDTP;
	HostInfo serverDTPInfo = extractPassivePacket(str_port);
	serverDTP.sin_addr.S_un.S_addr = inet_addr(serverDTPInfo.ip);
	serverDTP.sin_family = AF_INET;
	serverDTP.sin_port = htons(serverDTPInfo.port);
	if (connect(socketDTP, (struct sockaddr*)&serverDTP, sizeof(serverDTP)) < 0)
	{
		cout << "Connect to server DTP error" << endl;
		return 1;
	}

	char cmdSend[COMMAND_LEN];
	string cmdUpload = "RETR " + filename + "\n";
	//strcpy(cmdSend, );

	// send command
	tmpres = send(socketClient, cmdUpload.c_str(), cmdUpload.length(), 0);
	tmpres = recv(socketClient, buf, BUFSIZ, 0);
	sscanf(buf, "%d", &codeftp);

	// Receive information
	memset(buf, 0, sizeof buf);
	while ((tmpres = recv(socketDTP, buf, BUFSIZ, 0)) > 0)
	{
		sscanf(buf, "%d", &codeftp);
		printf("%s\n", buf);
		ofstream file(filename, ios::in || ios::binary);
		string data = string(buf);
		file << data;
		memset(buf, 0, tmpres);
	}
	tmpres = recv(socketClient, buf, BUFSIZ, 0);
	sscanf(buf, "%d", &codeftp);
	memset(buf, 0, tmpres);
	int retCode = closesocket(socketDTP);
	return codeftp;
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
	// Create passive mode
	bool isPassiveMode = true;

	// Now, start to run
	while (1) {
		char cmd[50];
		printf("ftp> ");
		//scanf("%[^\n]s", cmd);
		fgets(cmd, MAX_COMMAND_LEN, stdin);
		char tokenCmd[10];
		vector<string> param;
		sscanf(cmd, "%s", tokenCmd);

		stringstream ss(cmd);
		string token;
		int i = 0;
		while (getline(ss, token, ' '))
		{
			if (i > 0) {
				int len = token.length();
				char* c_filename = new char[len];
				sscanf(token.c_str(), "%s", c_filename);
				if (c_filename[len - 1] == ',')
				{
					token.replace(len - 1, 1, "");
					sscanf(token.c_str(), "%s", c_filename);
				}

				param.push_back(string(c_filename));
			}
			i++;
		}
		// TODO: process each command here
		// Exit
		if (strcmp(tokenCmd, "exit") == 0) {
			char quit[COMMAND_LEN];
			strcpy(quit, "QUIT\n");
			tmpres = send(socketClient, quit, strlen(quit), 0);
			memset(buf, 0, sizeof buf);
			tmpres = recv(socketClient, buf, BUFSIZ, 0);
			sscanf(buf, "%d", &codeftp);
			if (codeftp != 221)
			{
				printf("Quit server failed, douma\n");
			}
			break;
		}

		// List file
		if (strcmp(tokenCmd, "ls") == 0)
		{
			// Create passive mode
			char passive[COMMAND_LEN];
			strcpy(passive, "PASV\n");
			/// Get DTP passive information
			tmpres = send(socketClient, passive, strlen(passive), 0);
			memset(buf, 0, sizeof buf);
			tmpres = recv(socketClient, buf, BUFSIZ, 0); // 227 Entering Passive Mode (127,0,0,1,195,47)
														 /// Create DTP socket
			SOCKET socketDTP = createDTPSocket();
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
			char cmdSend[COMMAND_LEN];
			strcpy(cmdSend, "LIST\n");

			// send command
			tmpres = send(socketClient, cmdSend, strlen(cmdSend), 0);
			//tmpres = send(socketDTP, cmdSend, strlen(cmdSend), 0);

			// Receive information
			memset(buf, 0, sizeof buf);
			while ((tmpres = recv(socketDTP, buf, BUFSIZ, 0)) > 0)
			{
				sscanf(buf, "%d", &codeftp);
				printf("%s\n", buf);
				memset(buf, 0, tmpres);
			}

			// Receive the packet: """ 150 Opening data channel for directory listing of "/"
			/// and 226 Successfully transferred "/" """
			tmpres = recv(socketClient, buf, BUFSIZ, 0);
			memset(buf, 0, tmpres);
			int retCode = closesocket(socketDTP);
		}
		else if (strcmp(tokenCmd, "put") == 0)
		{
			if (isPassiveMode==true) 
			{
				char passive[COMMAND_LEN];
				strcpy(passive, "PASV\n");
				tmpres = send(socketClient, passive, strlen(passive), 0);
				memset(buf, 0, sizeof buf);
				tmpres = recv(socketClient, buf, BUFSIZ, 0); // 227 Entering Passive Mode (127,0,0,1,195,47)
			}
			else {
				int port = rand() % (65535 - 49152) + 49152;
				stringstream ss;
				ss << port / 256;
				string port1 = ss.str();
				ss << port % 256;
				string port2 = ss.str();
				string cmd = "PORT 127,0,0,1," + port1 + "," + port2 + "\n";
				//sprintf(buf, "PORT 127,0,0,1,%s,%s\r\n", port1, port2);
				
				tmpres = send(socketClient, cmd.c_str(), cmd.length(), 0);
				memset(buf, 0, sizeof buf);
				tmpres = recv(socketClient, buf, BUFSIZ, 0);
				memset(buf, 0, sizeof buf);
				string str_port = "227 Entering Passive Mode (127,0,0,1," + port1 + "," + port2 + ")";
				strcpy(buf, str_port.c_str());

			}
			string filename = param[0];//"text.txt";
			int returnCode = uploadFile(filename, socketClient, buf);
			if (returnCode == 226) {
				string notify = "Upoad file \"" + filename + "\" thanh cong!";
				printf("%s\n", notify.c_str());
			}
			else {
				string notify = "Upoad file \"" + filename + "\" khong thanh cong!";
				printf("%s\n", notify.c_str());
			}
		}
		else if (strcmp(tokenCmd, "mput") == 0)
		{
			int i = 0;

			while (i < param.size()) {
				if (isPassiveMode) {
					char passive[COMMAND_LEN];
					strcpy(passive, "PASV\n");
					tmpres = send(socketClient, passive, strlen(passive), 0);
					memset(buf, 0, sizeof buf);
					tmpres = recv(socketClient, buf, BUFSIZ, 0); // 227 Entering Passive Mode (127,0,0,1,195,47)
				}
				else {
					int port = rand() % (65535 - 49152) + 49152;
					stringstream ss;
					ss << port / 256;
					string port1 = ss.str();
					ss << port % 256;
					string port2 = ss.str();
					string cmd = "PORT 127,0,0,1," + port1 + "," + port2 + "\n";
					//sprintf(buf, "PORT 127,0,0,1,%s,%s\r\n", port1, port2);

					tmpres = send(socketClient, cmd.c_str(), cmd.length(), 0);
					memset(buf, 0, sizeof buf);
					tmpres = recv(socketClient, buf, BUFSIZ, 0);
					memset(buf, 0, sizeof buf);
					string str_port = "227 Entering Passive Mode (127,0,0,1," + port1 + "," + port2 + ")";
					strcpy(buf, str_port.c_str());

				}
				string filename = param[i];
				int returnCode = uploadFile(filename, socketClient, buf);
				if (returnCode == 226) {
					string notify = "Upoad file \"" + filename + "\" thanh cong!";
					printf("%s\n", notify.c_str());
				}
				else {
					string notify = "Upoad file \"" + filename + "\" khong thanh cong!";
					printf("%s\n", notify.c_str());
					break;
				}
				i++;
			}

		}
		else if (strcmp(tokenCmd, "get") == 0)
		{
			// Process download file
			if (isPassiveMode) {
				char passive[COMMAND_LEN];
				strcpy(passive, "PASV\n");
				tmpres = send(socketClient, passive, strlen(passive), 0);
				memset(buf, 0, sizeof buf);
				tmpres = recv(socketClient, buf, BUFSIZ, 0); // 227 Entering Passive Mode (127,0,0,1,195,47)
			}
			else {
				int port = rand() % (65535 - 49152) + 49152;
				stringstream ss;
				ss << port / 256;
				string port1 = ss.str();
				ss << port % 256;
				string port2 = ss.str();
				string cmd = "PORT 127,0,0,1," + port1 + "," + port2 + "\n";
				strcpy(buf, cmd.c_str());
				//sprintf(buf, "PORT 127,0,0,1,%s,%s\r\n", port1, port2);

				tmpres = send(socketClient, buf, strlen(buf), 0);
				memset(buf, 0, sizeof buf);
				tmpres = recv(socketClient, buf, BUFSIZ, 0);
				memset(buf, 0, sizeof buf);
				string str_port = "227 Entering Passive Mode (127,0,0,1," + port1 + "," + port2 + ")";
				strcpy(buf, str_port.c_str());

			}
			string filename = param[0];
			int returnCode = downloadFile(filename, socketClient, buf);
			//printf("%d", returnCode);
			if (returnCode == 226)
			{
				string notify = "Download file \"" + filename + "\" thanh cong!";
				printf("%s\n", notify.c_str());
			}
			else
			{
				string notify = "Download file \"" + filename + "\" that bai!";
				printf("%s\n", notify.c_str());
			}
		}
		else if (strcmp(tokenCmd, "mget") == 0)
		{
			int i = 0;

			while (i < param.size()) {
				if (isPassiveMode) {
					char passive[COMMAND_LEN];
					strcpy(passive, "PASV\n");
					tmpres = send(socketClient, passive, strlen(passive), 0);
					memset(buf, 0, sizeof buf);
					tmpres = recv(socketClient, buf, BUFSIZ, 0); // 227 Entering Passive Mode (127,0,0,1,195,47)
				}
				else {
					int port = rand() % (65535 - 49152) + 49152;
					stringstream ss;
					ss << port / 256;
					string port1 = ss.str();
					ss << port % 256;
					string port2 = ss.str();
					string cmd = "PORT 127,0,0,1," + port1 + "," + port2 + "\n";
					//sprintf(buf, "PORT 127,0,0,1,%s,%s\r\n", port1, port2);

					tmpres = send(socketClient, cmd.c_str(), cmd.length(), 0);
					memset(buf, 0, sizeof buf);
					tmpres = recv(socketClient, buf, BUFSIZ, 0);
					memset(buf, 0, sizeof buf);
					string str_port = "227 Entering Passive Mode (127,0,0,1," + port1 + "," + port2 + ")";
					strcpy(buf, str_port.c_str());

				}
				string filename = param[i];
				int returnCode = downloadFile(filename, socketClient, buf);
				//printf("%d", returnCode);
				if (returnCode == 226)
				{
					string notify = "Download file \"" + filename + "\" thanh cong!";
					printf("%s\n", notify.c_str());
				}
				else
				{
					string notify = "Download file \"" + filename + "\" that bai!";
					printf("%s\n", notify.c_str());
				}
			}
		}
		else if (strcmp(tokenCmd, "cd") == 0) {
			// Process change directory
			char destPath[COMMAND_OPTION_LEN];
			sscanf(cmd, "%*s %s", destPath);
			char cd[MAX_COMMAND_LEN];
			sprintf(cd, "CWD %s\n", destPath);
			tmpres = send(socketClient, cd, strlen(cd), 0);
			memset(buf, 0, BUFSIZ);
			if ((tmpres = recv(socketClient, buf, BUFSIZ, 0)) > 0) {
				sscanf(buf, "%d", &codeftp);
				replylogcode(codeftp);
			}
			else {
				printf("Receive data error, please try it later, ahuhu !!!\n");
			}
		}
		else if (strcmp(tokenCmd, "passive") == 0)
		{
			isPassiveMode = true;
			printf("Passive mode is on. Off Active mode!\n");
		}
		else if (strcmp(tokenCmd, "active") == 0)
		{
			isPassiveMode = false;
			printf("Active mode is on. Off Passive mode!\n");
		}
		else
		{

		}
	}

	int retcode = closesocket(socketClient);
	if (retcode == SOCKET_ERROR)
		errexit("Close socket failed: %d\n", WSAGetLastError());

	retcode = WSACleanup();
	if (retcode == SOCKET_ERROR)
		errexit("Cleanup failed: %d\n", WSAGetLastError());

	return 0;
}

void errexit(const char *format, ...)
{
	va_list	args;

	va_start(args, format);
	vfprintf(stderr, format, args);
	va_end(args);
	WSACleanup();
	pause();
	exit(1);
}


void pause(void)
{
	char c;

	printf("Press Enter to continue\n");
	scanf("%c", &c);
}


