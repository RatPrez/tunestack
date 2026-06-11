#pragma once

class BottomBar
{
public:
    BottomBar();
    ~BottomBar();

    void draw();

private:
    void drawTrackInfo();
    void drawTrackProgress();
    void drawControls();
    void drawVolume();

    bool drawButton(const char* id, const char* icon, const char* hint);

};
