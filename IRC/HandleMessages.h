#pragma once
class HandleMessages
{
public:
	HandleMessages(SOCKET* sock, SendRecvMsg* sr, MessageStorage* _storage, string _username);
	int execute();
private:
	SOCKET* ConnectSocket;
	SendRecvMsg* srObj;
	MessageStorage* storage;
	string username;
	void createRoom();
	pair<int, vector<string>> listRoom();
	void joinRoom();
	void sendMessage();
	void listMembers();
};

