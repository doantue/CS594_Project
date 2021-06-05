#undef UNICODE

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <process.h>
#include <vector>
#include <iostream>
#include <string>
#include <unordered_map>
#include <sstream>
#include <iomanip>
#include <unordered_set>
#include "ProcessMsg.h"
#include "../IRC/Common.h"
#include "StringProcess.h"

// Need to link with Ws2_32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma once
// Define global variables
std::unordered_map<std::string, std::unordered_set<std::string>> clients;
std::unordered_map<std::string, std::unordered_set<std::string>> rooms;

unsigned __stdcall ClientSession(void *data)
{
	SOCKET ClientSocket = (SOCKET)data;
	//clients.push_back(&ClientSocket);
	struct sockaddr_in clientAddr;
	socklen_t slen = sizeof(clientAddr);
	if (getpeername(ClientSocket, (struct sockaddr *)&clientAddr, &slen) == 0) {
		char str[INET_ADDRSTRLEN];
		inet_ntop(AF_INET, &clientAddr.sin_addr, str, INET_ADDRSTRLEN);
		printf("client IP: %s\n", str);
	}
	// Process the client.
	int iResult;
	int iSendResult;
	char recvbuf[DEFAULT_BUFLEN];
	int recvbuflen = DEFAULT_BUFLEN;
	do {
		
		iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);
		if (iResult > 0) {
			std::cout << "========= New Request =========" << std::endl;
		}
		else if (iResult == 0)
			printf("Connection closing...\n");
		else {
			printf("recv failed with error: %d\n", WSAGetLastError());
			closesocket(ClientSocket);
			WSACleanup();
			return 1;
		}
		
		ProcessMsg prObj;
		prObj._clientSock = &ClientSocket;
		// Extract command
		std::string rawdata(recvbuf);
		std::string cmd = rawdata.substr(0, 3);
		std::cout << "Command: " << cmd << std::endl;
		// Extract user
		std::size_t posSpace = rawdata.find(' ');
		std::string user = "";
		if (posSpace != std::string::npos) {
			user = rawdata.substr(4, posSpace - 4);
		}
		std::cout << "User: " << user << std::endl;
		if (clients.find(user) == clients.end()) {
			clients.insert({ user, {} });
		}
		
		if (cmd == CONNECT_REQ) {
			std::string strsend = CONNECT_RES "@" + user + " ";
			const char* sendbuf = strsend.c_str();
			prObj._payload = sendbuf;
			prObj._payloadLength = strsend.size();
			if (prObj.sendMsg() == 1) break;
			std::cout << "Send to client: " << strsend << std::endl;
		}
		else if (cmd == CREATE_ROOM_REQ) {
			int headLen = user.size() + 5;
			std::string params = rawdata.substr(headLen);
			posSpace = params.find(' ');
			const std::string roomName = params.substr(0, posSpace);
			// Room name invalid
			if (rooms.find(roomName) != rooms.end()) {
				std::string strsend = ERROR_RES "@" + user + " " ERR_ROOM_EXIST;
				strsend += " ";
				const char* sendbuf = strsend.c_str();
				prObj._payload = sendbuf;
				prObj._payloadLength = strsend.size();
				if (prObj.sendMsg() == 1) break;
				std::cout << "Send error code to client: " << ERR_ROOM_INV << std::endl;
			}
			else {
				rooms[roomName] = {user};
				clients[user].insert(roomName);
				std::cout << "Room name: " << roomName << std::endl;
				std::string strsend = SUCCESS_RES "@" + user + " ";
				const char* sendbuf = strsend.c_str();
				prObj._payload = sendbuf;
				prObj._payloadLength = strsend.size();
				if (prObj.sendMsg() == 1) break;
				std::cout << "Send to client: " << strsend << std::endl;
			}
		}
		else if (cmd == LIST_ROOM_REQ) {
			std::stringstream sstr;
			sstr << std::setw(NUM_LIST_LENGTH) << std::setfill('0') << rooms.size();
			std::string numRooms = sstr.str();
			std::string strsend = LIST_ROOMS_RES "@" + user + " " + numRooms + " ";
			for (auto it = rooms.begin(); it != rooms.end(); it++) {
				strsend = strsend + it->first + " ";
			}
			const char* sendbuf = strsend.c_str();
			prObj._payload = sendbuf;
			prObj._payloadLength = strsend.size();
			if (prObj.sendMsg() == 1) break;
			std::cout << "Send to client: " << strsend << std::endl;
		}
		else if (cmd == JOIN_ROOM_REQ) {
			vector<string> param = StringProcess::parseParams(rawdata, user, 1);
			if (param.size() > 0) {
				string jname = param[0];
				cout << jname << " " <<jname.size() << endl;
				if (rooms.find(jname) != rooms.end()) {
					rooms[jname].insert(user);
					clients[user].insert(jname);
					prObj.sendSuccessRes(user);
				}
				else {
					prObj.sendErrorRes(user, ERR_ROOM_N_EXIST);
				}
			}
			else {
				prObj.sendErrorRes(user, ERR_DATA_CORRUPT);
			}
		}
		else if (cmd == LEAVE_ROOM_REQ) {

		}
		else if (cmd == LIST_MEMBER_REQ) {
			vector<string> param = StringProcess::parseParams(rawdata, user, 1);
			if (param.size() > 0) {
				string lname = param[0];
				if (rooms.find(lname) != rooms.end()) {
					std::stringstream sstr;
					sstr << std::setw(NUM_LIST_LENGTH) << std::setfill('0') << rooms[lname].size();
					std::string numUsers = sstr.str();
					std::string strsend = LIST_MEMBERS_RES "@" + user + " " + numUsers + " ";
					auto memList = rooms[lname];
					for (auto it = memList.begin(); it != memList.end(); it++) {
						strsend = strsend + *it + " ";
					}
					const char* sendbuf = strsend.c_str();
					prObj._payload = sendbuf;
					prObj._payloadLength = strsend.size();
					if (prObj.sendMsg() == 1) break;
					std::cout << "Send to client: " << strsend << std::endl;
				}
				else {
					prObj.sendErrorRes(user, ERR_ROOM_N_EXIST);
				}
			}
			else {
				prObj.sendErrorRes(user, ERR_DATA_CORRUPT);
			}
		}
		else if (cmd == SEND_MSG_REQ) {

		}
		else if (cmd == DISCONNECT_REQ) {
			clients.erase(user);
			for (auto it = rooms.begin(); it != rooms.end(); it++) {
				if (it->second.find(user) != it->second.end()) {
					it->second.erase(user);
				}
			}
			closesocket(ClientSocket);
			//WSACleanup();
			std::cout << "Delete connection with user: " << user << std::endl;
			return 0;
		}
		else if (cmd == HEARTBEAT_REQ) {

		}
		std::cout << "Number of connections: " << clients.size() << std::endl;
	} while (iResult > 0);
	return 0;
}



