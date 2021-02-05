#pragma once

#include <memory>
#include <vector>
#include <string>
#include <time.h>


class SplitPacketPart
{
public:
	int length;
	int offset;
	std::string data;

	SplitPacketPart()
	{
		length = 0;
		offset = 0;
		data = "";
	}

};

class SplitPacket
{
private:
	std::vector<SplitPacketPart> parts;
public:
	SplitPacket()
	{
		parts.clear();
		updateTime = 0;
		sequenceNumber = 0;
		totalExpectedSize = 0;
		expectedPartsCount = 0;
	}
	unsigned int updateTime;
	int sequenceNumber;
	int totalExpectedSize;
	int expectedPartsCount;

	int GetPartsCount()
	{
		return parts.size();
	}

	int GetCurrentPacketSize()
	{
		int size = 0;

		if (parts.size() > 0)
		{
			for (const auto data : parts)
			{
				size = size + data.length;
			}

		}

		return size;
	}

	bool HasProperParts()
	{
		return GetPartsCount() == expectedPartsCount && expectedPartsCount > 0;
	}

	bool HasProperSize()
	{
		return GetCurrentPacketSize() == totalExpectedSize && totalExpectedSize > 0;
	}

	bool IsComplete()
	{
		return HasProperParts() && HasProperSize();
	}

	void InsertPart(int offset, int length, char* data)
	{
		SplitPacketPart  newPart;
		newPart.length = length;
		newPart.offset = offset;
		for (int i = 0; i < length; i++)
		{
			newPart.data.push_back(data[i]);
		}

		parts.push_back(newPart);
		this->updateTime = (unsigned int)time(NULL);
	}

	bool IsOld()
	{
		return this->updateTime > 0 && (((unsigned int)time(NULL) - this->updateTime) > 10);
	}

	char* CreateAssembledPacket()
	{
		if (!IsComplete())
			return 0;

		char* pkg = new char[totalExpectedSize];
		memset(pkg, 0, totalExpectedSize);

		for (auto data : parts)
		{
			memcpy(pkg + data.offset, data.data.c_str(), data.length);
		}

		return pkg;
	}

	void Reset()
	{
		updateTime = 0;
		sequenceNumber = 0;
		totalExpectedSize = 0;
		expectedPartsCount = 0;

		for (auto data : parts)
		{
			data.data = "";
		}

		if (parts.empty())
			return;

		parts.clear();
	}
};