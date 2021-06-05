#ifndef SENDRECVMSG_H_
#define SENDRECVMSG_H_

class SendRecvMsg
{
public:
	SendRecvMsg();
	~SendRecvMsg();
	int sendMsg();
	int receive();
	int _payloadLength;
	std::string _payload;
	SOCKET* _clientSock;
};

#endif /* SENDRECVMSG_H_ */