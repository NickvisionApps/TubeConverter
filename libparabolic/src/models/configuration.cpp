#include "models/configuration.h"

using namespace Nickvision::App;

namespace Nickvision::TubeConverter::Shared::Models
{
    Configuration::Configuration(const std::string& key, const std::string& appName)
        : DataFileBase{ key, appName }
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
#else
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

    DownloaderOptions Configuration::getDownloaderOptions() const
    {
        DownloaderOptions options;
        options.setOverwriteExistingFiles(m_json.get("OverwriteExistingFiles", true).asBool());
        options.setMaxNumberOfActiveDownloads(m_json.get("MaxNumberOfActiveDownloads", 5).asInt());
#ifdef _WIN32
        options.setLimitCharacters(m_json.get("LimitCharacters", true).asBool());
#else
        options.setLimitCharacters(m_json.get("LimitCharacters", false).asBool());
#endif
        options.setIncludeAutoGeneratedSubtitles(m_json.get("IncludeAutoGeneratedSubtitles", false).asBool());
        options.setUseAria(m_json.get("UseAria", false).asBool());
        options.setAriaMaxConnectionsPerServer(m_json.get("AriaMaxConnectionsPerServer", 16).asInt());
        options.setAriaMinSplitSize(m_json.get("AriaMinSplitSize", 20).asInt());
        options.setSpeedLimit(m_json.get("SpeedLimit", 1024).asUInt());
        options.setProxyUrl(m_json.get("ProxyUrl", "").asString());
        options.setCookiesPath(m_json.get("CookiesPath", "").asString());
        options.setYouTubeSponsorBlock(m_json.get("YouTubeSponsorBlock", false).asBool());
        options.setEmbedMetadata(m_json.get("EmbedMetadata", true).asBool());
        options.setCropAudioThumbnails(m_json.get("CropAudioThumbnails", false).asBool());
        options.setRemoveSourceData(m_json.get("RemoveSourceData", false).asBool());
        options.setEmbedChapters(m_json.get("EmbedChapters", false).asBool());
        options.setEmbedSubtitles(m_json.get("EmbedSubtitle", true).asBool());
        return options;
    }

    void Configuration::setDownloaderOptions(const DownloaderOptions& downloaderOptions)
    {
        m_json["OverwriteExistingFiles"] = downloaderOptions.getOverwriteExistingFiles();
        m_json["MaxNumberOfActiveDownloads"] = downloaderOptions.getMaxNumberOfActiveDownloads();
        m_json["LimitCharacters"] = downloaderOptions.getLimitCharacters();
        m_json["IncludeAutoGeneratedSubtitles"] = downloaderOptions.getIncludeAutoGeneratedSubtitles();
        m_json["UseAria"] = downloaderOptions.getUseAria();
        m_json["AriaMaxConnectionsPerServer"] = downloaderOptions.getAriaMaxConnectionsPerServer();
        m_json["AriaMinSplitSize"] = downloaderOptions.getAriaMinSplitSize();
        m_json["SpeedLimit"] = downloaderOptions.getSpeedLimit();
        m_json["ProxyUrl"] = downloaderOptions.getProxyUrl();
        m_json["CookiesPath"] = downloaderOptions.getCookiesPath().string();
        m_json["YouTubeSponsorBlock"] = downloaderOptions.getYouTubeSponsorBlock();
        m_json["EmbedMetadata"] = downloaderOptions.getEmbedMetadata();
        m_json["CropAudioThumbnails"] = downloaderOptions.getCropAudioThumbnails();
        m_json["RemoveSourceData"] = downloaderOptions.getRemoveSourceData();
        m_json["EmbedChapters"] = downloaderOptions.getEmbedChapters();
        m_json["EmbedSubtitle"] = downloaderOptions.getEmbedSubtitles();
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