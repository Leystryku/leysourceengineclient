#pragma once

#include <string>

class netmsg_common
{
private:
	netmsg_common();

public:
	netmsg_common(std::string name, int msgtype, int mininumremainingsize)
	{
		this->name = name;
		this->msgtype = msgtype;
		this->mininumremainingsize = mininumremainingsize;
	}

	std::string GetName()
	{
		return name;
	}

	int GetMsgType()
	{
		return msgtype;
	}

	int GetMinimumRemainingSize()
	{
		return mininumremainingsize;
	}

	inline bool LengthTooSmall(int length)
	{
		if (GetMinimumRemainingSize() > length)
		{
			return true;
		}

		return false;
	}
private:
	std::string name;
	int msgtype;
	int mininumremainingsize;
};