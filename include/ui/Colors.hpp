#pragma once

#include <imgui.h>

namespace Colors
{
    inline constexpr ImVec4 kTransparent { 1.f, 1.f, 1.f, .0f };
    inline constexpr ImVec4 kPanelBg { 1.f, 1.f, 1.f, .03f };
    inline constexpr ImVec4 kPanelBorder { 1.f, 1.f, 1.f, 0.3f };

    inline constexpr ImVec4 kTermainlGreen { 0.18f, 0.98f, 0.18f, 1.0f };

    inline constexpr ImVec4 kWhite { 1.f, 1.f, 1.f, 1.f };
    inline constexpr ImVec4 kWhiteHalf { 1.f, 1.f, 1.f, .5f };


    inline constexpr ImVec4 kTrackIdle { 1.f, 1.f, 1.f, 0.1f };
    inline constexpr ImVec4 kTrackProgress { 0.f, 0.2f, 0.8f, 0.3f };
    inline constexpr ImVec4 kTrackProgressHover { 0.f, 0.2f, 0.8f, 0.6f };
}
