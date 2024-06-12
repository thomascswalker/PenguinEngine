#pragma once

#include <string>
#include <vector>

#include "Math/MathFwd.h"

namespace Strings
{
    static void Split(const std::string& InString, std::vector<std::string>& OutStrings, const std::string& Delimiter)
    {
        OutStrings.clear();

        std::string Temp;
        int32 StringSize = static_cast<int32>(InString.size());
        for (int32 Index = 0; Index < StringSize; Index++)
        {
            if (InString.substr(Index, Delimiter.size()) == Delimiter)
            {
                if (!Temp.empty())
                {
                    OutStrings.push_back(Temp);
                    Temp.clear();
                    Index += static_cast<int32>(Delimiter.size()) - 1;
                }
            }
            else if (Index + Delimiter.size() >= InString.size())
            {
                Temp += InString.substr(Index, Delimiter.size());
                OutStrings.push_back(Temp);
                break;
            }
            else
            {
                Temp += InString[Index];
            }
        }
    }

    static bool Contains(const std::string& InString, const char* Char)
    {
        return InString.find(Char) != std::string::npos;
    }

    static bool Contains(const std::string& InString, const std::string& CompareString)
    {
        return InString.find(CompareString) != std::string::npos;
    }
}
