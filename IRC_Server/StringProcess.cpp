#include <string>
#include <vector>
#include <iostream>
#include "StringProcess.h"



StringProcess::StringProcess()
{
}


StringProcess::~StringProcess()
{
}
pair<string, string> StringProcess::parseCmdUser(string& data) {
	string cmd = data.substr(0, 3);
	size_t posSpace = data.find(' ');
	string user = "";
	if (posSpace != string::npos) {
		user = data.substr(4, posSpace - 4);
	}
	return{ cmd, user };
}

vector<string> StringProcess::parseList(string& data, string& user) {
	vector<string> ret;
	int pos = user.size() + 5; // user length + 3 command code + @ + " "
	string numstr = data.substr(pos, 3);
	int num = stoi(numstr);
	string list = data.substr(pos + 4);
	for (auto i = 0; i < num; i++) {
		auto posSpace = list.find(' ');
		if (posSpace != string::npos) {
			ret.push_back(list.substr(0, posSpace));
			list = list.substr(posSpace + 1);
		}
		else break;
	}
	return ret;
}

vector<string> StringProcess::parseParams(string& data, string& user, int numParams) {
	vector<string> ret;
	int pos = user.size() + 5; // user length + 3 command code + @ + " "
							   //cout << data << " " << data.size() << endl;
	string list = data.substr(pos);
	//cout << list << endl;
	for (auto i = 0; i < numParams; i++) {
		auto posSpace = list.find(' ');
		if (posSpace != string::npos) {
			ret.push_back(list.substr(0, posSpace));
			if (posSpace >= list.size()) break;
			list = list.substr(posSpace + 1);
		}
		else break;
	}
	return ret;
}

pair<int, string> StringProcess::validateName(string& name) {
	cout << "name length " << name.size() << endl;
	pair<int, string> ret = { 0, "" };
	if (name.size() == 0) {
		ret.first = 1;
		ret.second = "Name is not empty.";
	}
	else if (name.find(' ') != string::npos) {
		ret.first = 1;
		ret.second = "Name does not contain space character.";
	}
	else if (name.size() > 20) {
		ret.first = 1;
		ret.second = "Name is not longer than 20 characters.";
	}
	return ret;
}