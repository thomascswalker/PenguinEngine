#pragma once

#include <string>
#include <vector>

#include "Math/MathFwd.h"

namespace Strings
{
	static void split(const std::string& inString, std::vector<std::string>& outStrings, const std::string& delimiter)
	{
		outStrings.clear();

		std::string temp;
		int32		stringSize = static_cast<int32>(inString.size());
		for (int32 index = 0; index < stringSize; index++)
		{
			if (inString.substr(index, delimiter.size()) == delimiter)
			{
				if (!temp.empty())
				{
					outStrings.push_back(temp);
					temp.clear();
					index += static_cast<int32>(delimiter.size()) - 1;
				}
			}
			else if (index + delimiter.size() >= inString.size())
			{
				temp += inString.substr(index, delimiter.size());
				outStrings.push_back(temp);
				break;
			}
			else
			{
				temp += inString[index];
			}
		}
	}

	static bool contains(const std::string& inString, const char* c)
	{
		return inString.find(c) != std::string::npos;
	}

	static bool contains(const std::string& inString, const std::string& otherString)
	{
		return inString.find(otherString) != std::string::npos;
	}

	inline std::wstring toWString(const std::string& str)
	{
		return { str.begin(), str.end() };
	}
} // namespace Strings
