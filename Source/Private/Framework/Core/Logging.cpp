#include "Framework/Core/Logging.h"

Logging::Logger* Logging::Logger::m_instance = getInstance();

Logging::Logger* Logging::Logger::getInstance()
{
	if (m_instance == nullptr)
	{
		m_instance = new Logger();
	}
	return m_instance;
}

int Logging::Logger::getCount(const ELogLevel inLevel)
{
	int count = 0;
	for (const auto& msgLevel : m_messages | std::views::values)
	{
		if (msgLevel == inLevel)
		{
			count++;
		}
	}
	return count;
}

std::vector<std::string> Logging::Logger::getMessages(const ELogLevel inLevel)
{
	std::vector<std::string> result;
	for (const auto& [Msg, MsgLevel] : m_messages)
	{
		if (MsgLevel == inLevel)
		{
			result.push_back(Msg);
		}
	}
	return result;
}
