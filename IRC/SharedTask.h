#pragma once
using namespace std;
class SharedTask
{
public:
	SharedTask();
	~SharedTask();
	static vector<string> parseCmdUser(string& data);
	static vector<string> parseList(string& data, string& user);
	static vector<string> parseParams(string& data, string& user, int numParams);
	static pair<int, string> validateName(string& name);
	static pair<int, string> validateRoomList(string rooms, vector<string> roomlist);
	static bool checkQueue(queue<pair<string, string>>* buf, int waitTime);
	static string intToStr(int num, int paddingLength);
};

