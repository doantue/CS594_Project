#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <iostream>
#include "../IRC/Common.h"
#include "ProcessMsg.h"
#include "StringProcess.h"



ProcessMsg::ProcessMsg()
{
}


ProcessMsg::~ProcessMsg()
{
}
int ProcessMsg::receiveMsg() {
	int iResult;
	char payload[DEFAULT_BUFLEN];
	iResult = recv(*_clientSock, payload, DEFAULT_BUFLEN, 0);
	if (iResult > 0) {
		std::cout << "========= New Request =========" << std::endl;
		std::string str(payload);
		_payload = str;
		//_payload = str.substr(0,_payloadLength);
	}
	else if (iResult == 0) {
		std::cout << "Connection closed." << std::endl;
		return 1;
	}
	else {
		std::cout << "recv failed with error: " << WSAGetLastError() << std::endl;
		return 1;
	}
	return 0;
}

int ProcessMsg::sendMsg() {
	char* payload = new char[_payload.size()];
	//std::cout << "Payload size: " << _payload.size() << std::endl;
	std::copy(_payload.begin(), _payload.end(), payload);
	//std::cout << "Payload: " << payload << std::endl;
	int iResult = send(*_clientSock, payload, _payload.size(), 0);
	if (iResult == SOCKET_ERROR) {
		std::cout << "send failed with error: " << WSAGetLastError() << std::endl;
		closesocket(*_clientSock);
		//WSACleanup();
		return 1;
	}
	return 0;
}

int ProcessMsg::sendSuccessRes(std::string& user) {
	std::string str = SUCCESS_RES "@" + user + " ";
	int len = str.size() + PAYLOAD_LENGTH_DIGIT + 1;
	str = str + StringProcess::intToStr(len, PAYLOAD_LENGTH_DIGIT) + " ";
	const char* sendbuf = str.c_str();
	int iResult  = send(*_clientSock, sendbuf, str.size(), 0);
	if (iResult == SOCKET_ERROR) {
		std::cout << "send failed with error: " << WSAGetLastError() << std::endl;
		closesocket(*_clientSock);
		return 1;
	}
	std::cout << "Send to client: " << str << std::endl;
	return 0;
}

int ProcessMsg::sendErrorRes(std::string& user, std::string code) {
	std::string str = ERROR_RES "@" + user + " " ;
	int len = str.size() + PAYLOAD_LENGTH_DIGIT + 1 + ERR_CODE_LENGTH_DIGIT + 1;
	str = str + StringProcess::intToStr(len, PAYLOAD_LENGTH_DIGIT) + " " + code + " ";
	const char* sendbuf = str.c_str();
	int iResult = send(*_clientSock, sendbuf, str.size(), 0);
	if (iResult == SOCKET_ERROR) {
		std::cout << "send failed with error: " << WSAGetLastError() << std::endl;
		closesocket(*_clientSock);
		return 1;
	}
	std::cout << "Send to client: " << str << std::endl;
	return 0;
}