
#include <Shlwapi.h>
#include <string>

#include "../deps/osw/Steamworks.h"
#include "../deps/osw/ISteamUser017.h"
#include "commandline.h"

CommandLine::CommandLine()
{
	this->args.clear();
}

void CommandLine::InitParams(const char* argv[], int argc)
{
	for (int i = 0; i < argc; i++)
	{
		this->args.push_back(argv[i]);
	}
}


bool CommandLine::GetParameterInternal(std::string param, std::string* fill, bool optional)
{
	bool fndparam = false;

	for (auto pos = this->args.begin(); pos != this->args.end(); ++pos)
	{
		std::string arg = *pos;

		if (fndparam)
		{
			fill->clear();
			fill->append(arg);
			return true;
		}

		if (arg == param)
		{
			fndparam = true;
			continue;
		}
	}

	if (!optional)
	{
		printf("Could not find parameter: %s\n", param.c_str());
		exit(1);
	}

	return false;
}

int CommandLine::GetParameterNumber(const char* param, bool optional, int defaultvalue)
{
	std::string stringparam = "";
	bool found = GetParameterInternal(param, &stringparam, optional);

	if (!found && optional)
	{
		return defaultvalue;
	}

	return atoi(stringparam.c_str());
}

std::string CommandLine::GetParameterString(const char* param, bool optional, std::string defaultvalue)
{
	std::string stringparam = "";
	bool found = GetParameterInternal(param, &stringparam, optional);

	if (!found && optional)
	{
		return defaultvalue;
	}

	return  stringparam;
}