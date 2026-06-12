#include "api/LastFM.hpp"

#include <algorithm>
#include <ctime>
#include <iomanip>
#include <sstream>

#include <json.hpp>

using json = nlohmann::json;

static constexpr const char* kHost = "https://ws.audioscrobbler.com";
static constexpr const char* kBase = "/2.0/";

// ---------------------------------------------------------------------------
// helpers

static std::string jsonStr(const json& j, const char* key)
{
    if (!j.contains(key) || j[key].is_null()) { return {}; }
    const auto& v = j[key];
    if (v.is_string()) { return v.get<std::string>(); }
    return v.dump();
}

static int jsonInt(const json& j, const char* key)
{
    if (!j.contains(key) || j[key].is_null()) { return 0; }
    const auto& v = j[key];
    if (v.is_number()) { return v.get<int>(); }
    try { return std::stoi(v.get<std::string>()); } catch (...) { return 0; }
}

static std::vector<LastFMImage> imagesFromJson(const json& j)
{
    std::vector<LastFMImage> result;
    if (!j.contains("image") || !j["image"].is_array()) { return result; }
    for (const auto& img : j["image"]) {
        LastFMImage i;
        i.url  = jsonStr(img, "#text");
        i.size = jsonStr(img, "size");
        result.push_back(std::move(i));
    }
    return result;
}

static std::vector<LastFMTag> tagsFromJson(const json& j, const char* arrayKey = "tag")
{
    std::vector<LastFMTag> result;
    if (!j.contains(arrayKey)) { return result; }
    const auto& arr = j[arrayKey];
    auto parse = [&](const json& t) {
        LastFMTag tag;
        tag.name  = jsonStr(t, "name");
        tag.url   = jsonStr(t, "url");
        tag.count = jsonInt(t, "count");
        result.push_back(std::move(tag));
    };
    if (arr.is_array()) { for (const auto& t : arr) { parse(t); } }
    else if (arr.is_object()) { parse(arr); }
    return result;
}

static LastFMTrack trackFromJson(const json& j)
{
    LastFMTrack t;
    t.name      = jsonStr(j, "name");
    t.url       = jsonStr(j, "url");
    t.mbid      = jsonStr(j, "mbid");
    t.duration  = jsonInt(j, "duration");
    t.listeners = jsonInt(j, "listeners");
    t.playCount = jsonInt(j, "playcount");
    t.loved     = (jsonStr(j, "userloved") == "1");
    if (j.contains("artist")) {
        const auto& a = j["artist"];
        t.artist = a.is_string() ? a.get<std::string>() : jsonStr(a, "name");
    }
    if (j.contains("album")) {
        const auto& a = j["album"];
        t.album = a.is_string() ? a.get<std::string>() : jsonStr(a, "title");
    }
    t.images = imagesFromJson(j);
    // track.getInfo nests album art under "album" — fall back to it when track has no images
    if (t.images.empty() && j.contains("album") && j["album"].is_object()) {
        t.images = imagesFromJson(j["album"]);
    }
    if (j.contains("toptags") && j["toptags"].is_object())
        t.tags = tagsFromJson(j["toptags"]);
    return t;
}

static LastFMArtist artistFromJson(const json& j)
{
    LastFMArtist a;
    a.name      = jsonStr(j, "name");
    a.url       = jsonStr(j, "url");
    a.mbid      = jsonStr(j, "mbid");
    a.listeners = jsonInt(j, "listeners");
    a.playCount = jsonInt(j, "playcount");
    if (j.contains("bio") && j["bio"].is_object()) {
        a.bioSummary = jsonStr(j["bio"], "summary");
        a.bio        = jsonStr(j["bio"], "content");
    }
    a.images = imagesFromJson(j);
    if (j.contains("tags") && j["tags"].is_object())
        a.tags = tagsFromJson(j["tags"]);
    if (j.contains("similar") && j["similar"].is_object() && j["similar"].contains("artist")) {
        const auto& sim = j["similar"]["artist"];
        if (sim.is_array()) {
            for (const auto& s : sim) { a.similarArtists.push_back(jsonStr(s, "name")); }
        }
    }
    return a;
}

static LastFMAlbum albumFromJson(const json& j)
{
    LastFMAlbum a;
    a.name        = jsonStr(j, "name");
    a.artist      = jsonStr(j, "artist");
    a.url         = jsonStr(j, "url");
    a.mbid        = jsonStr(j, "mbid");
    a.releaseDate = jsonStr(j, "releasedate");
    a.listeners   = jsonInt(j, "listeners");
    a.playCount   = jsonInt(j, "playcount");
    a.images = imagesFromJson(j);
    if (j.contains("tags") && j["tags"].is_object())
        a.tags = tagsFromJson(j["tags"]);
    return a;
}

