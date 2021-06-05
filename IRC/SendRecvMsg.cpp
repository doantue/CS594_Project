#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <iostream>
#include "SendRecvMsg.h"
#include "Common.h"

SendRecvMsg::SendRecvMsg()
{

}


SendRecvMsg::~SendRecvMsg()
{
}

int SendRecvMsg::receive() {

	int iResult;
	char payload[DEFAULT_BUFLEN];
	iResult = recv(*_clientSock, payload, _payloadLength, 0);
	std::cout << "iResult :" << iResult << std::endl;
	if (iResult > 0) {
		std::cout << "========= New Request =========" << std::endl;
		std::string str(payload);
		_payload = str;
	}
	else if (iResult == 0) {
		std::cout << "Connection closed." << std::endl;
		return 1;
	}
	else {
		std::cout << "recv failed with error: " << WSAGetLastError() << std::endl;
		return 1;
	}
	return iResult;
}

int SendRecvMsg::sendMsg() {
	char* payload = new char[_payload.size()];
	std::copy(_payload.begin(), _payload.end(), payload);
	int iResult = send(*_clientSock, payload, _payload.size(), 0);
	if (iResult == SOCKET_ERROR) {
		std::cout << "send failed with error: " << WSAGetLastError() << std::endl;
		closesocket(*_clientSock);
		WSACleanup();
		return 1;
	}
	return 0;
}