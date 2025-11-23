#pragma once

#include "URL.hpp"

#include <string>

namespace Earth::HTTP
{
    std::string Fetch(const URL& url);
}
