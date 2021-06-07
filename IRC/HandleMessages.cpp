#include <winsock2.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include <istream>
#include <vector>
#include <queue>
#include <thread>
#include <sstream>
#include <iomanip>
#include "SendRecvMsg.h"
#include "Common.h"
#include "SharedTask.h"
#include "MessageStorage.h"
#include "HandleMessages.h"



HandleMessages::HandleMessages(SOCKET* sock, SendRecvMsg* sr, MessageStorage* _storage, string _username)
{
	ConnectSocket = sock;
	srObj = sr;
	storage = _storage;
	username = _username;
}



int HandleMessages::execute() {
	char recvbuf[DEFAULT_BUFLEN];
	int iResult;
	int recvbuflen = DEFAULT_BUFLEN;
	while (1) {
		int iResult;
		cout << endl;
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
			this->createRoom();
			break;
		}
		case 2: {
			auto result = this->listRoom();
			if (!result.first) {
				cout << "===== List of Rooms =====" << std::endl;
				for each (string room in result.second){
					std::cout << "      " << room << std::endl;
				}
			}
			else {
				std::cout << "===> ERROR: "<< result.second[0] << std::endl;
			}
			break;
		}
		case 3: {
			this->joinRoom();
			break;
		}
		case 5: {
			this->listMembers();
			break;
		}
		case 6: {
			this->sendMessage();
			break;
		}
		case 7: {
			string strsend = DISCONNECT_REQ "@" + username + " ";
			int len = strsend.size() + PAYLOAD_LENGTH_DIGIT + 1;
			strsend = strsend + SharedTask::intToStr(len, PAYLOAD_LENGTH_DIGIT) + " ";
			srObj->_payload = strsend;
			srObj->_payloadLength = srObj->_payload.size();
			srObj->sendMsg();
			//shutdown the connection since no more data will be sent
			iResult = shutdown(*ConnectSocket, SD_SEND);
			if (iResult == SOCKET_ERROR) {
				std::cout << "Shutdown failed with error: " << WSAGetLastError() << std::endl;
				return 1;
			}
			closesocket(*ConnectSocket);
			WSACleanup();
			return 0;
			break;
		}
		default:
			break;
		}
	}
}

void HandleMessages::createRoom() {
	std::string roomName;
	do {
		std::cout << ">>> Enter room name to create: ";
		std::getline(std::cin, roomName, '\n');
		auto result = SharedTask::validateName(roomName);
		if (result.first == 0) break;
		std::cout << "===> ERROR: " << result.second << std::endl;
		roomName = "";
	} while (1);
	string strsend = CREATE_ROOM_REQ "@" + username + " ";
	string rstr = roomName + " ";
	int len = strsend.size() + PAYLOAD_LENGTH_DIGIT + 1 + rstr.size();
	strsend = strsend + SharedTask::intToStr(len, PAYLOAD_LENGTH_DIGIT) + " " + rstr;
	srObj->_payload = strsend;
	srObj->_payloadLength = srObj->_payload.size();
	srObj->sendMsg();
	//Timeout receive msg
	if (SharedTask::checkQueue(&storage->msgBuf, MSG_RECV_TIMEOUT)) {
		auto data = storage->msgBuf.front();
		storage->msgBuf.pop();
		if (data.first == SUCCESS_RES)
			std::cout << "===> INFO: The room was created." << std::endl;
		else {
			vector<string> codes = SharedTask::parseParams(data.second, username, 1);
			std::cout << "===> ERROR, code: " << codes[0] << std::endl;
		}
	}
	else {
		//close connection
	}
}

pair<int, vector<string>> HandleMessages::listRoom() {
	string strsend = LIST_ROOM_REQ "@" + username + " ";
	int len = strsend.size() + PAYLOAD_LENGTH_DIGIT + 1;
	strsend = strsend + SharedTask::intToStr(len, PAYLOAD_LENGTH_DIGIT);
	srObj->_payload = strsend;
	srObj->_payloadLength = srObj->_payload.size();
	srObj->sendMsg();
	if (SharedTask::checkQueue(&storage->msgBuf, MSG_RECV_TIMEOUT)) {
		auto data = storage->msgBuf.front();
		storage->msgBuf.pop();
		if (data.first == LIST_ROOMS_RES) {
			vector<std::string> list = SharedTask::parseList(data.second, username);
			return{ 0, list };
		}
		else {
			return{ 1, {"unknown error happens."} };
		}
	}
	else {
		//close connection
	}
}

