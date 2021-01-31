#include <string>
#include <stdint.h>
#include <vector>

class CommandLine
{
public:
	CommandLine();

	void InitParams(const char* argv[], int argc);


	int GetParameterNumber(const char* param, bool optional = false, int defaultvalue = 0);
	std::string GetParameterString(const char* param, bool optional = false, std::string defaultvalue = "");

private:
	bool CommandLine::GetParameterInternal(std::string param, std::string* fill, bool optional = false);
	std::vector<std::string> args;
};