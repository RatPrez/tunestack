#pragma once

#include <optional>
#include <string>
#include <vector>

#define CPPHTTPLIB_OPENSSL_SUPPORT
#include <httplib.h>

struct LastFMImage
{
    std::string url;
    std::string size;  // small, medium, large, extralarge, mega
};

struct LastFMTag
{
    std::string name;
    std::string url;
    int count = 0;
};

struct LastFMArtist
{
    std::string name;
    std::string url;
    std::string mbid;
    std::string bio;
    std::string bioSummary;
    int listeners = 0;
    int playCount = 0;
    std::vector<LastFMImage> images;
    std::vector<LastFMTag> tags;
    std::vector<std::string> similarArtists;
};

struct LastFMAlbum
{
    std::string name;
    std::string artist;
    std::string url;
    std::string mbid;
    std::string releaseDate;
    int listeners = 0;
    int playCount = 0;
    std::vector<LastFMImage> images;
    std::vector<LastFMTag> tags;
};

struct LastFMTrack
{
    std::string name;
    std::string artist;
    std::string album;
    std::string url;
    std::string mbid;
    int duration = 0;   // seconds
    int listeners = 0;
    int playCount = 0;
    bool loved = false;
    std::vector<LastFMTag> tags;
    std::vector<LastFMImage> images;
};

struct LastFMScrobble
{
    std::string track;
    std::string artist;
    std::string album;
    std::time_t timestamp = 0;
};


class LastFM
{
public:
    explicit LastFM(const std::string& apiKey, const std::string& secret = {});

    // auth — call getAuthUrl, user logs in, then finaliseAuth with the token from the callback
    std::string getAuthUrl() const;
    bool finaliseAuth(const std::string& token);
    bool isAuthenticated() const { return !m_sessionKey.empty(); }
    void setSessionKey(const std::string& key) { m_sessionKey = key; }
    const std::string& getSessionKey() const { return m_sessionKey; }

    // track
    std::optional<LastFMTrack>              trackInfo(const std::string& artist, const std::string& track, const std::string& username = {}) const;
    std::vector<LastFMTrack>                trackSearch(const std::string& query, int limit = 25) const;
    std::vector<LastFMTrack>                trackSimilar(const std::string& artist, const std::string& track, int limit = 25) const;
    std::vector<LastFMTag>                  trackTopTags(const std::string& artist, const std::string& track) const;
    bool                                    trackLove(const std::string& artist, const std::string& track);
    bool                                    trackUnlove(const std::string& artist, const std::string& track);
    bool                                    trackScrobble(const LastFMScrobble& scrobble);
    bool                                    trackScrobbleBatch(const std::vector<LastFMScrobble>& scrobbles);
    bool                                    trackUpdateNowPlaying(const std::string& artist, const std::string& track, const std::string& album = {}, int duration = 0);

    // artist
    std::optional<LastFMArtist>             artistInfo(const std::string& artist, const std::string& username = {}) const;
    std::vector<LastFMArtist>               artistSearch(const std::string& query, int limit = 25) const;
    std::vector<LastFMArtist>               artistSimilar(const std::string& artist, int limit = 25) const;
    std::vector<LastFMTag>                  artistTopTags(const std::string& artist) const;
    std::vector<LastFMTrack>                artistTopTracks(const std::string& artist, int limit = 25) const;
    std::vector<LastFMAlbum>                artistTopAlbums(const std::string& artist, int limit = 25) const;

    // album
    std::optional<LastFMAlbum>              albumInfo(const std::string& artist, const std::string& album, const std::string& username = {}) const;
    std::vector<LastFMTag>                  albumTopTags(const std::string& artist, const std::string& album) const;

    // charts
    std::vector<LastFMTrack>                chartTopTracks(int limit = 25) const;
    std::vector<LastFMArtist>               chartTopArtists(int limit = 25) const;
    std::vector<LastFMTag>                  chartTopTags(int limit = 25) const;

    // tag
    std::vector<LastFMTrack>                tagTopTracks(const std::string& tag, int limit = 25) const;
    std::vector<LastFMArtist>               tagTopArtists(const std::string& tag, int limit = 25) const;
    std::vector<LastFMAlbum>                tagTopAlbums(const std::string& tag, int limit = 25) const;

private:
    std::string get(const std::string& params) const;
    std::string post(const std::string& params);
    std::string sign(const std::string& params) const;
    std::string urlEncode(const std::string& s) const;

    static std::vector<LastFMImage>  parseImages(const std::string& json);
    static std::vector<LastFMTag>    parseTags(const std::string& json, const std::string& key = "tag");
    static LastFMTrack               parseTrack(const std::string& json);
    static LastFMArtist              parseArtist(const std::string& json);
    static LastFMAlbum               parseAlbum(const std::string& json);
    static std::string               extractField(const std::string& json, const std::string& key);
    static int                       extractInt(const std::string& json, const std::string& key);
    static std::vector<std::string>  extractArray(const std::string& json, const std::string& key);

    std::string m_apiKey;
    std::string m_secret;
    std::string m_sessionKey;
    mutable httplib::Client m_client;
};
