#pragma once
using namespace std;
class StringProcess
{
public:
	StringProcess();
	~StringProcess();
	static pair<string, string> parseCmdUser(string& data);
	static vector<string> parseList(string& data, string& user);
	static vector<string> parseParams(string& data, string& user, int numParams);
	static pair<int, string> validateName(string& name);
	static void printMap(unordered_map<string, unordered_set<string>> m);
};

