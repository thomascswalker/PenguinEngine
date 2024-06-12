#include "Framework/Core/Logging.h"

Logging::Logger* Logging::Logger::Instance = GetInstance();

Logging::Logger* Logging::Logger::GetInstance()
{
    if (Instance == nullptr)
    {
        Instance = new Logger();
    }
    return Instance;
}

int Logging::Logger::GetCount(ELogLevel InLevel)
{
    int Count = 0;
    for (const auto& MsgLevel : Messages | std::views::values)
    {
        if (MsgLevel == InLevel)
        {
            Count++;
        }
    }
    return Count;
}

std::vector<std::string> Logging::Logger::GetMessages(ELogLevel InLevel)
{
    std::vector<std::string> Result;
    for (const auto& [Msg, MsgLevel] : Messages)
    {
        if (MsgLevel == InLevel)
        {
            Result.push_back(Msg);
        }
    }
    return Result;
}
