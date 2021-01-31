#pragma once

#include <string>

class netmsg_common
{
private:
	netmsg_common();

public:
	netmsg_common(std::string name, int msgtype)
	{
		this->name = name;
		this->msgtype = msgtype;
	}

	std::string GetName()
	{
		return name;
	}

	int GetMsgType()
	{
		return msgtype;
	}

private:
	int msgtype;
	std::string name;
};