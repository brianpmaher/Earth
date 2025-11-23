#include "Logger.hpp"

#include <imgui.h>
#include <spdlog/sinks/base_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include <mutex>
#include <vector>

namespace Earth
{
    namespace
    {
        struct LogMessage
        {
            spdlog::level::level_enum Level;
            std::string Message;
        };

        std::vector<LogMessage> s_LogMessages;
        std::mutex s_LogMutex;
        bool s_ScrollToBottom = false;
        ImGuiTextFilter s_Filter;

        template <typename Mutex>
        class ImGuiSink : public spdlog::sinks::base_sink<Mutex>
        {
          protected:
            void sink_it_(const spdlog::details::log_msg& msg) override
            {
                spdlog::memory_buf_t formatted;
                spdlog::sinks::base_sink<Mutex>::formatter_->format(msg, formatted);

                std::lock_guard<std::mutex> lock(s_LogMutex);
                s_LogMessages.push_back({msg.level, fmt::to_string(formatted)});
                s_ScrollToBottom = true;
            }

            void flush_() override
            {
            }
        };

        std::shared_ptr<spdlog::sinks::stdout_color_sink_mt> s_ConsoleSink;
        std::shared_ptr<ImGuiSink<std::mutex>> s_ImGuiSink;
    }

    void Logger::Init()
    {
        s_ConsoleSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        s_ConsoleSink->set_pattern("%^[%T] %n: %v%$");

        s_ImGuiSink = std::make_shared<ImGuiSink<std::mutex>>();
        s_ImGuiSink->set_pattern("[%T] [%l] %n: %v");
    }

    void Logger::Draw(bool* p_open)
    {
        if (!ImGui::Begin("Log", p_open))
        {
            ImGui::End();
            return;
        }

        if (ImGui::Button("Clear"))
        {
            std::lock_guard<std::mutex> lock(s_LogMutex);
            s_LogMessages.clear();
        }
        ImGui::SameLine();
        s_Filter.Draw("Filter", -100.0f);

        ImGui::Separator();
        ImGui::BeginChild("scrolling", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);

        std::lock_guard<std::mutex> lock(s_LogMutex);
        for (const auto& msg : s_LogMessages)
        {
            if (!s_Filter.PassFilter(msg.Message.c_str()))
                continue;

            ImVec4 color;
            switch (msg.Level)
            {
            case spdlog::level::trace:
                color = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
                break;
            case spdlog::level::debug:
                color = ImVec4(0.0f, 0.5f, 1.0f, 1.0f);
                break;
            case spdlog::level::info:
                color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
                break;
            case spdlog::level::warn:
                color = ImVec4(1.0f, 1.0f, 0.0f, 1.0f);
                break;
            case spdlog::level::err:
                color = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
                break;
            case spdlog::level::critical:
                color = ImVec4(1.0f, 0.0f, 1.0f, 1.0f);
                break;
            default:
                color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
                break;
            }
            ImGui::PushStyleColor(ImGuiCol_Text, color);
            ImGui::TextUnformatted(msg.Message.c_str());
            ImGui::PopStyleColor();
        }

        if (s_ScrollToBottom)
        {
            ImGui::SetScrollHereY(1.0f);
            s_ScrollToBottom = false;
        }

        ImGui::EndChild();
        ImGui::End();
    }

    Logger::Logger(std::string_view name)
    {
        if (!s_ConsoleSink)
        {
            Init();
        }

        std::vector<spdlog::sink_ptr> sinks;
        sinks.push_back(s_ConsoleSink);
        sinks.push_back(s_ImGuiSink);

        m_Logger = std::make_shared<spdlog::logger>(std::string(name), sinks.begin(), sinks.end());
        m_Logger->set_level(spdlog::level::trace);
    }
}
