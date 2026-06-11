#pragma once
#include <string>

struct ImVec2;

enum class Anchor : int
{
    TopLeft,
    TopRight
};

class TopBar
{
public:
    TopBar();
    ~TopBar();

    void draw();

private:
    char m_searchBuf[256] = {};

    void drawLeft(const ImVec2& windowPos, const ImVec2& windowSize);
    void drawSearch(const ImVec2& windowPos, const ImVec2& windowSize);
    void drawRight(const ImVec2& windowPos, const ImVec2& windowSize);
    bool drawButton(const int& index, const Anchor& anchor, const std::string& label, const std::string& hint = "");

};
