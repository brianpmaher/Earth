#pragma once

#include <nlohmann/json.hpp>

#include "URL.hpp"

#include <string>

namespace Earth
{
    class TileJSON
    {
      public:
        TileJSON(const URL& url);

        const nlohmann::json& GetJson() const
        {
            return m_Json;
        }

      private:
        nlohmann::json m_Json;
    };
}