// ---------------------------------------------------------------------------

LastFM::LastFM(const std::string& apiKey, const std::string& secret)
    : m_apiKey(apiKey), m_secret(secret), m_client(kHost)
{
    m_client.set_follow_location(true);
}

// ---------------------------------------------------------------------------
// network

std::string LastFM::get(const std::string& params) const
{
    const std::string path = std::string(kBase) + "?format=json&api_key=" + m_apiKey + "&" + params;
    auto res = m_client.Get(path);
    if (!res || res->status != 200) { return {}; }
    return res->body;
}

std::string LastFM::post(const std::string& body)
{
    httplib::Headers headers = { {"Content-Type", "application/x-www-form-urlencoded"} };
    auto res = m_client.Post(kBase, headers, body, "application/x-www-form-urlencoded");
    if (!res || res->status != 200) { return {}; }
    return res->body;
}

std::string LastFM::urlEncode(const std::string& s) const
{
    std::ostringstream out;
    for (unsigned char c : s) {
        if (std::isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
            out << c;
        } else {
            out << '%' << std::uppercase << std::hex << std::setw(2) << std::setfill('0') << (int)c;
        }
    }
    return out.str();
}

// minimal api_sig: sorted key=value concat + secret, md5 hex
// requires openssl — if not available this remains a stub (scrobbling won't work without it)
std::string LastFM::sign(const std::string& /*params*/) const
{
    // stub — implement with OpenSSL MD5 if scrobbling is needed
    return {};
}

// ---------------------------------------------------------------------------
// auth

std::string LastFM::getAuthUrl() const
{
    return "https://www.last.fm/api/auth/?api_key=" + m_apiKey;
}

bool LastFM::finaliseAuth(const std::string& token)
{
    const std::string body = "method=auth.getSession&token=" + urlEncode(token)
        + "&api_key=" + m_apiKey + "&format=json";
    const auto resp = post(body);
    if (resp.empty()) { return false; }
    try {
        auto j = json::parse(resp);
        if (j.contains("session")) {
            m_sessionKey = jsonStr(j["session"], "key");
            return !m_sessionKey.empty();
        }
    } catch (...) {}
    return false;
}

// ---------------------------------------------------------------------------
// track

std::optional<LastFMTrack> LastFM::trackInfo(const std::string& artist, const std::string& track, const std::string& username) const
{
    std::string q = "method=track.getInfo&artist=" + urlEncode(artist) + "&track=" + urlEncode(track);
    if (!username.empty()) q += "&username=" + urlEncode(username);
    const auto resp = get(q);
    if (resp.empty()) { return std::nullopt; }
    try {
        auto j = json::parse(resp);
        if (j.contains("track")) { return trackFromJson(j["track"]); }
    } catch (...) {}
    return std::nullopt;
}

std::vector<LastFMTrack> LastFM::trackSearch(const std::string& query, int limit) const
{
    const auto resp = get("method=track.search&track=" + urlEncode(query) + "&limit=" + std::to_string(limit));
    std::vector<LastFMTrack> result;
    if (resp.empty()) { return result; }
    try {
        auto j = json::parse(resp);
        const auto& tracks = j["results"]["trackmatches"]["track"];
        if (tracks.is_array()) { for (const auto& t : tracks) result.push_back(trackFromJson(t)); }
        else if (tracks.is_object()) { result.push_back(trackFromJson(tracks)); }
    } catch (...) {}
    return result;
}

std::vector<LastFMTrack> LastFM::trackSimilar(const std::string& artist, const std::string& track, int limit) const
{
    const auto resp = get("method=track.getSimilar&artist=" + urlEncode(artist)
        + "&track=" + urlEncode(track) + "&limit=" + std::to_string(limit));
    std::vector<LastFMTrack> result;
    if (resp.empty()) { return result; }
    try {
        auto j = json::parse(resp);
        const auto& arr = j["similartracks"]["track"];
        if (arr.is_array()) { for (const auto& t : arr) result.push_back(trackFromJson(t)); }
    } catch (...) {}
    return result;
}

std::vector<LastFMTag> LastFM::trackTopTags(const std::string& artist, const std::string& track) const
{
    const auto resp = get("method=track.getTopTags&artist=" + urlEncode(artist) + "&track=" + urlEncode(track));
    if (resp.empty()) { return {}; }
    try {
        auto j = json::parse(resp);
        if (j.contains("toptags")) { return tagsFromJson(j["toptags"]); }
    } catch (...) {}
    return {};
}

bool LastFM::trackLove(const std::string& artist, const std::string& track)
{
    if (m_sessionKey.empty()) { return false; }
    const std::string body = "method=track.love&artist=" + urlEncode(artist)
        + "&track=" + urlEncode(track)
        + "&api_key=" + m_apiKey
        + "&sk=" + m_sessionKey
        + "&format=json";
    return !post(body).empty();
}

