#include "ui/panels/SettingsModal.hpp"

#include <cstring>

#include <imgui.h>

#include "core/Settings.hpp"

void SettingsModal::open()
{
    m_open = true;

    const std::string existing = Settings::Instance()->get<std::string>("youtube_api_key");
    std::strncpy(m_apiKeyBuf, existing.c_str(), sizeof(m_apiKeyBuf) - 1);
    m_apiKeyBuf[sizeof(m_apiKeyBuf) - 1] = '\0';

    const std::string existingLfm = Settings::Instance()->get<std::string>("lastfm_api_key");
    std::strncpy(m_lastfmKeyBuf, existingLfm.c_str(), sizeof(m_lastfmKeyBuf) - 1);
    m_lastfmKeyBuf[sizeof(m_lastfmKeyBuf) - 1] = '\0';
}

void SettingsModal::draw()
{
    if (m_open) {
        ImGui::OpenPopup("Settings");
        m_open = false;
    }

    ImGui::SetNextWindowSize(ImVec2(420, 230), ImGuiCond_Always);
    ImGui::SetNextWindowPos(
        ImGui::GetMainViewport()->GetCenter(),
        ImGuiCond_Always,
        ImVec2(0.5f, 0.5f)
    );

    constexpr ImGuiWindowFlags kFlags =
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove   |
        ImGuiWindowFlags_NoCollapse;

    if (!ImGui::BeginPopupModal("Settings", nullptr, kFlags))  { return; }

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
