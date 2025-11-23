#pragma once

#include <format>
#include <string>

namespace Earth
{
    class URL
    {
      public:
        URL(std::string url) : m_URL(std::move(url))
        {
        }
        URL(const char* url) : m_URL(url)
        {
        }

        const std::string& Get() const
        {
            return m_URL;
        }

      private:
        std::string m_URL;
    };
}

template <>
struct std::formatter<Earth::URL> : std::formatter<std::string>
{
    auto format(const Earth::URL& url, std::format_context& ctx) const
    {
        return formatter<std::string>::format(url.Get(), ctx);
    }
};