bool LastFM::trackUnlove(const std::string& artist, const std::string& track)
{
    if (m_sessionKey.empty()) { return false; }
    const std::string body = "method=track.unlove&artist=" + urlEncode(artist)
        + "&track=" + urlEncode(track)
        + "&api_key=" + m_apiKey
        + "&sk=" + m_sessionKey
        + "&format=json";
    return !post(body).empty();
}

bool LastFM::trackScrobble(const LastFMScrobble& s)
{
    return trackScrobbleBatch({ s });
}

bool LastFM::trackScrobbleBatch(const std::vector<LastFMScrobble>& scrobbles)
{
    if (m_sessionKey.empty() || scrobbles.empty()) { return false; }
    std::string body = "method=track.scrobble&api_key=" + m_apiKey + "&sk=" + m_sessionKey + "&format=json";
    for (int i = 0; i < (int)scrobbles.size(); ++i) {
        const auto& s = scrobbles[i];
        const std::string idx = "[" + std::to_string(i) + "]";
        body += "&artist" + idx + "=" + urlEncode(s.artist);
        body += "&track"  + idx + "=" + urlEncode(s.track);
        body += "&timestamp" + idx + "=" + std::to_string(s.timestamp);
        if (!s.album.empty()) body += "&album" + idx + "=" + urlEncode(s.album);
    }
    return !post(body).empty();
}

bool LastFM::trackUpdateNowPlaying(const std::string& artist, const std::string& track, const std::string& album, int duration)
{
    if (m_sessionKey.empty()) { return false; }
    std::string body = "method=track.updateNowPlaying&api_key=" + m_apiKey
        + "&sk=" + m_sessionKey
        + "&artist=" + urlEncode(artist)
        + "&track=" + urlEncode(track)
        + "&format=json";
    if (!album.empty())  body += "&album=" + urlEncode(album);
    if (duration > 0)    body += "&duration=" + std::to_string(duration);
    return !post(body).empty();
}

// ---------------------------------------------------------------------------
// artist

std::optional<LastFMArtist> LastFM::artistInfo(const std::string& artist, const std::string& username) const
{
    std::string q = "method=artist.getInfo&artist=" + urlEncode(artist);
    if (!username.empty()) q += "&username=" + urlEncode(username);
    const auto resp = get(q);
    if (resp.empty()) { return std::nullopt; }
    try {
        auto j = json::parse(resp);
        if (j.contains("artist")) { return artistFromJson(j["artist"]); }
    } catch (...) {}
    return std::nullopt;
}

std::vector<LastFMArtist> LastFM::artistSearch(const std::string& query, int limit) const
{
    const auto resp = get("method=artist.search&artist=" + urlEncode(query) + "&limit=" + std::to_string(limit));
    std::vector<LastFMArtist> result;
    if (resp.empty()) { return result; }
    try {
        auto j = json::parse(resp);
        const auto& arr = j["results"]["artistmatches"]["artist"];
        if (arr.is_array()) { for (const auto& a : arr) result.push_back(artistFromJson(a)); }
        else if (arr.is_object()) { result.push_back(artistFromJson(arr)); }
    } catch (...) {}
    return result;
}

std::vector<LastFMArtist> LastFM::artistSimilar(const std::string& artist, int limit) const
{
    const auto resp = get("method=artist.getSimilar&artist=" + urlEncode(artist) + "&limit=" + std::to_string(limit));
    std::vector<LastFMArtist> result;
    if (resp.empty()) { return result; }
    try {
        auto j = json::parse(resp);
        const auto& arr = j["similarartists"]["artist"];
        if (arr.is_array()) { for (const auto& a : arr) result.push_back(artistFromJson(a)); }
    } catch (...) {}
    return result;
}

std::vector<LastFMTag> LastFM::artistTopTags(const std::string& artist) const
{
    const auto resp = get("method=artist.getTopTags&artist=" + urlEncode(artist));
    if (resp.empty()) { return {}; }
    try {
        auto j = json::parse(resp);
        if (j.contains("toptags")) { return tagsFromJson(j["toptags"]); }
    } catch (...) {}
    return {};
}

std::vector<LastFMTrack> LastFM::artistTopTracks(const std::string& artist, int limit) const
{
    const auto resp = get("method=artist.getTopTracks&artist=" + urlEncode(artist) + "&limit=" + std::to_string(limit));
    std::vector<LastFMTrack> result;
    if (resp.empty()) { return result; }
    try {
        auto j = json::parse(resp);
        const auto& arr = j["toptracks"]["track"];
        if (arr.is_array()) { for (const auto& t : arr) result.push_back(trackFromJson(t)); }
    } catch (...) {}
    return result;
}

