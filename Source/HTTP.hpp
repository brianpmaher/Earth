#pragma once

#include "URL.hpp"

#include <atomic>
#include <string>

namespace Earth::HTTP
{
    std::string Fetch(const URL& url, std::atomic<bool>* cancelled = nullptr);
}
