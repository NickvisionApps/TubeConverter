#if (defined(_WIN32) && !defined(_CRT_SECURE_NO_WARNINGS))
#define _CRT_SECURE_NO_WARNINGS
#endif

#ifndef MAINWINDOWCONTROLLER_H
#define MAINWINDOWCONTROLLER_H

#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <vector>
#include <libnick/app/appinfo.h>
#include <libnick/app/datafilemanager.h>
#include <libnick/app/windowgeometry.h>
#include <libnick/events/event.h>
#include <libnick/keyring/keyring.h>
#include <libnick/notifications/notificationsenteventargs.h>
#include <libnick/notifications/shellnotificationsenteventargs.h>
#include <libnick/system/suspendinhibitor.h>
#include <libnick/taskbar/taskbaritem.h>
#include <libnick/update/updater.h>
#include "controllers/adddownloaddialogcontroller.h"
#include "controllers/credentialdialogcontroller.h"
#include "controllers/keyringdialogcontroller.h"
#include "controllers/preferencesviewcontroller.h"
#include "models/downloadmanager.h"
#include "models/historicdownload.h"
#include "models/startupinformation.h"
#include "models/theme.h"

namespace Nickvision::TubeConverter::Shared::Controllers
{
    /**
     * @brief A controller for a MainWindow.
     */
    class MainWindowController
    {
    public:
        /**
         * @brief Constructs a MainWindowController.
         * @param args A list of argument strings for the application
         */
        MainWindowController(const std::vector<std::string>& args);
        /**
         * @brief Gets the Saved event for the application's configuration.
         * @return The configuration Saved event
         */
        Nickvision::Events::Event<Nickvision::Events::EventArgs>& configurationSaved();
        /**
         * @brief Gets the event for when a notification is sent.
         * @return The notification sent event
         */
        Nickvision::Events::Event<Nickvision::Notifications::NotificationSentEventArgs>& notificationSent();
        /**
         * @brief Gets the event for when a shell notification is sent.
         * @return The shell notification sent event
         */
        Nickvision::Events::Event<Nickvision::Notifications::ShellNotificationSentEventArgs>& shellNotificationSent();
        /**
         * @brief Gets the AppInfo object for the application
         * @return The current AppInfo object
         */
        const Nickvision::App::AppInfo& getAppInfo() const;
        /**
         * @brief Gets the DownloadManager for the application.
         * @return The DownloadManager
         */
        Models::DownloadManager& getDownloadManager();
        /**
         * @brief Gets the preferred theme for the application.
         * @return The preferred theme
         */
        Models::Theme getTheme();
        /**
         * @brief Sets whether or not to show the disclaimer on startup.
         * @param showDisclaimerOnStartup True to show the disclaimer, else false
         */
        void setShowDisclaimerOnStartup(bool showDisclaimerOnStartup);
        /**
         * @brief Sets whether or not the MainWindow is active.
         * @param isWindowActive True if the MainWindow is active, else false
         */
        void setIsWindowActive(bool isWindowActive);
        /**
         * @brief Gets the debugging information for the application.
         * @param extraInformation Extra, ui-specific, information to include in the debug info statement
         * @return The application's debug information
         */
        std::string getDebugInformation(const std::string& extraInformation = "") const;
        /**
         * @brief Gets whether or not the application can be shut down.
         * @return True if can shut down, else false
         */
        bool canShutdown() const;
        /**
         * @brief Gets the help url for a specific page.
         * @param pageName The name of the page to get the help url for
         * @return The help url for the page
         */
        std::string getHelpUrl(const std::string& pageName = "index");
        /**
         * @brief Gets an AddDownloadDialogController.
         * @return The AddDownloadDialogController
         */
        std::shared_ptr<AddDownloadDialogController> createAddDownloadDialogController();
        /**
         * @brief Gets a CredentialDialogController.
         * @param args The DownloadCredentialNeededEventArgs
         * @return The CredentialDialogController
         */
        std::shared_ptr<CredentialDialogController> createCredentialDialogController(const Events::DownloadCredentialNeededEventArgs& args);
        /**
         * @brief Gets a KeyringDialogController.
         * @return The KeyringDialogController
         */
        std::shared_ptr<KeyringDialogController> createKeyringDialogController();
        /**
         * @brief Gets a PreferencesViewController.
         * @return The PreferencesViewController
         */
        std::shared_ptr<PreferencesViewController> createPreferencesViewController();
        /**
         * @brief Starts the application.
         * @brief Will only have an effect on the first time called.
         * @return The WindowGeometry to use for the application window at startup
         */
#ifdef _WIN32
        const Models::StartupInformation& startup(HWND hwnd);
#elif defined(__linux__)
        const Models::StartupInformation& startup(const std::string& desktopFile);
#else
        const Models::StartupInformation& startup();
#endif
        /**
         * @brief Shuts down the application.
         * @param geometry The window geometry to save
         */
        void shutdown(const Nickvision::App::WindowGeometry& geometry);
        /**
         * @brief Checks for an application update and sends a notification if one is available.
         */
        void checkForUpdates();
#ifdef _WIN32
        /**
         * @brief Downloads and installs the latest application update in the background.
         * @brief Will send a notification if the update fails.
         * @brief MainWindowController::checkForUpdates() must be called before this method.
         */
        void windowsUpdate();
#endif

    private:
        /**
         * @brief Handles when the configuration is saved.
         */
        void onConfigurationSaved();
        /**
         * @brief Handles when a download is completed.
         * @param args DownloadCompletedEventArgs
         */
        void onDownloadCompleted(const Events::DownloadCompletedEventArgs& args);
        bool m_started;
        std::vector<std::string> m_args;
        Nickvision::App::AppInfo m_appInfo;
        Nickvision::App::DataFileManager m_dataFileManager;
        std::shared_ptr<Nickvision::Update::Updater> m_updater;
        Nickvision::Taskbar::TaskbarItem m_taskbar;
        Nickvision::System::SuspendInhibitor m_suspendInhibitor;
        Nickvision::Keyring::Keyring m_keyring;
        Models::DownloadManager m_downloadManager;
        bool m_isWindowActive;
        Nickvision::Events::Event<Nickvision::Notifications::NotificationSentEventArgs> m_notificationSent;
        Nickvision::Events::Event<Nickvision::Notifications::ShellNotificationSentEventArgs> m_shellNotificationSent;
    };
}

#endif //MAINWINDOWCONTROLLER_H