std::vector<LastFMAlbum> LastFM::artistTopAlbums(const std::string& artist, int limit) const
{
    const auto resp = get("method=artist.getTopAlbums&artist=" + urlEncode(artist) + "&limit=" + std::to_string(limit));
    std::vector<LastFMAlbum> result;
    if (resp.empty()) { return result; }
    try {
        auto j = json::parse(resp);
        const auto& arr = j["topalbums"]["album"];
        if (arr.is_array()) { for (const auto& a : arr) result.push_back(albumFromJson(a)); }
    } catch (...) {}
    return result;
}

// ---------------------------------------------------------------------------
// album

std::optional<LastFMAlbum> LastFM::albumInfo(const std::string& artist, const std::string& album, const std::string& username) const
{
    std::string q = "method=album.getInfo&artist=" + urlEncode(artist) + "&album=" + urlEncode(album);
    if (!username.empty()) q += "&username=" + urlEncode(username);
    const auto resp = get(q);
    if (resp.empty()) { return std::nullopt; }
    try {
        auto j = json::parse(resp);
        if (j.contains("album")) { return albumFromJson(j["album"]); }
    } catch (...) {}
    return std::nullopt;
}

std::vector<LastFMTag> LastFM::albumTopTags(const std::string& artist, const std::string& album) const
{
    const auto resp = get("method=album.getTopTags&artist=" + urlEncode(artist) + "&album=" + urlEncode(album));
    if (resp.empty()) { return {}; }
    try {
        auto j = json::parse(resp);
        if (j.contains("toptags")) { return tagsFromJson(j["toptags"]); }
    } catch (...) {}
    return {};
}

// ---------------------------------------------------------------------------
// charts

std::vector<LastFMTrack> LastFM::chartTopTracks(int limit) const
{
    const auto resp = get("method=chart.getTopTracks&limit=" + std::to_string(limit));
    std::vector<LastFMTrack> result;
    if (resp.empty()) { return result; }
    try {
        auto j = json::parse(resp);
        const auto& arr = j["tracks"]["track"];
        if (arr.is_array()) { for (const auto& t : arr) result.push_back(trackFromJson(t)); }
    } catch (...) {}
    return result;
}

std::vector<LastFMArtist> LastFM::chartTopArtists(int limit) const
{
    const auto resp = get("method=chart.getTopArtists&limit=" + std::to_string(limit));
    std::vector<LastFMArtist> result;
    if (resp.empty()) { return result; }
    try {
        auto j = json::parse(resp);
        const auto& arr = j["artists"]["artist"];
        if (arr.is_array()) { for (const auto& a : arr) result.push_back(artistFromJson(a)); }
    } catch (...) {}
    return result;
}

std::vector<LastFMTag> LastFM::chartTopTags(int limit) const
{
    const auto resp = get("method=chart.getTopTags&limit=" + std::to_string(limit));
    if (resp.empty()) { return {}; }
    try {
        auto j = json::parse(resp);
        if (j.contains("tags")) { return tagsFromJson(j["tags"]); }
    } catch (...) {}
    return {};
}

// ---------------------------------------------------------------------------
// tag

std::vector<LastFMTrack> LastFM::tagTopTracks(const std::string& tag, int limit) const
{
    const auto resp = get("method=tag.getTopTracks&tag=" + urlEncode(tag) + "&limit=" + std::to_string(limit));
    std::vector<LastFMTrack> result;
    if (resp.empty()) { return result; }
    try {
        auto j = json::parse(resp);
        const auto& arr = j["tracks"]["track"];
        if (arr.is_array()) { for (const auto& t : arr) result.push_back(trackFromJson(t)); }
    } catch (...) {}
    return result;
}

std::vector<LastFMArtist> LastFM::tagTopArtists(const std::string& tag, int limit) const
{
    const auto resp = get("method=tag.getTopArtists&tag=" + urlEncode(tag) + "&limit=" + std::to_string(limit));
    std::vector<LastFMArtist> result;
    if (resp.empty()) { return result; }
    try {
        auto j = json::parse(resp);
        const auto& arr = j["topartists"]["artist"];
        if (arr.is_array()) { for (const auto& a : arr) result.push_back(artistFromJson(a)); }
    } catch (...) {}
    return result;
}

std::vector<LastFMAlbum> LastFM::tagTopAlbums(const std::string& tag, int limit) const
{
    const auto resp = get("method=tag.getTopAlbums&tag=" + urlEncode(tag) + "&limit=" + std::to_string(limit));
    std::vector<LastFMAlbum> result;
    if (resp.empty()) { return result; }
    try {
        auto j = json::parse(resp);
        const auto& arr = j["albums"]["album"];
        if (arr.is_array()) { for (const auto& a : arr) result.push_back(albumFromJson(a)); }
    } catch (...) {}
    return result;
}
