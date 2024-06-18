#include "models/configuration.h"

using namespace Nickvision::App;

namespace Nickvision::TubeConverter::Shared::Models
{
    Configuration::Configuration(const std::string& key)
        : ConfigurationBase{ key }
    {
    }

    Theme Configuration::getTheme() const
    {
        return static_cast<Theme>(m_json.get("Theme", static_cast<int>(Theme::System)).asInt());
    }

    void Configuration::setTheme(Theme theme)
    {
        m_json["Theme"] = static_cast<int>(theme);
    }

    WindowGeometry Configuration::getWindowGeometry() const
    {
        WindowGeometry geometry;
        const Json::Value json{ m_json["WindowGeometry"] };
        geometry.setWidth(json.get("Width", 900).asInt64());
        geometry.setHeight(json.get("Height", 700).asInt64());
        geometry.setIsMaximized(json.get("IsMaximized", false).asBool());
        return geometry;
    }

    void Configuration::setWindowGeometry(const WindowGeometry& geometry)
    {
        m_json["WindowGeometry"]["Width"] = static_cast<Json::Int64>(geometry.getWidth());
        m_json["WindowGeometry"]["Height"] = static_cast<Json::Int64>(geometry.getHeight());
        m_json["WindowGeometry"]["IsMaximized"] = geometry.isMaximized();
    }

    bool Configuration::getAutomaticallyCheckForUpdates() const
    {
#ifdef _WIN32
        bool def{ true };
#elif defined(__linux__)
        bool def{ false };
#endif
        return m_json.get("AutomaticallyCheckForUpdates", def).asBool();
    }

    void Configuration::setAutomaticallyCheckForUpdates(bool check)
    {
        m_json["AutomaticallyCheckForUpdates"] = check;
    }

    CompletedNotificationPreference Configuration::getCompletedNotificationPreference() const
    {
        return static_cast<CompletedNotificationPreference>(m_json.get("CompletedNotificationPreference", static_cast<int>(CompletedNotificationPreference::ForEach)).asInt());
    }

    void Configuration::setCompletedNotificationPreference(CompletedNotificationPreference preference)
    {
        m_json["CompletedNotificationPreference"] = static_cast<int>(preference);
    }

    bool Configuration::getPreventSuspendWhenDownloading() const
    {
        return m_json.get("PreventSuspendWhenDownloading", false).asBool();
    }

    void Configuration::setPreventSuspendWhenDownloading(bool prevent)
    {
        m_json["PreventSuspendWhenDownloading"] = prevent;
    }

    bool Configuration::getOverwriteExistingFiles() const
    {
        return m_json.get("OverwriteExistingFiles", true).asBool();
    }

    void Configuration::setOverwriteExistingFiles(bool overwrite)
    {
        m_json["OverwriteExistingFiles"] = overwrite;
    }

    int Configuration::getMaxNumberOfActiveDownloads() const
    {
        return m_json.get("MaxNumberOfActiveDownloads", 5).asInt();
    }

    void Configuration::setMaxNumberOfActiveDownloads(int max)
    {
        if(max < 1 || max > 10)
        {
            max = 5;
        }
        m_json["MaxNumberOfActiveDownloads"] = max;
    }

    bool Configuration::getLimitCharacters() const
    {
#ifdef _WIN32
        return m_json.get("LimitCharacters", true).asBool();
#else
        return m_json.get("LimitCharacters", false).asBool();
#endif
    }

    void Configuration::setLimitCharacters(bool limit)
    {
        m_json["LimitCharacters"] = limit;
    }

    bool Configuration::getIncludeAutoGeneratedSubtitles() const
    {
        return m_json.get("IncludeAutoGeneratedSubtitles", false).asBool();
    }

    void Configuration::setIncludeAutoGeneratedSubtitles(bool include)
    {
        m_json["IncludeAutoGeneratedSubtitles"] = include;
    }

    bool Configuration::getUseAria() const
    {
        return m_json.get("UseAria", false).asBool();
    }

    void Configuration::setUseAria(bool useAria)
    {
        m_json["UseAria"] = useAria;
    }

    int Configuration::getAriaMaxConnectionsPerServer() const
    {
        return m_json.get("AriaMaxConnectionsPerServer", 16).asInt();
    }

    void Configuration::setAriaMaxConnectionsPerServer(int maxConnections)
    {
        if(maxConnections < 1 || maxConnections > 16)
        {
            maxConnections = 16;
        }
        m_json["AriaMaxConnectionsPerServer"] = maxConnections;
    }

    int Configuration::getAriaMinSplitSize() const
    {
        return m_json.get("AriaMinSplitSize", 20).asInt();
    }

    void Configuration::setAriaMinSplitSize(int minSplitSize)
    {
        if(minSplitSize < 1 || minSplitSize > 1024)
        {
            minSplitSize = 20;
        }
        m_json["AriaMinSplitSize"] = minSplitSize;
    }

    unsigned int Configuration::getSpeedLimit() const
    {
        
        return m_json.get("SpeedLimit", 1024).asUInt();
    }

    void Configuration::setSpeedLimit(unsigned int speedLimit)
    {
        if(speedLimit < 512 || speedLimit > 10240)
        {
            speedLimit = 1024;
        }
        m_json["SpeedLimit"] = speedLimit;
    }

    std::string Configuration::getProxyUrl() const
    {
        return m_json.get("ProxyUrl", "").asString();
    }

    void Configuration::setProxyUrl(const std::string& proxyUrl)
    {
        m_json["ProxyUrl"] = proxyUrl;
    }

    std::filesystem::path Configuration::getCookiesPath() const
    {
        std::filesystem::path path{ m_json.get("CookiesPath", "").asString() };
        if(std::filesystem::exists(path))
        {
            return path;
        }
        return {};
    }

