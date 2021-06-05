#pragma once
class ProcessMsg
{
public:
	ProcessMsg();
	~ProcessMsg();
	int sendMsg();
	int receiveMsg();
	int sendSuccessRes(std::string& user);
	int sendErrorRes(std::string& user, std::string code);
	int _payloadLength;
	std::string _payload;
	SOCKET* _clientSock;
};

