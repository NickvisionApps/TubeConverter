#ifndef DOWNLOADMANAGER_H
#define DOWNLOADMANAGER_H

#include <memory>
#include <mutex>
#include <optional>
#include <unordered_map>
#include <vector>
#include <libnick/events/event.h>
#include <libnick/events/parameventargs.h>
#include <libnick/keyring/credential.h>
#include "historicdownload.h"
#include "download.h"
#include "downloaderoptions.h"
#include "downloadhistory.h"
#include "downloadrecoveryqueue.h"
#include "urlinfo.h"
#include "events/downloadaddedeventargs.h"
#include "events/downloadcompletedeventargs.h"
#include "events/downloadcredentialneededeventargs.h"
#include "events/downloadprogresschangedeventargs.h"

namespace Nickvision::TubeConverter::Shared::Models
{
    /**
     * @brief A manager of downloads.
     */
    class DownloadManager
    {
    public:
        /**
         * @brief Constructs a DownloadManager.
         * @param options The DownloaderOptions
         * @param history The DownloadHistory
         */
        DownloadManager(const DownloaderOptions& options, DownloadHistory& history, DownloadRecoveryQueue& recoveryQueue);
        /**
         * @brief Destructs a DownloadManager.
         */
        ~DownloadManager();
        /**
         * @brief Gets the event for when the history is changed.
         * @return The history changed event
         */
        Nickvision::Events::Event<Nickvision::Events::ParamEventArgs<std::vector<Models::HistoricDownload>>>& historyChanged();
        /**
         * @brief Gets the event for when a download is added.
         * @return The download added event
         */
        Nickvision::Events::Event<Events::DownloadAddedEventArgs>& downloadAdded();
        /**
         * @brief Gets the event for when a download is completed.
         * @return The download completed event
         */
        Nickvision::Events::Event<Events::DownloadCompletedEventArgs>& downloadCompleted();
        /**
         * @brief Gets the event for when a download's progress is changed.
         * @return The download progress changed event
         */
        Nickvision::Events::Event<Events::DownloadProgressChangedEventArgs>& downloadProgressChanged();
        /**
         * @brief Gets the event for when a download is stopped.
         * @return The download stopped event
         */
        Nickvision::Events::Event<Nickvision::Events::ParamEventArgs<int>>& downloadStopped();
        /**
         * @brief Gets the event for when a download is retried.
         * @return The download retried event
         */
        Nickvision::Events::Event<Nickvision::Events::ParamEventArgs<int>>& downloadRetried();
        /**
         * @brief Gets the event for when a download is started from the queue.
         * @return The download started from queue event
         */
        Nickvision::Events::Event<Nickvision::Events::ParamEventArgs<int>>& downloadStartedFromQueue();
        /**
         * @brief Gets the event for when a credential is needed for a download.
         * @return The download credential needed event
         */
        Nickvision::Events::Event<Events::DownloadCredentialNeededEventArgs>& downloadCredentialNeeded();
        /**
         * @brief Gets the remaining downloads count.
         * @return The remaining downloads count
         */
        size_t getRemainingDownloadsCount() const;
        /**
         * @brief Gets the downloading count.
         * @return The downloading count
         */
        size_t getDownloadingCount() const;
        /**
         * @brief Gets the queued count.
         * @return The queued count
         */
        size_t getQueuedCount() const;
        /**
         * @brief Gets the completed count.
         * @return The completed count
         */
        size_t getCompletedCount() const;
        /**
         * @brief Gets the options used for the downloader.
         * @return The DownloaderOptions
         */
        const DownloaderOptions& getDownloaderOptions() const;
        /**
         * @brief Sets the options to use for the downloader.
         * @param options The DownloaderOptions
         */
        void setDownloaderOptions(const DownloaderOptions& options);
        /**
         * @brief Gets the log of a download.
         * @param id The id of the download
         * @return The download log
         */
        const std::string& getDownloadLog(int id) const;
        /**
         * @brief Gets the command used to start a download.
         * @param id The id of the download
         * @return The download command
         */
        const std::string& getDownloadCommand(int id) const;
        /**
         * @brief Gets the status of a download.
         * @param id The id of the download
         * @return The download status
         */
        DownloadStatus getDownloadStatus(int id) const;
        /**
         * @brief Loads the download history.
         * @brief This method invokes the historyChanged event.
         * @brief This method will recover previous downloads that were interrupted by a crash.
         * @param recoverDownloads Whether or not to recover downloads
         * @return The number of downloads recovered
         */
        size_t startup(bool recoverDownloads);
        /**
         * @brief Clears the download history.
         * @brief This method invokes the historyChanged event.
         */
        void clearHistory();
        /**
         * @brief Removes a historic download from the history.
         * @brief This method invokes the historyChanged event.
         * @param download The historic download to remove
         */
        void removeHistoricDownload(const HistoricDownload& download);
        /**
         * @brief Fetches information about a URL.
         * @param url The URL to fetch information for
         * @param credential An optional credential to use for authentication
         * @return The UrlInfo if successful, else std::nullopt
         */
        std::optional<UrlInfo> fetchUrlInfo(const std::string& url, const std::optional<Keyring::Credential>& credential) const;
        /**
         * @brief Fetches information about a set of URLs from a batch file.
         * @param batchFile The batch file with listed URLs
         * @param credential An optional credential to use for authentication
         * @return The UrlInfo if successful, else std::nullopt
         */
        std::optional<UrlInfo> fetchUrlInfoFromBatchFile(const std::filesystem::path& batchFile, const std::optional<Keyring::Credential>& credential) const;
        /**
         * @brief Adds a download to the queue.
         * @brief This will invoke the downloadAdded event if added successfully.
         * @param options The options for the download
         * @param recovered Whether or not the download was previously recovered
         */
        void addDownload(const DownloadOptions& options, bool recovered = false);
        /**
         * @brief Requests that a download be stopped.
         * @brief This will invoke the downloadStopped event if stopped successfully.
         * @param id The id of the download to stop
         */
        void stopDownload(int id);
        /**
         * @brief Requests that a download be retried.
         * @brief This will invoke the downloadRetried event if retried successfully.
         * @param id The id of the download to retry
         */
        void retryDownload(int id);
        /**
         * @brief Requests that all downloads be stopped.
         * @brief This will invoke the downloadStopped event for each download stopped.
         */
        void stopAllDownloads();
        /**
         * @brief Requests that all failed downloads be retried.
         * @brief This will invoke the downloadRetried event for each download retried.
         */
        void retryFailedDownloads();
        /**
         * @brief Clears all downloads from the queue.
         * @return The ids of the downloads cleared
         */
        std::vector<int> clearQueuedDownloads();
        /**
         * @brief Clears all completed downloads.
         * @return The ids of the downloads cleared
         */
        std::vector<int> clearCompletedDownloads();