    void Configuration::setCookiesPath(const std::filesystem::path& cookiesPath)
    {
        if(std::filesystem::exists(cookiesPath))
        {
            m_json["CookiesPath"] = cookiesPath.string();
        }
        else
        {
            m_json["CookiesPath"] = "";
        }       
    }

    bool Configuration::getYouTubeSponsorBlock() const
    {
        return m_json.get("YouTubeSponsorBlock", false).asBool();
    }

    void Configuration::setYouTubeSponsorBlock(bool sponsorBlock)
    {
        m_json["YouTubeSponsorBlock"] = sponsorBlock;
    }

    bool Configuration::getDisallowConversions() const
    {
        return m_json.get("DisallowConversions", false).asBool();
    }

    void Configuration::setDisallowConversions(bool disallowConversions)
    {
        m_json["DisallowConversions"] = disallowConversions;
    }

    bool Configuration::getEmbedMetadata() const
    {
        return m_json.get("EmbedMetadata", true).asBool();
    }

    void Configuration::setEmbedMetadata(bool embedMetadata)
    {
        m_json["EmbedMetadata"] = embedMetadata;
    }

    bool Configuration::getCropAudioThumbnails() const
    {
        return m_json.get("CropAudioThumbnails", false).asBool();
    }

    void Configuration::setCropAudioThumbnails(bool cropAudioThumbnails)
    {
        m_json["CropAudioThumbnails"] = cropAudioThumbnails;
    }

    bool Configuration::getRemoveSourceData() const
    {
        return m_json.get("RemoveSourceData", false).asBool();
    }

    void Configuration::setRemoveSourceData(bool removeSourceData)
    {
        m_json["RemoveSourceData"] = removeSourceData;
    }

    bool Configuration::getEmbedChapters() const
    {
        return m_json.get("EmbedChapters", false).asBool();
    }

    void Configuration::setEmbedChapters(bool embedChapters)
    {
        m_json["EmbedChapters"] = embedChapters;
    }

    bool Configuration::getEmbedSubtitles() const
    {
        return m_json.get("EmbedSubtitle", true).asBool();
    }

    void Configuration::setEmbedSubtitles(bool embedSubtitle)
    {
        m_json["EmbedSubtitle"] = embedSubtitle;
    }

    std::filesystem::path Configuration::getPreviousSaveFolder() const
    {
        std::filesystem::path path{ m_json.get("PreviousSaveFolder", "").asString() };
        if(std::filesystem::exists(path))
        {
            return path;
        }
        return {};
    }

    void Configuration::setPreviousSaveFolder(const std::filesystem::path& previousSaveFolder)
    {
        if(std::filesystem::exists(previousSaveFolder))
        {
            m_json["PreviousSaveFolder"] = previousSaveFolder.string();
        }
        else
        {
            m_json["PreviousSaveFolder"] = "";
        }
    }

    MediaFileType Configuration::getPreviousMediaFileType() const
    {
        return { static_cast<MediaFileType::MediaFileTypeValue>(m_json.get("PreviousMediaFileType", static_cast<int>(MediaFileType::MP4)).asInt()) };
    }

    void Configuration::setPreviousMediaFileType(const MediaFileType& previousMediaFileType)
    {
        m_json["PreviousMediaFileType"] = static_cast<int>(previousMediaFileType);
    }

    MediaFileType Configuration::getPreviousGenericMediaFileType() const
    {
        return { static_cast<MediaFileType::MediaFileTypeValue>(m_json.get("PreviousGenericMediaFileType", static_cast<int>(MediaFileType::Video)).asInt()) };
    }

    void Configuration::setPreviousGenericMediaFileType(const MediaFileType& previousGenericMediaFileType)
    {
        if(previousGenericMediaFileType.isGeneric())
        {
            m_json["PreviousGenericMediaFileType"] = static_cast<int>(previousGenericMediaFileType);
        }
        else
        {
            m_json["PreviousGenericMediaFileType"] = static_cast<int>(MediaFileType::Video);
        }
    }

    VideoResolution Configuration::getPreviousVideoResolution() const
    {
        std::optional<VideoResolution> resolution{ VideoResolution::parse(m_json.get("PreviousVideoResolution", "").asString()) };
        if(resolution)
        {
            return resolution.value();
        }
        return VideoResolution{ -1, -1 };
    }

    void Configuration::setPreviousVideoResolution(const VideoResolution& previousVideoResolution)
    {
        m_json["PreviousVideoResolution"] = previousVideoResolution.str();
    }

    bool Configuration::getPreviousSubtitleState() const
    {
        return m_json.get("PreviousSubtitleState", false).asBool();
    }

    void Configuration::setPreviousSubtitleState(bool previousSubtitleState)
    {
        m_json["PreviousSubtitleState"] = previousSubtitleState;
    }

    bool Configuration::getPreviousPreferAV1State() const
    {
        return m_json.get("PreviousPreferAV1State", false).asBool();
    }

    void Configuration::setPreviousPreferAV1State(bool previousPreferAV1State)
    {
        m_json["PreviousPreferAV1State"] = previousPreferAV1State;
    }

    bool Configuration::getNumberTitles() const
    {
        return m_json.get("NumberTitles", false).asBool();
    }

    void Configuration::setNumberTitles(bool numberTitles)
    {
        m_json["NumberTitles"] = numberTitles;
    }

    bool Configuration::getShowDisclaimerOnStartup() const
    {
        return m_json.get("ShowDisclaimerOnStartup", true).asBool();
    }

    void Configuration::setShowDisclaimerOnStartup(bool showDisclaimerOnStartup)
    {
        m_json["ShowDisclaimerOnStartup"] = showDisclaimerOnStartup;
    }
}