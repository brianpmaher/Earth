#include "HTTP.hpp"

#include <curl/curl.h>

#include <format>
#include <print>
#include <stdexcept>

namespace
{
    size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp)
    {
        ((std::string*)userp)->append((char*)contents, size * nmemb);
        return size * nmemb;
    }
}

namespace Earth::HTTP
{
    std::string Fetch(const URL& url, std::atomic<bool>* cancelled)
    {
        if (cancelled && *cancelled)
        {
            throw std::runtime_error("Request cancelled");
        }

        CURLM* multi_handle = curl_multi_init();
        CURL* curl = curl_easy_init();
        std::string readBuffer;

        if (curl && multi_handle)
        {
            curl_easy_setopt(curl, CURLOPT_URL, url.Get().c_str());
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
            curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
            // Set User-Agent to avoid some servers blocking requests
            curl_easy_setopt(curl, CURLOPT_USERAGENT, "Earth/0.1");
            // Handle compressed responses
            curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, "");

            curl_multi_add_handle(multi_handle, curl);

            int still_running = 0;
            do
            {
                if (cancelled && *cancelled)
                {
                    curl_multi_remove_handle(multi_handle, curl);
                    curl_easy_cleanup(curl);
                    curl_multi_cleanup(multi_handle);
                    throw std::runtime_error("Request cancelled");
                }

                CURLMcode mc = curl_multi_perform(multi_handle, &still_running);
                if (mc)
                {
                    break;
                }

                if (still_running)
                {
                    curl_multi_poll(multi_handle, NULL, 0, 100, NULL);
                }
            } while (still_running);

            CURLMsg* msg = nullptr;
            int msgs_left = 0;
            while ((msg = curl_multi_info_read(multi_handle, &msgs_left)))
            {
                if (msg->msg == CURLMSG_DONE)
                {
                    if (msg->data.result != CURLE_OK)
                    {
                        std::string errorMsg =
                            std::format("curl_multi_perform() failed: {}", curl_easy_strerror(msg->data.result));
                        std::println(stderr, "{}", errorMsg);
                        curl_multi_remove_handle(multi_handle, curl);
                        curl_easy_cleanup(curl);
                        curl_multi_cleanup(multi_handle);
                        throw std::runtime_error(errorMsg);
                    }
                    else
                    {
                        long response_code;
                        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
                        if (response_code != 200)
                        {
                            std::string errorMsg =
                                std::format("HTTP request failed with status code: {}", response_code);
                            std::println(stderr, "{}", errorMsg);
                            curl_multi_remove_handle(multi_handle, curl);
                            curl_easy_cleanup(curl);
                            curl_multi_cleanup(multi_handle);
                            throw std::runtime_error(errorMsg);
                        }
                    }
                }
            }

            curl_multi_remove_handle(multi_handle, curl);
            curl_easy_cleanup(curl);
            curl_multi_cleanup(multi_handle);
        }
        else
        {
            if (curl)
                curl_easy_cleanup(curl);
            if (multi_handle)
                curl_multi_cleanup(multi_handle);
            throw std::runtime_error("Failed to initialize CURL");
        }

        return readBuffer;
    }
}
