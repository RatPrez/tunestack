#pragma once

#include <atomic>
#include <mutex>
#include <optional>
#include <queue>
#include <thread>
#include <vector>

#include "core/AppStatus.hpp"
#include "core/EventBus.hpp"
#include "core/YtDlp.hpp"
#include "core/PlaylistManager.hpp"
#include "core/PlayQueue.hpp"
#include "core/Player.hpp"
#include "core/MediaManager.hpp"
#include "core/Radio.hpp"
#include "core/Settings.hpp"
#include "core/Track.hpp"
#include "api/LastFM.hpp"

// panels
#include "ui/panels/TopBar.hpp"
#include "ui/panels/BottomBar.hpp"
#include "ui/panels/SideBar.hpp"
#include "ui/panels/MainView.hpp"
#include "ui/panels/SettingsModal.hpp"

class SDL_Renderer;

class App
{
public:
    explicit App(SDL_Renderer* renderer);
    ~App();
    void tick();
    void draw();

private:
    AppStatus       m_appStatus;
    EventBus        m_eventBus;
    Settings        m_settings;
    YtDlp           m_ytDlp;
    PlaylistManager m_playlists;
    Player       m_player;
    MediaManager m_mediaManager;
    LastFM       m_lastfm;
    PlayQueue    m_playQueue;
    Radio        m_radio;

    TopBar        m_topBar;
    BottomBar     m_bottomBar;
    SideBar       m_sideBar;
    MainView      m_mainView;
    SettingsModal m_settingsModal;

    void search(const std::string& query);
    void flushSearch();
    void startEnrichment();

    // search results
    std::mutex m_searchMutex;
    std::optional<std::vector<TrackResult>> m_pendingResults;

    // enrichment queue — background thread fills in album/art/duration via track.getInfo
    struct EnrichUpdate { int index; std::string album; std::string albumArtUrl; int duration; };
    std::mutex             m_enrichMutex;
    std::vector<TrackResult> m_currentResults; // working copy enriched in-place
    std::queue<int>          m_enrichPending;  // indices still to fetch
    std::queue<EnrichUpdate> m_enrichDone;     // completed, awaiting main-thread apply
    std::atomic<bool>        m_enrichRunning { false };
};
