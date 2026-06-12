#include "ui/panels/SettingsModal.hpp"

#include <cstring>
#include <format>

#include <imgui.h>

#include "core/MediaManager.hpp"
#include "core/Settings.hpp"

void SettingsModal::open()
{
    m_open = true;
    m_cacheCleared = false;

    const std::string existing = Settings::Instance()->get<std::string>("youtube_api_key");
    std::strncpy(m_apiKeyBuf, existing.c_str(), sizeof(m_apiKeyBuf) - 1);
    m_apiKeyBuf[sizeof(m_apiKeyBuf) - 1] = '\0';

    const std::string existingLfm = Settings::Instance()->get<std::string>("lastfm_api_key");
    std::strncpy(m_lastfmKeyBuf, existingLfm.c_str(), sizeof(m_lastfmKeyBuf) - 1);
    m_lastfmKeyBuf[sizeof(m_lastfmKeyBuf) - 1] = '\0';

    if (auto* mm = MediaManager::Instance()) { m_cacheBytes = mm->cacheSize(); }
}

void SettingsModal::draw()
{
    if (m_open) {
        ImGui::OpenPopup("Settings");
        m_open = false;
    }

    ImGui::SetNextWindowSize(ImVec2(420, 300), ImGuiCond_Always);
    ImGui::SetNextWindowPos(
        ImGui::GetMainViewport()->GetCenter(),
        ImGuiCond_Always,
        ImVec2(0.5f, 0.5f)
    );

    constexpr ImGuiWindowFlags kFlags =
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove   |
        ImGuiWindowFlags_NoCollapse;

    if (!ImGui::BeginPopupModal("Settings", nullptr, kFlags)) { return; }

    ImGui::Text("YouTube API Key");
    ImGui::Spacing();
    ImGui::SetNextItemWidth(-1);
    ImGui::InputText("##yt_api_key", m_apiKeyBuf, sizeof(m_apiKeyBuf));

    ImGui::Spacing();

    ImGui::Text("Last.fm API Key");
    ImGui::Spacing();
    ImGui::SetNextItemWidth(-1);
    ImGui::InputText("##lfm_api_key", m_lastfmKeyBuf, sizeof(m_lastfmKeyBuf));

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    ImGui::Text("Download Cache");
    ImGui::Spacing();

    const std::string sizeStr = m_cacheCleared ? "Cleared" : formatBytes(m_cacheBytes);
    ImGui::Text("Size: %s", sizeStr.c_str());
    ImGui::SameLine();

    const bool canClear = !m_cacheCleared && m_cacheBytes > 0;
    if (!canClear) { ImGui::BeginDisabled(); }
    if (ImGui::Button("Clear Cache")) {
        if (auto* mm = MediaManager::Instance()) {
            mm->clearCache();
            m_cacheBytes = 0;
            m_cacheCleared = true;
        }
    }
    if (!canClear) { ImGui::EndDisabled(); }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    const bool save = ImGui::Button("Save", ImVec2(80, 0));
    ImGui::SameLine();
    if (ImGui::Button("Cancel", ImVec2(80, 0))) { ImGui::CloseCurrentPopup(); }

    if (save) {
        Settings::Instance()->set("youtube_api_key", std::string(m_apiKeyBuf));
        Settings::Instance()->set("lastfm_api_key",  std::string(m_lastfmKeyBuf));
        ImGui::CloseCurrentPopup();
    }

    ImGui::EndPopup();
}

std::string SettingsModal::formatBytes(std::uintmax_t bytes)
{
    if (bytes == 0)                    { return "0 B"; }
    if (bytes < 1024)                  { return std::format("{} B", bytes); }
    if (bytes < 1024 * 1024)           { return std::format("{:.1f} KB", bytes / 1024.0); }
    if (bytes < 1024 * 1024 * 1024ULL){ return std::format("{:.1f} MB", bytes / (1024.0 * 1024)); }
    return std::format("{:.2f} GB", bytes / (1024.0 * 1024 * 1024));
}
