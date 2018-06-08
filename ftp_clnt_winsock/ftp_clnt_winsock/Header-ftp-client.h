#pragma once
#include <winsock2.h>	// Use winsock.h if you're at WinSock 1.1
#include <stdio.h>
#include <iostream>

#define SERVER_IP "127.0.0.1"
#define SERVER_FTP_PORT 21
#define LAST_COMMA_IP_PART 4
#define COMMAND_LEN 10

struct HostInfo {
	char ip[16];
	int h1;
	int h2;
	int port;
};

void errexit(const char *, ...);
void pause(void);