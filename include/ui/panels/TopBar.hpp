#pragma once

struct ImVec2;

class TopBar
{
public:
    TopBar();
    ~TopBar();

    void draw();

private:
    void drawLeft(const ImVec2& windowPos, const ImVec2& windowSize);
    void drawSearch(const ImVec2& windowPos, const ImVec2& windowSize);
    void drawRight(const ImVec2& windowPos, const ImVec2& windowSize);

};
