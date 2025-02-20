#include "models/downloadmanager.h"
#include <fstream>
#include <libnick/helpers/stringhelpers.h>
#include <libnick/system/environment.h>
#include <libnick/system/process.h>

using namespace Nickvision::Events;
using namespace Nickvision::Helpers;
using namespace Nickvision::Keyring;
using namespace Nickvision::System;
using namespace Nickvision::TubeConverter::Shared::Events;

namespace Nickvision::TubeConverter::Shared::Models
{
    static std::string s_empty{};

    DownloadManager::DownloadManager(const DownloaderOptions& options, DownloadHistory& history, DownloadRecoveryQueue& recoveryQueue)
        : m_options{ options },
        m_history{ history },
        m_recoveryQueue{ recoveryQueue }
    {
        m_history.saved() += [this](const EventArgs&){ m_historyChanged.invoke(m_history.getHistory()); };
    }

    DownloadManager::~DownloadManager()
    {
        stopAllDownloads();
    }

    Event<ParamEventArgs<std::vector<HistoricDownload>>>& DownloadManager::historyChanged()
    {
        return m_historyChanged;
    }

    Event<DownloadAddedEventArgs>& DownloadManager::downloadAdded()
    {
        return m_downloadAdded;
    }

    Event<DownloadCompletedEventArgs>& DownloadManager::downloadCompleted()
    {
        return m_downloadCompleted;
    }

    Event<DownloadProgressChangedEventArgs>& DownloadManager::downloadProgressChanged()
    {
        return m_downloadProgressChanged;
    }

    Event<ParamEventArgs<int>>& DownloadManager::downloadStopped()
    {
        return m_downloadStopped;
    }

    Event<ParamEventArgs<int>>& DownloadManager::downloadRetried()
    {
        return m_downloadRetried;
    }

    Event<ParamEventArgs<int>>& DownloadManager::downloadStartedFromQueue()
    {
        return m_downloadStartedFromQueue;
    }

    Event<DownloadCredentialNeededEventArgs>& DownloadManager::downloadCredentialNeeded()
    {
        return m_downloadCredentialNeeded;
    }

    size_t DownloadManager::getRemainingDownloadsCount() const
    {
        std::lock_guard<std::mutex> lock{ m_mutex };
        return m_downloading.size() + m_queued.size();
    }

    size_t DownloadManager::getDownloadingCount() const
    {
        std::lock_guard<std::mutex> lock{ m_mutex };
        return m_downloading.size();
    }

    size_t DownloadManager::getQueuedCount() const
    {
        std::lock_guard<std::mutex> lock{ m_mutex };
        return m_queued.size();
    }

    size_t DownloadManager::getCompletedCount() const
    {
        std::lock_guard<std::mutex> lock{ m_mutex };
        return m_completed.size();
    }

    const DownloaderOptions& DownloadManager::getDownloaderOptions() const
    {
        std::lock_guard<std::mutex> lock{ m_mutex };
        return m_options;
    }

    void DownloadManager::setDownloaderOptions(const DownloaderOptions& options)
    {
        std::unique_lock<std::mutex> lock{ m_mutex };
        m_options = options;
        lock.unlock();
        while(m_downloading.size() < static_cast<size_t>(m_options.getMaxNumberOfActiveDownloads()) && !m_queued.empty())
        {
            lock.lock();
            std::shared_ptr<Download> firstQueuedDownload{ (*m_queued.begin()).second };
            m_downloading.emplace(firstQueuedDownload->getId(), firstQueuedDownload);
            m_queued.erase(firstQueuedDownload->getId());
            lock.unlock();
            m_downloadStartedFromQueue.invoke(firstQueuedDownload->getId());
            firstQueuedDownload->start(m_options);
        }
    }

    const std::string& DownloadManager::getDownloadLog(int id) const
    {
        std::lock_guard<std::mutex> lock{ m_mutex };
        if(m_downloading.contains(id))
        {
            return m_downloading.at(id)->getLog();
        }
        if(m_queued.contains(id))
        {
            return m_queued.at(id)->getLog();
        }
        if(m_completed.contains(id))
        {
            return m_completed.at(id)->getLog();
        }
        return s_empty;
    }