    private:
        /**
         * @brief Adds a download to the queue.
         * @param download The download to add
         * @param recovered Whether or not the download was previously recovered
         */
        void addDownload(const std::shared_ptr<Download>& download, bool recovered = false);
        /**
         * @brief Handles when a download's progress is changed.
         * @param args Events::DownloadProgressChangedEventArgs
         */
        void onDownloadProgressChanged(const Events::DownloadProgressChangedEventArgs& args);
        /**
         * @brief Handles when a download is completed.
         * @param args Events::DownloadCompletedEventArgs
         */
        void onDownloadCompleted(const Events::DownloadCompletedEventArgs& args);
        mutable std::mutex m_mutex;
        DownloaderOptions m_options;
        DownloadHistory& m_history;
        DownloadRecoveryQueue& m_recoveryQueue;
        std::unordered_map<int, std::shared_ptr<Download>> m_downloading;
        std::unordered_map<int, std::shared_ptr<Download>> m_queued;
        std::unordered_map<int, std::shared_ptr<Download>> m_completed;
        Nickvision::Events::Event<Nickvision::Events::ParamEventArgs<std::vector<HistoricDownload>>> m_historyChanged;
        Nickvision::Events::Event<Events::DownloadAddedEventArgs> m_downloadAdded;
        Nickvision::Events::Event<Events::DownloadCompletedEventArgs> m_downloadCompleted;
        Nickvision::Events::Event<Events::DownloadProgressChangedEventArgs> m_downloadProgressChanged;
        Nickvision::Events::Event<Nickvision::Events::ParamEventArgs<int>> m_downloadStopped;
        Nickvision::Events::Event<Nickvision::Events::ParamEventArgs<int>> m_downloadRetried;
        Nickvision::Events::Event<Nickvision::Events::ParamEventArgs<int>> m_downloadStartedFromQueue;
        Nickvision::Events::Event<Events::DownloadCredentialNeededEventArgs> m_downloadCredentialNeeded;
    };
}

#endif //DOWNLOADMANAGER_H
