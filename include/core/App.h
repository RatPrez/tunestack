#pragma once

class SDL_Renderer;

class App
{
public:
    explicit App(SDL_Renderer* renderer);
    void draw();

};
