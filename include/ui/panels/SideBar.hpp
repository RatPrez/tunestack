#pragma once

#include <string>

class SideBar
{
public:
    SideBar();
    ~SideBar();

    void draw();

private:
    bool iconButton(int index, const char* icon, const char* tooltip);

};