int __cdecl main(void)
{
	printf("start IRC.\n");
	WSADATA wsaData;
	int iResult;

	SOCKET ListenSocket = INVALID_SOCKET;
	SOCKET ClientSocket = INVALID_SOCKET;

	struct addrinfo *result = NULL;
	struct addrinfo hints;

	int iSendResult;
	char recvbuf[DEFAULT_BUFLEN];
	int recvbuflen = DEFAULT_BUFLEN;

	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		printf("WSAStartup failed with error: %d\n", iResult);
		return 1;
	}

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	// Resolve the server address and port
	iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
	if (iResult != 0) {
		printf("getaddrinfo failed with error: %d\n", iResult);
		WSACleanup();
		return 1;
	}

	// Create a SOCKET for connecting to server
	ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (ListenSocket == INVALID_SOCKET) {
		printf("socket failed with error: %ld\n", WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		return 1;
	}

	// Setup the TCP listening socket
	iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
	if (iResult == SOCKET_ERROR) {
		printf("bind failed with error: %d\n", WSAGetLastError());
		freeaddrinfo(result);
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}

	freeaddrinfo(result);
	while(1){
		iResult = listen(ListenSocket, SOMAXCONN);
		if (iResult == SOCKET_ERROR) {
			printf("listen failed with error: %d\n", WSAGetLastError());
			closesocket(ListenSocket);
			WSACleanup();
			return 1;
		}
		//SOCKET ClientSocket;
		struct sockaddr_in clientAddr;
		socklen_t slen = sizeof(clientAddr);
		//while (ClientSocket = accept(ListenSocket, (struct sockaddr *)&clientAddr, &slen)) {
		ClientSocket = accept(ListenSocket, (struct sockaddr *)&clientAddr, &slen);
		if (ClientSocket == INVALID_SOCKET) {
			std::cout << "accept failed with error: " << WSAGetLastError() << std::endl;
			closesocket(ListenSocket);
			WSACleanup();
			break;
		}
		// Create a new thread for the accepted client (also pass the accepted client socket).
		unsigned threadID;
		HANDLE hThread = (HANDLE)_beginthreadex(NULL, 0, &ClientSession, (void*)ClientSocket, 0, &threadID);
		std::cout << "Thread ID: " << threadID << std::endl;
	}
	
	return 0;
}