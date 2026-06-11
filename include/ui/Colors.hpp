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
    inline constexpr ImVec4 kWhiteDead { 1.f, 1.f, 1.f, .2f };

    inline constexpr ImVec4 kAccent     { 0.35f, 0.65f, 1.0f, 1.0f };
    inline constexpr ImVec4 kAccentDim  { 0.35f, 0.65f, 1.0f, 0.5f };

    inline constexpr ImVec4 kTrackIdle         { 1.f, 1.f, 1.f, 0.1f };
    inline constexpr ImVec4 kTrackProgress     { 0.35f, 0.65f, 1.0f, 0.5f };
    inline constexpr ImVec4 kTrackProgressHover{ 0.35f, 0.65f, 1.0f, 0.85f };
}
