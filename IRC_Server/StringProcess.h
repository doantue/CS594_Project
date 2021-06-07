#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
using namespace std;
class StringProcess
{
public:
	StringProcess();
	~StringProcess();
	static vector<string> parseCmdUser(string& data);
	static vector<string> parseList(string& data, string& user);
	static vector<string> parseParams(string& data, string& user, int numParams);
	static pair<int, string> validateName(string& name);
	static void printMap(unordered_map<string, unordered_set<string>> m);
	static string intToStr(int num, int paddingLength);
};

