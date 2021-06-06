#pragma once
using namespace std;
class MessageStorage
{
public:
	MessageStorage();
	~MessageStorage();
	queue<pair<string, string>> msgBuf;
	string chatMsg;
};

