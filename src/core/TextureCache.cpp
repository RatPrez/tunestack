#include "core/TextureCache.hpp"

#include <SDL3/SDL.h>
#include <thread>

#define CPPHTTPLIB_OPENSSL_SUPPORT
#include <httplib.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "core/Window.hpp"

TextureCache::~TextureCache()
{
    for (auto& [p, texture] : m_loaded) {
        if (texture) {
            SDL_DestroyTexture(texture);
        }
    }
    std::lock_guard lock(m_pendingMutex);
    for (auto& [url, pending] : m_pending) {
        if (pending.pixels) { stbi_image_free(pending.pixels); }
    }
}

SDL_Texture* TextureCache::get(const std::string& path)
{
    auto it = m_loaded.find(path);
    if (it != m_loaded.end()) { return it->second; }

    {
        std::lock_guard lock(m_pendingMutex);
        auto pit = m_pending.find(path);
        if (pit != m_pending.end()) {
            SDL_Texture* texture = uploadPendingToGPU(path, pit->second);
            m_pending.erase(pit);
            return texture;
        }
    }

    if (!m_requested.count(path)) { requestPathLoad(path); }
    return nullptr;
}

SDL_Texture* TextureCache::getUrl(const std::string& url)
{
    auto it = m_loaded.find(url);
    if (it != m_loaded.end()) { return it->second; }

    {
        std::lock_guard lock(m_pendingMutex);
        auto pit = m_pending.find(url);
        if (pit != m_pending.end()) {
            SDL_Texture* texture = uploadPendingToGPU(url, pit->second);
            m_pending.erase(pit);
            return texture;
        }
    }

    if (!m_requested.count(url)) { requestUrlLoad(url); }
    return nullptr;
}

void TextureCache::addImageBytes(const std::string& key, const char* data, const int size)
{
    {
        std::lock_guard lock(m_pendingMutex);
        if (m_requested.count(key)) {
            return;
        }
        m_requested.insert(key);
    }
    int w, h, ch;
    unsigned char* pixels = stbi_load_from_memory(
        reinterpret_cast<const unsigned char*>(data), size, &w, &h, &ch, 4
    );
    if (!pixels) {
        return;
    }
    std::lock_guard lock(m_pendingMutex);
    m_pending[key] = { pixels, w, h };
}

void TextureCache::requestUrlLoad(const std::string& url)
{
    m_requested.insert(url);
    std::thread([this, url]() {
        const size_t schemeEnd = url.find("://");
        if (schemeEnd == std::string::npos) { return; }
        const std::string hostAndPath = url.substr(schemeEnd + 3);
        const size_t pathStart = hostAndPath.find('/');
        if (pathStart == std::string::npos) { return; }

        httplib::Client client("https://" + hostAndPath.substr(0, pathStart));
        client.set_follow_location(true);
        auto res = client.Get(hostAndPath.substr(pathStart));
        if (!res || res->status != 200) { return; }

        int w, h, ch;
        unsigned char* pixels = stbi_load_from_memory(
            reinterpret_cast<const unsigned char*>(res->body.data()),
            static_cast<int>(res->body.size()), &w, &h, &ch, 4
        );
        if (!pixels) { return; }

        std::lock_guard lock(m_pendingMutex);
        m_pending[url] = { pixels, w, h };
    }).detach();
}

void TextureCache::requestPathLoad(const std::string& path)
{
    m_requested.insert(path);
    std::thread([this, path]() {
        int w, h, ch;
        unsigned char* pixels = stbi_load(path.c_str(), &w, &h, &ch, 4);
        if (!pixels) { return; }

        std::lock_guard lock(m_pendingMutex);
        m_pending[path] = { pixels, w, h };
    }).detach();
}

SDL_Texture* TextureCache::uploadPendingToGPU(const std::string& key, PendingTexture& pending)
{
    auto* renderer = Window::Instance()->getRenderer();

    SDL_Texture* texture = SDL_CreateTexture(
        renderer, SDL_PIXELFORMAT_RGBA32,
        SDL_TEXTUREACCESS_STATIC, pending.width, pending.height
    );
    if (texture) {
        SDL_UpdateTexture(texture, nullptr, pending.pixels, pending.width * 4);
        m_loaded[key] = texture;
    }
    stbi_image_free(pending.pixels);
    pending.pixels = nullptr;
    return texture;
}
