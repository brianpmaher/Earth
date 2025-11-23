#pragma once

#include <spdlog/fmt/ostr.h>
#include <spdlog/spdlog.h>

#include <memory>
#include <string_view>

namespace Earth
{
    class Logger
    {
      public:
        static void Init();
        static void Draw(bool* p_open = nullptr);

        Logger(std::string_view name);

        template <typename... Args>
        void Debug(spdlog::format_string_t<Args...> fmt, Args&&... args)
        {
            m_Logger->debug(fmt, std::forward<Args>(args)...);
        }

        template <typename... Args>
        void Info(spdlog::format_string_t<Args...> fmt, Args&&... args)
        {
            m_Logger->info(fmt, std::forward<Args>(args)...);
        }

        template <typename... Args>
        void Warn(spdlog::format_string_t<Args...> fmt, Args&&... args)
        {
            m_Logger->warn(fmt, std::forward<Args>(args)...);
        }

        template <typename... Args>
        void Error(spdlog::format_string_t<Args...> fmt, Args&&... args)
        {
            m_Logger->error(fmt, std::forward<Args>(args)...);
        }

        template <typename... Args>
        void Fatal(spdlog::format_string_t<Args...> fmt, Args&&... args)
        {
            m_Logger->critical(fmt, std::forward<Args>(args)...);
        }

      private:
        std::shared_ptr<spdlog::logger> m_Logger;
    };
}