void HandleMessages::joinRoom() {
	std::string roomName;
	do {
		std::cout << ">>> Enter room name to join: ";
		std::getline(std::cin, roomName, '\n');
		auto result = SharedTask::validateName(roomName);
		if (result.first == 0) break;
		std::cout << "===> ERROR: " << result.second << std::endl;
		roomName = "";
	} while (1);
	string strsend = JOIN_ROOM_REQ "@" + username + " ";
	string rstr = roomName + " ";
	int len = strsend.size() + PAYLOAD_LENGTH_DIGIT + 1 + rstr.size();
	strsend = strsend + SharedTask::intToStr(len, PAYLOAD_LENGTH_DIGIT) + " " + rstr;
	srObj->_payload = strsend;
	srObj->_payloadLength = srObj->_payload.size();
	srObj->sendMsg();
	if (SharedTask::checkQueue(&storage->msgBuf, MSG_RECV_TIMEOUT)) {
		auto data = storage->msgBuf.front();
		storage->msgBuf.pop();
		if (data.first == SUCCESS_RES)
			std::cout << "===> INFO: You've joined the room " << roomName << "." << std::endl;
		else {
			vector<string> codes = SharedTask::parseParams(data.second, username, 1);
			std::cout << "===> ERROR, code: " << codes[0] << std::endl;
		}
	}
	else {
		//close connection
	}
}

void HandleMessages::sendMessage() {
	auto roomList = this->listRoom();
	if (roomList.first) {
		std::cout << "===> ERROR: " << roomList.second[0] << std::endl;
	}
	else {
		if (roomList.second.size() == 0) {
			cout << "===> ERROR: No room existed. " << endl;
		}
		else {
			cout << "===== List of Rooms =====" << std::endl;
			for (int i = 0; i < roomList.second.size(); i++) {
				std::cout << "      "<< i+1 <<". " << roomList.second[i] << std::endl;
			}
		}
	}
	string rooms;
	static pair<int, string> result;
	do {
		std::cout << ">>> Enter room number(s), seperated by space char: ";
		std::getline(std::cin, rooms, '\n');
		result = SharedTask::validateRoomList(rooms, roomList.second);
		if (result.first != 0) {
			rooms = result.second;
			break;
		}
		std::cout << "===> ERROR: " << result.second << std::endl;
		rooms = "";
	} while (1);
	string content = "";
	cout << ">>> Message content: ";
	std::getline(std::cin, content, '\n');
	std::string numRooms = SharedTask::intToStr(++result.first, NUM_LIST_LENGTH_DIGIT);
	string strsend = SEND_MSG_REQ "@" + username + " ";
	string rstr = numRooms + " " + rooms + " " + content + " ";
	int len = strsend.size() + PAYLOAD_LENGTH_DIGIT + 1 + rstr.size();
	strsend = strsend + SharedTask::intToStr(len, PAYLOAD_LENGTH_DIGIT) + " " + rstr;
	srObj->_payload = strsend;
	srObj->_payloadLength = srObj->_payload.size();
	srObj->sendMsg();
	if (SharedTask::checkQueue(&storage->msgBuf, MSG_RECV_TIMEOUT)) {
		auto data = storage->msgBuf.front();
		storage->msgBuf.pop();
		if (data.first == SUCCESS_RES)
			std::cout << "===> INFO: Your message was sent " << std::endl;
		else {
			vector<string> codes = SharedTask::parseParams(data.second, username, 1);
			std::cout << "===> ERROR, code: " << codes[0] << std::endl;
		}
	}
	else {
		//close connection
	}
}

void HandleMessages::listMembers() {
	std::string roomName;
	do {
		std::cout << ">>> Enter a room name to list: ";
		std::getline(std::cin, roomName, '\n');
		auto result = SharedTask::validateName(roomName);
		if (result.first == 0) break;
		std::cout << "===> ERROR: " << result.second << std::endl;
		roomName = "";
	} while (1);
	string strsend = LIST_MEMBER_REQ "@" + username + " ";
	string rstr = roomName + " ";
	int len = strsend.size() + PAYLOAD_LENGTH_DIGIT + 1 + rstr.size();
	strsend = strsend + SharedTask::intToStr(len, PAYLOAD_LENGTH_DIGIT) + " " + rstr;
	srObj->_payload = strsend;
	srObj->_payloadLength = srObj->_payload.size();
	srObj->sendMsg();
	if (SharedTask::checkQueue(&storage->msgBuf, MSG_RECV_TIMEOUT)) {
		auto data = storage->msgBuf.front();
		storage->msgBuf.pop();
		if (data.first == LIST_MEMBERS_RES) {
			vector<std::string> list = SharedTask::parseList(data.second, username);
			std::cout << "===== List of Members =====" << std::endl;
			for each (auto mem in list){
				std::cout << "      " << mem << std::endl;
			}
		}
		else {
			vector<string> codes = SharedTask::parseParams(data.second, username, 1);
			std::cout << "===> ERROR, code: " << codes[0] << std::endl;
		}
	}
	else {
		//close connection
	}
}