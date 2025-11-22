#include "Logger.hpp"

#include <print>

namespace Earth
{
    namespace
    {
        Logger::Level s_Level = Logger::Level::Info;
    }

    void Logger::SetGlobalLevel(Level level)
    {
        s_Level = level;
    }

    Logger::Logger(std::string_view name, Level level) : m_Name(name), m_Level(level)
    {
    }

    Logger::~Logger()
    {
    }

    void Logger::Log(std::string_view level, std::string_view message)
    {
        if (static_cast<int>(m_Level) >= static_cast<int>(s_Level))
        {
            std::println("[{}] [{}] {}", level, m_Name, message);
        }
    }
}
