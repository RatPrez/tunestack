#pragma once

#include <string>

class SideBar
{
public:
    SideBar();
    ~SideBar();

    void draw();

private:
    bool itemButton(const int& index, const std::string& label);

};