    const std::string& DownloadManager::getDownloadCommand(int id) const
    {
        std::lock_guard<std::mutex> lock{ m_mutex };
        if(m_downloading.contains(id))
        {
            return m_downloading.at(id)->getCommand();
        }
        if(m_queued.contains(id))
        {
            return m_queued.at(id)->getCommand();
        }
        if(m_completed.contains(id))
        {
            return m_completed.at(id)->getCommand();
        }
        return s_empty;
    }

    DownloadStatus DownloadManager::getDownloadStatus(int id) const
    {
        std::lock_guard<std::mutex> lock{ m_mutex };
        if(m_downloading.contains(id))
        {
            return m_downloading.at(id)->getStatus();
        }
        if(m_queued.contains(id))
        {
            return m_queued.at(id)->getStatus();
        }
        if(m_completed.contains(id))
        {
            return m_completed.at(id)->getStatus();
        }
        return DownloadStatus::Queued;
    }

    size_t DownloadManager::startup(bool recoverDownloads)
    {
        //Load Historic Downloads
        m_historyChanged.invoke(m_history.getHistory());
        //Recover Crashed Downloads
        if(!recoverDownloads)
        {
            m_recoveryQueue.clear();
            return 0;
        }
        std::unordered_map<int, DownloadOptions> recoverableDownloads{ m_recoveryQueue.getRecoverableDownloads() };
        m_recoveryQueue.clear();
        for(std::pair<const int, DownloadOptions>& pair : recoverableDownloads)
        {
            if(m_recoveryQueue.needsCredential(pair.first))
            {
                std::shared_ptr<Credential> credential{ std::make_shared<Credential>("", "", "", "") };
                while(credential->getUsername().empty() && credential->getPassword().empty())
                {
                    m_downloadCredentialNeeded.invoke({ pair.first, pair.second.getUrl(), credential });
                }
                pair.second.setCredential(*credential);
            }
            addDownload(pair.second, true);
        }
        return recoverableDownloads.size();
    }

    void DownloadManager::clearHistory()
    {
        m_history.clear();
    }

    void DownloadManager::removeHistoricDownload(const HistoricDownload& download)
    {
        m_history.removeDownload(download);
    }

    std::optional<UrlInfo> DownloadManager::fetchUrlInfo(const std::string& url, const std::optional<Credential>& credential) const
    {
        std::unique_lock<std::mutex> lock{ m_mutex };
        std::vector<std::string> arguments{ "--xff", "default", "--dump-single-json", "--skip-download", "--ignore-errors", "--no-warnings" };
        if(url.find("soundcloud.com") == std::string::npos)
        {
            arguments.push_back("--flat-playlist");
        }
        if(m_options.getLimitCharacters())
        {
            arguments.push_back("--windows-filenames");
        }
        if(!m_options.getProxyUrl().empty())
        {
            arguments.push_back("--proxy");
            arguments.push_back(m_options.getProxyUrl());
        }
        if(credential)
        {
            arguments.push_back("--username");
            arguments.push_back(credential->getUsername());
            arguments.push_back("--password");
            arguments.push_back(credential->getPassword());
        }
        if(m_options.getCookiesBrowser() != Browser::None && Environment::getDeploymentMode() == DeploymentMode::Local)
        {
            arguments.push_back("--cookies-from-browser");
            switch(m_options.getCookiesBrowser())
            {
            case Browser::Brave:
                arguments.push_back("brave");
                break;
            case Browser::Chrome:
                arguments.push_back("chrome");
                break;
            case Browser::Chromium:
                arguments.push_back("chromium");
                break;
            case Browser::Edge:
                arguments.push_back("edge");
                break;
            case Browser::Firefox:
                arguments.push_back("firefox");
                break;
            case Browser::Opera:
                arguments.push_back("opera");
                break;
            case Browser::Vivaldi:
                arguments.push_back("vivaldi");
                break;
            case Browser::Whale:
                arguments.push_back("whale");
                break;
            default:
                break;
            }
        }
        else if(std::filesystem::exists(m_options.getCookiesPath()))
        {
            arguments.push_back("--cookies");
            arguments.push_back(m_options.getCookiesPath().string());
        }
        arguments.push_back(url);
        Process process{ Environment::findDependency("yt-dlp"), arguments };
        process.start();
        if(process.waitForExit() != 0 || process.getOutput().empty())
        {
            return std::nullopt;
        }
        boost::json::value info = boost::json::parse(process.getOutput());
        if(!info.is_object())
        {
            return std::nullopt;
        }
        boost::json::object obj = info.as_object();
        obj["limit_characters"] = m_options.getLimitCharacters();
        //Handle YouTube Tabs as they require individual fetching
        if(obj.contains("entries") && obj["entries"].is_array())
        {
            std::vector<std::optional<UrlInfo>> urlInfos;
            for(const boost::json::value& entry : obj["entries"].as_array())
            {
                if(entry.is_object())
                {
                    boost::json::object e = entry.as_object();
                    //Check for YouTube Tab
                    if(e.contains("ie_key") && e["ie_key"].is_string() && e["ie_key"].as_string() == "YoutubeTab")
                    {
                        //Fetch UrlInfo for YoutubeTab
                        lock.unlock();
                        urlInfos.push_back(fetchUrlInfo(e["url"].as_string().c_str(), credential));
                        lock.lock();
                    }
                }
            }
            if(!urlInfos.empty())
            {
                //Build final UrlInfo
                return UrlInfo{ obj["title"].is_string() ? obj["title"].as_string().c_str() : url, urlInfos };
            }
        }
        return UrlInfo{ url, obj, m_options.getIncludeMediaIdInTitle(), m_options.getIncludeAutoGeneratedSubtitles(), m_options.getPreferredVideoCodec() };
    }

