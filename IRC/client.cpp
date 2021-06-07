#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include <istream>
#include <vector>
#include <queue>
#include <thread>
#include "SendRecvMsg.h"
#include "Common.h"
#include "SharedTask.h"
#include "MessageStorage.h"
#include "HandleMessages.h"


// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")
//queue<pair<string, string>> msgBuf;
void collectMsg(SOCKET* connectSock, MessageStorage* storage) {
	SendRecvMsg srObj;
	srObj._clientSock = connectSock;
	while(1) {
		int iResult = srObj.receive();
		if (iResult == 1) break;
		//cout << "Payload, Length: " << srObj._payloadLength << " " << srObj._payload << endl;
		auto ctr = SharedTask::parseCmdUser(srObj._payload);
		int msgLength = stoi(ctr[2]);
		string cleanMsg = srObj._payload.substr(0, msgLength);
		//cout << "Cmd, User, Len: " << ctr[0] << " " << ctr[1] << " " << ctr[2] << endl;
		if (ctr[0] == SEND_MSG_RES) {
			
			auto params = SharedTask::parseParams(cleanMsg, ctr[1], 2);
			if (params.size() > 0) {
				string log = "****[User: " + ctr[1] + "][Room: " + params[0] + "] " + params[1];
				cout << log << endl;
				storage->chatMsg += log;
				storage->chatMsg += "\n";
			}
		}
		else {
			//msgBuf.push({ ctr[0], srObj._payload });
			storage->msgBuf.push({ ctr[0], cleanMsg });
			//cout << srObj._payload << endl;
		}
		
	}
	cout << "Left thread" << endl;
}
int __cdecl main(int argc, char **argv)
{
	WSADATA wsaData;
	SOCKET ConnectSocket = INVALID_SOCKET;
	struct addrinfo *result = NULL,
		*ptr = NULL,
		hints;
	char recvbuf[DEFAULT_BUFLEN];
	int iResult;
	int recvbuflen = DEFAULT_BUFLEN;
	// Validate the parameters
	if (argc != 2) {
		printf("usage: %s server-name\n", argv[0]);
		return 1;
	}
	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		printf("WSAStartup failed with error: %d\n", iResult);
		return 1;
	}
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	// Resolve the server address and port
	iResult = getaddrinfo(argv[1], DEFAULT_PORT, &hints, &result);
	if (iResult != 0) {
		printf("getaddrinfo failed with error: %d\n", iResult);
		WSACleanup();
		return 1;
	}
	// Attempt to connect to an address until one succeeds
	for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {
		// Create a SOCKET for connecting to server
		ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype,
			ptr->ai_protocol);
		if (ConnectSocket == INVALID_SOCKET) {
			printf("socket failed with error: %ld\n", WSAGetLastError());
			WSACleanup();
			return 1;
		}
		// Connect to server.
		iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
		if (iResult == SOCKET_ERROR) {
			closesocket(ConnectSocket);
			ConnectSocket = INVALID_SOCKET;
			continue;
		}
		break;
	}
	freeaddrinfo(result);
	if (ConnectSocket == INVALID_SOCKET) {
		printf("Unable to connect to server!\n");
		WSACleanup();
		return 1;
	}
	std::cout << ">>> Enter your username to connect to IRC: ";
	std::string username;
	std::getline(std::cin, username, '\n');
	std::string str = CONNECT_REQ "@" + username + " ";
	int len = str.size() + PAYLOAD_LENGTH_DIGIT + 1;
	str = str + SharedTask::intToStr(len, PAYLOAD_LENGTH_DIGIT) + " ";
	const char* sendbuf = str.c_str();
	std::cout << "Connection request data: " << str << std::endl;
	SendRecvMsg srObj;
	srObj._payload = str;
	srObj._payloadLength = str.size();
	srObj._clientSock = &ConnectSocket;
	srObj.sendMsg();
	srObj.receive();
	//Check if successful connected
	std::cout << "Connection response data: " << srObj._payload << std::endl;
	//Create a background thread to get all incomming messages
	MessageStorage storage;
	thread t1(&collectMsg, &ConnectSocket, &storage);
	HandleMessages hm(&ConnectSocket, &srObj, &storage, username);
	hm.execute();
	return 0;
}
