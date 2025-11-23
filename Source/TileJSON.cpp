#include "TileJSON.hpp"
#include "HTTP.hpp"

namespace Earth
{
    TileJSON::TileJSON(const URL& url)
    {
        std::string jsonString = HTTP::Fetch(url);
        m_Json = nlohmann::json::parse(jsonString);
    }
}