    std::optional<UrlInfo> DownloadManager::fetchUrlInfoFromBatchFile(const std::filesystem::path& batchFile, const std::optional<Credential>& credential) const
    {
        if(!std::filesystem::exists(batchFile) || batchFile.extension().string() != ".txt")
        {
            return std::nullopt;
        }
        //Fetch URL info for each URL in the batch file
        std::vector<std::optional<UrlInfo>> urlInfos;
        std::ifstream file{ batchFile };
        std::string line;
        while(std::getline(file, line))
        {
            if(StringHelpers::isValidUrl(line))
            {
                std::optional<UrlInfo> urlInfo{ fetchUrlInfo(line, credential) };
                if(urlInfo)
                {
                    urlInfos.push_back(urlInfo);
                }
            }
        }
        file.close();
        //Build final UrlInfo
        if(urlInfos.empty())
        {
            return std::nullopt;
        }
        return UrlInfo{ batchFile, urlInfos };
    }

    void DownloadManager::addDownload(const DownloadOptions& options, bool recovered)
    {
        std::unique_lock<std::mutex> lock{ m_mutex };
        //Build a Download object
        std::shared_ptr<Download> download{ std::make_shared<Download>(options) };
        download->progressChanged() += [this](const DownloadProgressChangedEventArgs& args){ onDownloadProgressChanged(args); };
        download->completed() += [this](const DownloadCompletedEventArgs& args){ onDownloadCompleted(args); };
        lock.unlock();
        //Add the download
        addDownload(download, recovered);
    }

    void DownloadManager::stopDownload(int id)
    {
        std::unique_lock<std::mutex> lock{ m_mutex };
        bool stopped{ false };
        if(m_downloading.contains(id))
        {
            m_downloading.at(id)->stop();
            m_completed.emplace(id, m_downloading.at(id));
            m_downloading.erase(id);
            stopped = true;
        }
        else if(m_queued.contains(id))
        {
            m_completed.emplace(id, m_queued.at(id));
            m_queued.erase(id);
            stopped = true;
        }
        if(stopped)
        {
            m_recoveryQueue.removeDownload(id);
            lock.unlock();
            m_downloadStopped.invoke(id);
        }
    }

    void DownloadManager::retryDownload(int id)
    {
        std::unique_lock<std::mutex> lock{ m_mutex };
        if(m_completed.contains(id))
        {
            std::shared_ptr<Download> download{ m_completed.at(id) };
            m_completed.erase(id);
            lock.unlock();
            m_downloadRetried.invoke(id);
            addDownload(download);
        }
    }

