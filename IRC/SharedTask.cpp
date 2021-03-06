#include <string>
#include <vector>
#include <iostream>
#include <queue>
#include <thread>
#include <sstream>
#include <iomanip>
#include "Common.h"
#include "SharedTask.h"



SharedTask::SharedTask()
{
}


SharedTask::~SharedTask()
{
}

vector<string> SharedTask::parseCmdUser(string& data) {
	string cmd = data.substr(0, COMMAND_LENGTH_DIGIT);
	size_t posSpace = data.find(' ');
	string user = "";
	string len = "";
	if (posSpace != string::npos) {
		user = data.substr(COMMAND_LENGTH_DIGIT + 1, posSpace - (COMMAND_LENGTH_DIGIT + 1));
		int posLen = COMMAND_LENGTH_DIGIT + 1 + user.size() + 1;
		len = data.substr(posLen, PAYLOAD_LENGTH_DIGIT);
	}
	return{ cmd, user, len };
}

vector<string> SharedTask::parseList(string& data, string& user) {
	vector<string> ret;
	int pos = user.size() + COMMAND_LENGTH_DIGIT + PAYLOAD_LENGTH_DIGIT + 3; // 3:  @ + " " + " "
	string numstr = data.substr(pos, NUM_LIST_LENGTH_DIGIT);
	int num = stoi(numstr);
	string list = data.substr(pos + NUM_LIST_LENGTH_DIGIT + 1);
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

vector<string> SharedTask::parseParams(string& data, string& user, int numParams) {
	vector<string> ret;
	int pos = user.size() + COMMAND_LENGTH_DIGIT + PAYLOAD_LENGTH_DIGIT + 3; // user length + 3 command code + @ + " "
	//cout << data << " " << data.size() << endl;
	string list = data.substr(pos);
	if (numParams == 1) {
		auto posSpace = list.find(' ');
		if (posSpace != string::npos) {
			ret.push_back(list.substr(0, posSpace));
		}
	}
	else {
		//cout << list << endl;
		for (auto i = 0; i < numParams - 1; i++) {
			auto posSpace = list.find(' ');
			if (posSpace != string::npos) {
				ret.push_back(list.substr(0, posSpace));
				if (posSpace >= list.size()) break;
				list = list.substr(posSpace + 1);
			}
			else break;
		}
		if (list.size() > 0) ret.push_back(list);
	}
	return ret;
}

pair<int, string> SharedTask::validateName(string& name) {
	//cout << "name length " << name.size() << endl;
	pair<int, string> ret = {0, ""};
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

pair<int, string> SharedTask::validateRoomList(string rooms, vector<string> roomlist) {
	pair<int, string> ret = { 0, "" };
	int maxRoom = roomlist.size();
	string list = "";
	string delimiter = MESSAGE_DELIMITER;
	size_t pos = 0;
	string roomNum;
	while ((pos = rooms.find(delimiter)) != string::npos) {
		roomNum = rooms.substr(0, pos);
		//std::cout << token << std::endl;
		int num = stoi(roomNum);
		if (num <= maxRoom && num > 0) {
			list += (roomlist[num-1] + delimiter);
			ret.first++;
		}
		else {
			ret.first = 0;
			ret.second = "Invalid number.";
			return ret;
		}
		rooms.erase(0, pos + delimiter.length());
	}
	int num = stoi(rooms);
	if (num > 0 && num <= maxRoom) {
		list += roomlist[num - 1];
		ret.first++;
	}
	else {
		ret.first = 0;
		ret.second = "Invalid number.";
		return ret;
	}
	ret.second = list;
	return ret;
}

bool SharedTask::checkQueue(queue<pair<string, string>>* buf, int waitTime) {
	int rep = 2 * waitTime;
	for (int i = 0; i < rep; i++) {
		if (!buf->empty()) return true;
		this_thread::sleep_for(std::chrono::milliseconds(500));
	}
	return false;
}

string SharedTask::intToStr(int num, int paddingLength) {
	stringstream sstr;
	sstr << std::setw(paddingLength) << std::setfill('0') << num;
	return sstr.str();
}