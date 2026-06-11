#pragma once

#include <mutex>
#include <string>
#include <unordered_map>
#include <unordered_set>

struct SDL_Texture;

struct PendingTexture
{
    unsigned char* pixels = nullptr;
    int width = 0;
    int height = 0;
};

class TextureCache
{
public:
    ~TextureCache();

    void addImageBytes(const std::string& key, const char* data, const int size);

    SDL_Texture* get(const std::string& path);
    SDL_Texture* getUrl(const std::string& url);

    static TextureCache* Instance() {
        static TextureCache m_instance;
        return &m_instance;
    }

private:
    std::unordered_map<std::string, SDL_Texture*> m_loaded;
    std::unordered_set<std::string> m_requested;
    std::mutex m_pendingMutex;
    std::unordered_map<std::string, PendingTexture> m_pending;

    void requestUrlLoad(const std::string& url);
    void requestPathLoad(const std::string& path);

    SDL_Texture* uploadPendingToGPU(const std::string& key, PendingTexture& pending);

};