    void DownloadManager::stopAllDownloads()
    {
        std::unique_lock<std::mutex> lock{ m_mutex };
        //Get Downloading and Queued keys
        std::vector<int> keys;
        keys.reserve(m_downloading.size() + m_queued.size());
        for(const std::pair<const int, std::shared_ptr<Download>>& pair : m_downloading)
        {
            keys.push_back(pair.first);
        }
        for(const std::pair<const int, std::shared_ptr<Download>>& pair : m_queued)
        {
            keys.push_back(pair.first);
        }
        lock.unlock();
        //Stop downloads
        for(int key : keys)
        {
            stopDownload(key);
        }
    }

    void DownloadManager::retryFailedDownloads()
    {
        std::unique_lock<std::mutex> lock{ m_mutex };
        //Get Completed keys
        std::vector<int> keys;
        for(const std::pair<const int, std::shared_ptr<Download>>& pair : m_completed)
        {
            if(pair.second->getStatus() == DownloadStatus::Error)
            {
                keys.push_back(pair.first);
            }
        }
        lock.unlock();
        //Retry downloads
        for(int key : keys)
        {
            retryDownload(key);
        }
    }

    std::vector<int> DownloadManager::clearQueuedDownloads()
    {
        std::lock_guard<std::mutex> lock{ m_mutex };
        std::vector<int> cleared;
        for(const std::pair<const int, std::shared_ptr<Download>>& pair : m_queued)
        {
            cleared.push_back(pair.first);
        }
        m_queued.clear();
        return cleared;
    }

    std::vector<int> DownloadManager::clearCompletedDownloads()
    {
        std::lock_guard<std::mutex> lock{ m_mutex };
        std::vector<int> cleared;
        for(const std::pair<const int, std::shared_ptr<Download>>& pair : m_completed)
        {
            cleared.push_back(pair.first);
        }
        m_completed.clear();
        return cleared;
    }

    void DownloadManager::addDownload(const std::shared_ptr<Download>& download, bool recovered)
    {
        std::unique_lock<std::mutex> lock{ m_mutex };
        if(!recovered)
        {
            m_recoveryQueue.addDownload(download->getId(), download->getOptions());
        }
        if(m_downloading.size() < static_cast<size_t>(m_options.getMaxNumberOfActiveDownloads()))
        {
            m_downloading.emplace(download->getId(), download);
            lock.unlock();
            m_downloadAdded.invoke({ download->getId(), download->getPath(), download->getUrl(), DownloadStatus::Running });
            download->start(m_options);
        }
        else
        {
            m_queued.emplace(download->getId(), download);
            lock.unlock();
            m_downloadAdded.invoke({ download->getId(), download->getPath(), download->getUrl(), download->getStatus() });
        }
        m_history.addDownload({ download->getUrl(), download->getPath().filename().stem().string(), download->getPath() });
    }

    void DownloadManager::onDownloadProgressChanged(const DownloadProgressChangedEventArgs& args)
    {
        std::unique_lock<std::mutex> lock{ m_mutex };
        if(!m_downloading.contains(args.getId()))
        {
            return;
        }
        const std::shared_ptr<Download>& download{ m_downloading.at(args.getId()) };
        if(download->getStatus() == DownloadStatus::Stopped)
        {
            return;
        }
        lock.unlock();
        m_downloadProgressChanged.invoke(args);
    }

    void DownloadManager::onDownloadCompleted(const DownloadCompletedEventArgs& args)
    {
        std::unique_lock<std::mutex> lock{ m_mutex };
        if(!m_downloading.contains(args.getId()))
        {
            return;
        }
        std::shared_ptr<Download> download{ m_downloading.at(args.getId()) };
        if(download->getStatus() == DownloadStatus::Stopped)
        {
            return;
        }
        m_completed.emplace(download->getId(), download);
        m_downloading.erase(download->getId());
        m_recoveryQueue.removeDownload(download->getId());
        lock.unlock();
        m_downloadCompleted.invoke(args);
        lock.lock();
        //Start Download from Queue if There is Space
        if(m_downloading.size() < static_cast<size_t>(m_options.getMaxNumberOfActiveDownloads()) && !m_queued.empty())
        {
            std::shared_ptr<Download> firstQueuedDownload{ (*m_queued.begin()).second };
            m_downloading.emplace(firstQueuedDownload->getId(), firstQueuedDownload);
            m_queued.erase(firstQueuedDownload->getId());
            lock.unlock();
            m_downloadStartedFromQueue.invoke(firstQueuedDownload->getId());
            firstQueuedDownload->start(m_options);
        }
    }
}
