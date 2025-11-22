#pragma once

#include <format>
#include <string>

namespace Earth
{
    class Logger
    {
      public:
        enum class Level
        {
            Debug,
            Info,
            Warn,
            Error,
            Fatal
        };

        static void SetGlobalLevel(Level level);

        Logger(std::string_view name, Level level = Level::Info);

        template <typename... Args>
        void Debug(std::string_view format, Args&&... args)
        {
            Log("Debug", std::vformat(format, std::make_format_args(args...)));
        }

        template <typename... Args>
        void Info(std::string_view format, Args&&... args)
        {
            Log("Info", std::vformat(format, std::make_format_args(args...)));
        }

        template <typename... Args>
        void Warn(std::string_view format, Args&&... args)
        {
            Log("Warn", std::vformat(format, std::make_format_args(args...)));
        }

        template <typename... Args>
        void Error(std::string_view format, Args&&... args)
        {
            Log("Error", std::vformat(format, std::make_format_args(args...)));
        }

        template <typename... Args>
        void Fatal(std::string_view format, Args&&... args)
        {
            Log("Fatal", std::vformat(format, std::make_format_args(args...)));
        }

      private:
        std::string m_Name;
        Level m_Level;

        void Log(std::string_view level, std::string_view message);
    };
}
