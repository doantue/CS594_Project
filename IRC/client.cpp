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
queue<pair<string, string>> msgBuf;
void collectMsg(SOCKET* connectSock, MessageStorage* storage) {
	SendRecvMsg srObj;
	srObj._clientSock = connectSock;
	while(1) {
		int iResult = srObj.receive();
		if (iResult == 1) break;
		auto ctr = SharedTask::parseCmdUser(srObj._payload);
		if (ctr.first == SEND_MSG_RES) {
			
			auto params = SharedTask::parseParams(srObj._payload, ctr.second, 2);
			if (params.size() > 0) {
				string log = "****[User: " + ctr.second + "][Room: " + params[0] + "] " + params[1];
				cout << log << endl;
				storage->chatMsg += log;
				storage->chatMsg += "\n";
			}
		}
		else {
			msgBuf.push({ ctr.first, srObj._payload });
			storage->msgBuf.push({ ctr.first, srObj._payload });
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
	/*
	while (1) {
		
		std::cout << " 1. Create a room." << std::endl;
		std::cout << " 2. List all rooms." << std::endl;
		std::cout << " 3. Join a room." << std::endl;
		std::cout << " 4. Leave a room." << std::endl;
		std::cout << " 5. List members of a room." << std::endl;
		std::cout << " 6. Send a message to a room." << std::endl;
		std::cout << " 7. Disconnect from the server." << std::endl;
		std::cout << ">>> Select next command: ";
		int num;
		while (std::cin >> num) {
			if (std::cin.get() == '\n') break;
		}
		switch (num)
		{
		case 1: {
			std::string roomName;
			do {
				std::cout << ">>> Enter room name: ";
				std::getline(std::cin, roomName, '\n');
				auto result = SharedTask::validateName(roomName);
				if (result.first == 0) break;
				std::cout << "===> ERROR: " << result.second << std::endl;
				roomName = "";
			} while (1);
			srObj._payload = CREATE_ROOM_REQ "@" + username + " " + roomName + " ";
			srObj._payloadLength = srObj._payload.size();
			srObj.sendMsg();
			//Timeout receive msg
			queue<pair<string, string>>* msgBuf = &storage.msgBuf;
			if (SharedTask::checkQueue(msgBuf, MSG_RECV_TIMEOUT)) {
				if (storage.msgBuf.size() > 0)
					cout << "Try object storage: " << storage.msgBuf.front().second << endl;
				auto data = msgBuf->front();
				msgBuf->pop();
				if(data.first== SUCCESS_RES)
					std::cout << "The room was created." << std::endl;
				else{
					vector<string> codes = SharedTask::parseParams(data.second, username, 1);
					std::cout << "Error happens, code: " << codes[0] << std::endl;
				}
			}
			else {
				//close connection
			}
			break;
		}
		case 2:{
			srObj._payload = LIST_ROOM_REQ "@" + username + " ";
			srObj._payloadLength = srObj._payload.size();
			srObj.sendMsg();
			queue<pair<string, string>>* msgBuf = &storage.msgBuf;
			if (SharedTask::checkQueue(msgBuf, MSG_RECV_TIMEOUT)) {
				auto data = msgBuf->front();
				msgBuf->pop();
				if (data.first == LIST_ROOMS_RES) {
					vector<std::string> list = SharedTask::parseList(data.second, username);
					cout << "===== List of Rooms =====" << std::endl;
					for each (auto room in list)
					{
						std::cout << "      " << room << std::endl;
					}
				}
				else {
					std::cout << "Error happens. " << std::endl;
				}
			}
			else {
				//close connection
			}
			break;
		}
		case 3: {
			std::cout << ">>> Enter a room name to join: ";
			std::string roomName;
			std::getline(std::cin, roomName, '\n');
			srObj._payload = JOIN_ROOM_REQ "@" + username + " " + roomName + " ";
			srObj._payloadLength = srObj._payload.size();
			srObj.sendMsg();
			srObj.receive();
			std::cout << "Response: " << srObj._payload << endl;
			break;
		}
		case 5: {
			std::string roomName;
			do {
				std::cout << ">>> Enter a room name to list: ";
				std::getline(std::cin, roomName, '\n');
				auto result = SharedTask::validateName(roomName);
				if (result.first == 0) break;
				std::cout << "===> ERROR: " << result.second << std::endl;
				roomName = "";
			} while (1);
			srObj._payload = LIST_MEMBER_REQ "@" + username + " " + roomName + " ";
			srObj._payloadLength = srObj._payload.size();
			srObj.sendMsg();		
			iResult = srObj.receive();
			if (iResult > 0) {
				auto ctr = SharedTask::parseCmdUser(srObj._payload);
				cout << "payload: " << srObj._payload << endl;
				if (ctr.first == LIST_MEMBERS_RES) {
					vector<string> list = SharedTask::parseList(srObj._payload, username);
					std::cout << "===== List of Members =====" << std::endl;
					for each (auto mem in list)
					{
						std::cout << "      " << mem << std::endl;
					}
				}
				else {
					vector<string> param = SharedTask::parseParams(srObj._payload, username, 1);
					if (param.size() > 0) {
						cout << "====> Error happens, code: " << param[0] << endl;
					}
				}
			}
			break;
		}
		case 6: {
			std::string roomName;
			do {
				std::cout << ">>> Enter a room name to send message: ";
				std::getline(std::cin, roomName, '\n');
				auto result = SharedTask::validateName(roomName);
				if (result.first == 0) break;
				std::cout << "===> ERROR: " << result.second << std::endl;
				roomName = "";
			} while (1);
			srObj._payload = SEND_MSG_REQ "@" + username + " 001 " + roomName + " ";
			srObj._payloadLength = srObj._payload.size();
			srObj.sendMsg();
			break;
		}
		case 7:
			srObj._payload = DISCONNECT_REQ "@" + username + " ";
			srObj._payloadLength = srObj._payload.size();
			srObj.sendMsg();
			// shutdown the connection since no more data will be sent
			iResult = shutdown(ConnectSocket, SD_SEND);
			if (iResult == SOCKET_ERROR) {
				std::cout << "Shutdown failed with error: " << WSAGetLastError() << std::endl;
				return 1;
			}
			// Receive until the peer closes the connection
			do {

				iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
				if (iResult > 0)
					printf("Bytes received: %d\n", iResult);
				else if (iResult == 0)
					printf("Connection closed\n");
				else
					printf("recv failed with error: %d\n", WSAGetLastError());

			} while (iResult > 0);
			closesocket(ConnectSocket);
			WSACleanup();
			return 0;
			break;
		default:
			break;
		}
	}
	*/
	return 0;
}
