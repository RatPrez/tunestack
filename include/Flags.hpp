#pragma once

#include <imgui.h>

namespace Flags
{

    inline constexpr ImGuiWindowFlags kStaticDiv =  ImGuiWindowFlags_NoResize |
                                                    ImGuiWindowFlags_NoMove |
                                                    ImGuiWindowFlags_NoCollapse |
                                                    ImGuiWindowFlags_NoTitleBar |
                                                    ImGuiWindowFlags_NoScrollbar |
                                                    ImGuiWindowFlags_NoScrollWithMouse;


}
