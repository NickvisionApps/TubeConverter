#include "views/mainwindow.h"
#include <filesystem>
#include <format>
#include <thread>
#include <libnick/app/appinfo.h>
#include <libnick/helpers/codehelpers.h>
#include <libnick/notifications/shellnotification.h>
#include <libnick/localization/gettext.h>
#include "helpers/builder.h"
#include "helpers/dialogptr.h"
#include "helpers/gtkhelpers.h"
#include "views/adddownloaddialog.h"
#include "views/credentialdialog.h"
#include "views/keyringpage.h"
#include "views/preferencesdialog.h"

using namespace Nickvision::App;
using namespace Nickvision::Events;
using namespace Nickvision::Helpers;
using namespace Nickvision::Keyring;
using namespace Nickvision::Notifications;
using namespace Nickvision::TubeConverter::GNOME::Controls;
using namespace Nickvision::TubeConverter::GNOME::Helpers;
using namespace Nickvision::TubeConverter::Shared::Controllers;
using namespace Nickvision::TubeConverter::Shared::Events;
using namespace Nickvision::TubeConverter::Shared::Models;
using namespace Nickvision::Update;

namespace Nickvision::TubeConverter::GNOME::Views
{
    enum Pages
    {
        Home = 0,
        Keyring = 1,
        History = 2,
        Downloading = 4,
        Queued = 5,
        Completed = 6
    };

    MainWindow::MainWindow(const std::shared_ptr<MainWindowController>& controller, GtkApplication* app)
        : m_controller{ controller },
        m_app{ app },
        m_builder{ "main_window" },
        m_window{ m_builder.get<AdwApplicationWindow>("root") }
    {
        //Setup Window
        gtk_application_add_window(GTK_APPLICATION(app), GTK_WINDOW(m_window));
        gtk_window_set_title(GTK_WINDOW(m_window), m_controller->getAppInfo().getShortName().c_str());
        gtk_window_set_icon_name(GTK_WINDOW(m_window), m_controller->getAppInfo().getId().c_str());
        if(m_controller->getAppInfo().getVersion().getVersionType() == VersionType::Preview)
        {
            gtk_widget_add_css_class(GTK_WIDGET(m_window), "devel");
        }
        adw_window_title_set_title(m_builder.get<AdwWindowTitle>("title"), m_controller->getAppInfo().getShortName().c_str());
        //Register Events
        g_signal_connect(m_window, "close_request", G_CALLBACK(+[](GtkWindow*, gpointer data) -> bool { return reinterpret_cast<MainWindow*>(data)->onCloseRequested(); }), this);
        g_signal_connect(m_window, "notify::is-active", G_CALLBACK(+[](GObject*, GParamSpec* pspec, gpointer data){ reinterpret_cast<MainWindow*>(data)->onVisibilityChanged(); }), this);
        g_signal_connect(m_window, "notify::visible", G_CALLBACK(+[](GObject*, GParamSpec* pspec, gpointer data){ reinterpret_cast<MainWindow*>(data)->onVisibilityChanged(); }), this);
        g_signal_connect(m_builder.get<GObject>("listNavItems"), "row-activated", G_CALLBACK(+[](GtkListBox*, GtkListBoxRow*, gpointer data) { adw_navigation_split_view_set_show_content(reinterpret_cast<MainWindow*>(data)->m_builder.get<AdwNavigationSplitView>("navView"), true); }), this);
        g_signal_connect(m_builder.get<GObject>("listNavItems"), "row-selected", G_CALLBACK(+[](GtkListBox* self, GtkListBoxRow* row, gpointer data) { reinterpret_cast<MainWindow*>(data)->onNavItemSelected(self, row); }), this);
        m_controller->notificationSent() += [this](const NotificationSentEventArgs& args) { GtkHelpers::dispatchToMainThread([this, args]{ onNotificationSent(args); }); };
        m_controller->shellNotificationSent() += [this](const ShellNotificationSentEventArgs& args) { onShellNotificationSent(args); };
        m_controller->getDownloadManager().historyChanged() += [this](const ParamEventArgs<std::vector<HistoricDownload>>& args) { GtkHelpers::dispatchToMainThread([this, args]{ onHistoryChanged(args); }); };
        m_controller->getDownloadManager().downloadCredentialNeeded() += [this](const DownloadCredentialNeededEventArgs& args) { onDownloadCredentialNeeded(args); };
        m_controller->getDownloadManager().downloadAdded() += [this](const DownloadAddedEventArgs& args) { GtkHelpers::dispatchToMainThread([this, args]{ onDownloadAdded(args); }); };
        m_controller->getDownloadManager().downloadCompleted() += [this](const DownloadCompletedEventArgs& args) { GtkHelpers::dispatchToMainThread([this, args]{ onDownloadCompleted(args); }); };
        m_controller->getDownloadManager().downloadProgressChanged() += [this](const DownloadProgressChangedEventArgs& args) { GtkHelpers::dispatchToMainThread([this, args]{ onDownloadProgressChanged(args); }); };
        m_controller->getDownloadManager().downloadStopped() += [this](const ParamEventArgs<int>& args) { GtkHelpers::dispatchToMainThread([this, args]{ onDownloadStopped(args); }); };
        m_controller->getDownloadManager().downloadRetried() += [this](const ParamEventArgs<int>& args) { GtkHelpers::dispatchToMainThread([this, args]{ onDownloadRetried(args); }); };
        m_controller->getDownloadManager().downloadStartedFromQueue() += [this](const ParamEventArgs<int>& args) { GtkHelpers::dispatchToMainThread([this, args]{ onDownloadStartedFromQueue(args); }); };
        //Quit Action
        GSimpleAction* actQuit{ g_simple_action_new("quit", nullptr) };
        g_signal_connect(actQuit, "activate", G_CALLBACK(+[](GSimpleAction*, GVariant*, gpointer data){ reinterpret_cast<MainWindow*>(data)->quit(); }), this);
        g_action_map_add_action(G_ACTION_MAP(m_window), G_ACTION(actQuit));
        GtkHelpers::setAccelForAction(m_app, "win.quit", "<Ctrl>Q");
        //Preferences Action
        GSimpleAction* actPreferences{ g_simple_action_new("preferences", nullptr) };
        g_signal_connect(actPreferences, "activate", G_CALLBACK(+[](GSimpleAction*, GVariant*, gpointer data){ reinterpret_cast<MainWindow*>(data)->preferences(); }), this);
        g_action_map_add_action(G_ACTION_MAP(m_window), G_ACTION(actPreferences));
        GtkHelpers::setAccelForAction(m_app, "win.preferences", "<Ctrl>comma");
        //Keyboard Shortcuts Action
        GSimpleAction* actKeyboardShortcuts{ g_simple_action_new("keyboardShortcuts", nullptr) };
        g_signal_connect(actKeyboardShortcuts, "activate", G_CALLBACK(+[](GSimpleAction*, GVariant*, gpointer data){ reinterpret_cast<MainWindow*>(data)->keyboardShortcuts(); }), this);
        g_action_map_add_action(G_ACTION_MAP(m_window), G_ACTION(actKeyboardShortcuts));
        GtkHelpers::setAccelForAction(m_app, "win.keyboardShortcuts", "<Ctrl>question");
        //Help Action
        GSimpleAction* actHelp{ g_simple_action_new("help", nullptr) };
        g_signal_connect(actHelp, "activate", G_CALLBACK(+[](GSimpleAction*, GVariant*, gpointer data){ reinterpret_cast<MainWindow*>(data)->help(); }), this);
        g_action_map_add_action(G_ACTION_MAP(m_window), G_ACTION(actHelp));
        GtkHelpers::setAccelForAction(m_app, "win.help", "F1");
        //About Action
        GSimpleAction* actAbout{ g_simple_action_new("about", nullptr) };
        g_signal_connect(actAbout, "activate", G_CALLBACK(+[](GSimpleAction*, GVariant*, gpointer data){ reinterpret_cast<MainWindow*>(data)->about(); }), this);
        g_action_map_add_action(G_ACTION_MAP(m_window), G_ACTION(actAbout));
        //Add Download Action
        m_actAddDownload = g_simple_action_new("addDownload", nullptr);
        g_signal_connect(m_actAddDownload, "activate", G_CALLBACK(+[](GSimpleAction*, GVariant*, gpointer data){ reinterpret_cast<MainWindow*>(data)->addDownload(); }), this);
        g_action_map_add_action(G_ACTION_MAP(m_window), G_ACTION(m_actAddDownload));
        GtkHelpers::setAccelForAction(m_app, "win.addDownload", "<Ctrl>N");
        //Clear History Action
        GSimpleAction* actClearHistory{ g_simple_action_new("clearHistory", nullptr) };
        g_signal_connect(actClearHistory, "activate", G_CALLBACK(+[](GSimpleAction*, GVariant*, gpointer data){ reinterpret_cast<MainWindow*>(data)->clearHistory(); }), this);
        g_action_map_add_action(G_ACTION_MAP(m_window), G_ACTION(actClearHistory));
        //Stop All Downloads Action
        GSimpleAction* actStopAllDownloads{ g_simple_action_new("stopAllDownloads", nullptr) };
        g_signal_connect(actStopAllDownloads, "activate", G_CALLBACK(+[](GSimpleAction*, GVariant*, gpointer data){ reinterpret_cast<MainWindow*>(data)->stopAllDownloads(); }), this);
        g_action_map_add_action(G_ACTION_MAP(m_window), G_ACTION(actStopAllDownloads));
        GtkHelpers::setAccelForAction(m_app, "win.stopAllDownloads", "<Ctrl><Shift>Delete");
        //Clear Queued Downloads Action
        GSimpleAction* actClearQueuedDownloads{ g_simple_action_new("clearQueuedDownloads", nullptr) };
        g_signal_connect(actClearQueuedDownloads, "activate", G_CALLBACK(+[](GSimpleAction*, GVariant*, gpointer data){ reinterpret_cast<MainWindow*>(data)->clearQueuedDownloads(); }), this);
        g_action_map_add_action(G_ACTION_MAP(m_window), G_ACTION(actClearQueuedDownloads));
        //Clear Completed Downloads Action
        GSimpleAction* actClearCompletedDownloads{ g_simple_action_new("clearCompletedDownloads", nullptr) };
        g_signal_connect(actClearCompletedDownloads, "activate", G_CALLBACK(+[](GSimpleAction*, GVariant*, gpointer data){ reinterpret_cast<MainWindow*>(data)->clearCompletedDownloads(); }), this);
        g_action_map_add_action(G_ACTION_MAP(m_window), G_ACTION(actClearCompletedDownloads));
        //Retry Failed Downloads Action
        GSimpleAction* actRetryFailedDownloads{ g_simple_action_new("retryFailedDownloads", nullptr) };
        g_signal_connect(actRetryFailedDownloads, "activate", G_CALLBACK(+[](GSimpleAction*, GVariant*, gpointer data){ reinterpret_cast<MainWindow*>(data)->retryFailedDownloads(); }), this);
        g_action_map_add_action(G_ACTION_MAP(m_window), G_ACTION(actRetryFailedDownloads));
        GtkHelpers::setAccelForAction(m_app, "win.retryFailedDownloads", "<Ctrl>R");
    }

    MainWindow::~MainWindow()
    {
        gtk_window_destroy(GTK_WINDOW(m_window));
    }

    void MainWindow::show()
    {
        gtk_window_present(GTK_WINDOW(m_window));
#ifdef __linux__
        const StartupInformation& info{ m_controller->startup(m_controller->getAppInfo().getId() + ".desktop") };
#else
        const StartupInformation& info{ m_controller->startup() };
#endif
        gtk_window_set_default_size(GTK_WINDOW(m_window), static_cast<int>(info.getWindowGeometry().getWidth()), static_cast<int>(info.getWindowGeometry().getHeight()));
        if(info.getWindowGeometry().isMaximized())
        {
            gtk_window_maximize(GTK_WINDOW(m_window));
        }
        gtk_list_box_select_row(m_builder.get<GtkListBox>("listNavItems"), gtk_list_box_get_row_at_index(m_builder.get<GtkListBox>("listNavItems"), Pages::Home));
        adw_view_stack_set_visible_child_name(m_builder.get<AdwViewStack>("downloadingViewStack"), "no-downloading");
        adw_view_stack_set_visible_child_name(m_builder.get<AdwViewStack>("queuedViewStack"), "no-queued");
        adw_view_stack_set_visible_child_name(m_builder.get<AdwViewStack>("completedViewStack"), "no-completed");
        g_simple_action_set_enabled(m_actAddDownload, info.canDownload());
        if(info.showDisclaimer())
        {
            AdwAlertDialog* dialog{ ADW_ALERT_DIALOG(adw_alert_dialog_new(_("Disclaimer"), _("The authors of Nickvision Parabolic are not responsible/liable for any misuse of this program that may violate local copyright/DMCA laws. Users use this application at their own risk."))) };
            adw_alert_dialog_set_extra_child(dialog, gtk_check_button_new_with_label(_("Don't show this message again")));
            adw_alert_dialog_add_responses(dialog, "close", _("Close"), nullptr);
            adw_alert_dialog_set_default_response(dialog, "close");
            adw_alert_dialog_set_close_response(dialog, "close");
            g_signal_connect(dialog, "response", G_CALLBACK(+[](AdwAlertDialog* self, const char*, gpointer data)
            {
                MainWindow* mainWindow{ reinterpret_cast<MainWindow*>(data) };
                mainWindow->m_controller->setShowDisclaimerOnStartup(!gtk_check_button_get_active(GTK_CHECK_BUTTON(adw_alert_dialog_get_extra_child(self))));
            }), this);
            adw_dialog_present(ADW_DIALOG(dialog), GTK_WIDGET(m_window));
        }
        if(!info.getUrlToValidate().empty())
        {
            addDownload(info.getUrlToValidate());
        }
    }

    void MainWindow::addDownload(const std::string& url)
    {
        DialogPtr<AddDownloadDialog> dialog{ m_controller->createAddDownloadDialogController(), url, GTK_WINDOW(m_window) };
        dialog->present();
    }

    bool MainWindow::onCloseRequested()
    {
        if(!m_controller->canShutdown())
        {
            AdwAlertDialog* dialog{ ADW_ALERT_DIALOG(adw_alert_dialog_new(_("Exit?"), _("There are downloads in progress. Are you sure you want to exit?"))) };
            adw_alert_dialog_add_responses(dialog, "yes", _("Yes"), "no", _("No"), nullptr);
            adw_alert_dialog_set_response_appearance(dialog, "yes", ADW_RESPONSE_DESTRUCTIVE);
            adw_alert_dialog_set_default_response(dialog, "no");
            adw_alert_dialog_set_close_response(dialog, "no");
            g_signal_connect(dialog, "response", G_CALLBACK(+[](AdwAlertDialog* self, const char* response, gpointer data)
            {
                if(std::string(response) == "yes")
                {
                    MainWindow* mainWindow{ reinterpret_cast<MainWindow*>(data) };
                    mainWindow->m_controller->getDownloadManager().stopAllDownloads();
                    gtk_window_close(GTK_WINDOW(mainWindow->m_window));
                }
            }), this);
            adw_dialog_present(ADW_DIALOG(dialog), GTK_WIDGET(m_window));
            return true;
        }
        int width;
        int height;
        gtk_window_get_default_size(GTK_WINDOW(m_window), &width, &height);
        m_controller->shutdown({ width, height, static_cast<bool>(gtk_window_is_maximized(GTK_WINDOW(m_window))) });
        return false;
    }

    void MainWindow::onVisibilityChanged()
    {
        m_controller->setIsWindowActive(gtk_window_is_active(GTK_WINDOW(m_window)) && gtk_widget_is_visible(GTK_WIDGET(m_window)));
    }

    void MainWindow::onNotificationSent(const NotificationSentEventArgs& args)
    {
        AdwToast* toast{ adw_toast_new(args.getMessage().c_str()) };
        adw_toast_overlay_add_toast(m_builder.get<AdwToastOverlay>("toastOverlay"), toast);
    }

    void MainWindow::onShellNotificationSent(const ShellNotificationSentEventArgs& args)
    {
#ifdef __linux__
        ShellNotification::send(args, m_controller->getAppInfo().getId(), _("Open"));
#else
        ShellNotification::send(args);
#endif
    }

    void MainWindow::onNavItemSelected(GtkListBox* box, GtkListBoxRow* row)
    {
        adw_navigation_split_view_set_show_content(m_builder.get<AdwNavigationSplitView>("navView"), true);
        if(row == gtk_list_box_get_row_at_index(box, Pages::Home))
        {
            adw_navigation_page_set_title(m_builder.get<AdwNavigationPage>("navPageContent"), _("Home"));
            adw_view_stack_set_visible_child_name(m_builder.get<AdwViewStack>("viewStack"), "home");
        }
        else if(row == gtk_list_box_get_row_at_index(box, Pages::Keyring))
        {
            ControlPtr<KeyringPage> keyringPage{ m_controller->createKeyringDialogController(), m_builder.get<AdwToastOverlay>("toastOverlay"), GTK_WINDOW(m_window) };
            adw_navigation_page_set_title(m_builder.get<AdwNavigationPage>("navPageContent"), _("Keyring"));
            adw_view_stack_set_visible_child_name(m_builder.get<AdwViewStack>("viewStack"), "keyring");
            adw_bin_set_child(m_builder.get<AdwBin>("binKeyring"), GTK_WIDGET(keyringPage->gobj()));
        }
        else if(row == gtk_list_box_get_row_at_index(box, Pages::History))
        {
            adw_navigation_page_set_title(m_builder.get<AdwNavigationPage>("navPageContent"), _("History"));
            adw_view_stack_set_visible_child_name(m_builder.get<AdwViewStack>("viewStack"), "history");
        }
        else if(row == gtk_list_box_get_row_at_index(box, Pages::Downloading))
        {
            adw_navigation_page_set_title(m_builder.get<AdwNavigationPage>("navPageContent"), _("Downloading"));
            adw_view_stack_set_visible_child_name(m_builder.get<AdwViewStack>("viewStack"), "downloading");
        }
        else if(row == gtk_list_box_get_row_at_index(box, Pages::Queued))
        {
            adw_navigation_page_set_title(m_builder.get<AdwNavigationPage>("navPageContent"), _("Queued"));
            adw_view_stack_set_visible_child_name(m_builder.get<AdwViewStack>("viewStack"), "queued");
        }
        else if(row == gtk_list_box_get_row_at_index(box, Pages::Completed))
        {
            adw_navigation_page_set_title(m_builder.get<AdwNavigationPage>("navPageContent"), _("Completed"));
            adw_view_stack_set_visible_child_name(m_builder.get<AdwViewStack>("viewStack"), "completed");
        }
    }

    void MainWindow::onHistoryChanged(const ParamEventArgs<std::vector<HistoricDownload>>& args)
    {
        for(AdwActionRow* row : m_historyRows)
        {
            adw_preferences_group_remove(m_builder.get<AdwPreferencesGroup>("historyGroup"), GTK_WIDGET(row));
        }
        m_historyRows.clear();
        adw_view_stack_set_visible_child_name(m_builder.get<AdwViewStack>("historyViewStack"), args.getParam().size() > 0 ? "history" : "no-history");
        for(const HistoricDownload& download : args.getParam())
        {
            //Row
            AdwActionRow* row{ ADW_ACTION_ROW(adw_action_row_new()) };
            adw_preferences_row_set_use_markup(ADW_PREFERENCES_ROW(row), false);
            adw_preferences_row_set_title(ADW_PREFERENCES_ROW(row), download.getTitle().c_str());
            adw_action_row_set_subtitle(row, download.getUrl().c_str());
            adw_preferences_group_add(m_builder.get<AdwPreferencesGroup>("historyGroup"), GTK_WIDGET(row));
            m_historyRows.push_back(row);
            //Play button
            if(std::filesystem::exists(download.getPath()))
            {
                GtkButton* playButton{ GTK_BUTTON(gtk_button_new_from_icon_name("media-playback-start-symbolic")) };
                gtk_widget_set_valign(GTK_WIDGET(playButton), GTK_ALIGN_CENTER);
                gtk_widget_set_tooltip_text(GTK_WIDGET(playButton), _("Play"));
                gtk_widget_add_css_class(GTK_WIDGET(playButton), "flat");
                g_signal_connect_data(playButton, "clicked", G_CALLBACK(+[](GtkButton*, gpointer data)
                {
                    std::filesystem::path* path{ reinterpret_cast<std::filesystem::path*>(data) };
                    GtkFileLauncher* launcher{ gtk_file_launcher_new(g_file_new_for_path(path->string().c_str())) };
                    gtk_file_launcher_launch(launcher, nullptr, nullptr, GAsyncReadyCallback(+[](GObject* source, GAsyncResult* res, gpointer)
                    { 
                        gtk_file_launcher_launch_finish(GTK_FILE_LAUNCHER(source), res, nullptr); 
                        g_object_unref(source);
                    }), nullptr);
                }), new std::filesystem::path(download.getPath()), GClosureNotify(+[](gpointer data, GClosure*)
                {
                    delete reinterpret_cast<std::filesystem::path*>(data);
                }), G_CONNECT_DEFAULT);
                adw_action_row_add_suffix(row, GTK_WIDGET(playButton));
            }
            //Download button
            GtkButton* downloadButton{ GTK_BUTTON(gtk_button_new_from_icon_name("document-save-symbolic")) };
            std::pair<MainWindow*, HistoricDownload>* downloadPair{ new std::pair<MainWindow*, HistoricDownload>(this, download) };
            gtk_widget_set_valign(GTK_WIDGET(downloadButton), GTK_ALIGN_CENTER);
            gtk_widget_set_tooltip_text(GTK_WIDGET(downloadButton), _("Download Again"));
            gtk_widget_add_css_class(GTK_WIDGET(downloadButton), "flat");
            g_signal_connect_data(downloadButton, "clicked", GCallback(+[](GtkButton*, gpointer data)
            {
                std::pair<MainWindow*, HistoricDownload>* pair{ reinterpret_cast<std::pair<MainWindow*, HistoricDownload>*>(data) };
                pair->first->addDownload(pair->second.getUrl());
            }), downloadPair, GClosureNotify(+[](gpointer data, GClosure*)
            {
                delete reinterpret_cast<std::pair<MainWindow*, HistoricDownload>*>(data);
            }), G_CONNECT_DEFAULT);
            adw_action_row_add_suffix(row, GTK_WIDGET(downloadButton));
            //Delete button
            GtkButton* deleteButton{ GTK_BUTTON(gtk_button_new_from_icon_name("user-trash-symbolic")) };
            std::pair<MainWindow*, HistoricDownload>* deletePair{ new std::pair<MainWindow*, HistoricDownload>(this, download) };
            gtk_widget_set_valign(GTK_WIDGET(deleteButton), GTK_ALIGN_CENTER);
            gtk_widget_set_tooltip_text(GTK_WIDGET(deleteButton), _("Delete"));
            gtk_widget_add_css_class(GTK_WIDGET(deleteButton), "flat");
            g_signal_connect(deleteButton, "clicked", GCallback(+[](GtkButton*, gpointer data)
            {
                std::pair<MainWindow*, HistoricDownload>* pair{ reinterpret_cast<std::pair<MainWindow*, HistoricDownload>*>(data) };
                pair->first->m_controller->getDownloadManager().removeHistoricDownload(pair->second);
                delete pair;
            }), deletePair);
            adw_action_row_add_suffix(row, GTK_WIDGET(deleteButton));
        }
    }

    void MainWindow::onDownloadCredentialNeeded(const DownloadCredentialNeededEventArgs& args)
    {
        bool closed;
        DialogPtr<CredentialDialog> dialog{ m_controller->createCredentialDialogController(args), GTK_WINDOW(m_window) };
        dialog->closed() += [&closed](const EventArgs&){ closed = true; };
        dialog->present();
        while(!closed)
        {
            g_main_context_iteration(g_main_context_default(), true);
            std::this_thread::sleep_for(std::chrono::microseconds(1));
        }
    }

    void MainWindow::onDownloadAdded(const DownloadAddedEventArgs& args)
    {
        gtk_list_box_select_row(m_builder.get<GtkListBox>("listNavItems"), gtk_list_box_get_row_at_index(m_builder.get<GtkListBox>("listNavItems"), Pages::Downloading));
        ControlPtr<DownloadRow> row{ args, GTK_WINDOW(m_window) };
        row->stopped() += [this](const ParamEventArgs<int>& args){ m_controller->getDownloadManager().stopDownload(args.getParam()); };
        row->retried() += [this](const ParamEventArgs<int>& args){ m_controller->getDownloadManager().retryDownload(args.getParam()); };
        row->commandToClipboardRequested() += [this](const ParamEventArgs<int>& args){ gdk_clipboard_set_text(gdk_display_get_clipboard(gdk_display_get_default()), m_controller->getDownloadManager().getDownloadCommand(args.getParam()).c_str()); };
        if(args.getStatus() == DownloadStatus::Queued)
        {
            GtkHelpers::addToBox(m_builder.get<GtkBox>("listQueued"), GTK_WIDGET(row->gobj()), true);
            adw_view_stack_set_visible_child_name(m_builder.get<AdwViewStack>("queuedViewStack"), "queued");
            gtk_label_set_label(m_builder.get<GtkLabel>("queuedCountLabel"), std::to_string(m_controller->getDownloadManager().getQueuedCount()).c_str());
        }
        else
        {
            GtkHelpers::addToBox(m_builder.get<GtkBox>("listDownloading"), GTK_WIDGET(row->gobj()), true);
            adw_view_stack_set_visible_child_name(m_builder.get<AdwViewStack>("downloadingViewStack"), "downloading");
            gtk_label_set_label(m_builder.get<GtkLabel>("downloadingCountLabel"), std::to_string(m_controller->getDownloadManager().getDownloadingCount()).c_str());
        }
        m_downloadRows[args.getId()] = row;
    }

    void MainWindow::onDownloadCompleted(const DownloadCompletedEventArgs& args)
    {
        m_downloadRows[args.getId()]->setCompleteState(args);
        GtkHelpers::moveFromBox(m_builder.get<GtkBox>("listDownloading"), m_builder.get<GtkBox>("listCompleted"), GTK_WIDGET(m_downloadRows[args.getId()]->gobj()), true);
        adw_view_stack_set_visible_child_name(m_builder.get<AdwViewStack>("downloadingViewStack"), m_controller->getDownloadManager().getDownloadingCount() > 0 ? "downloading" : "no-downloading");
        adw_view_stack_set_visible_child_name(m_builder.get<AdwViewStack>("completedViewStack"), "completed");
        gtk_label_set_label(m_builder.get<GtkLabel>("downloadingCountLabel"), std::to_string(m_controller->getDownloadManager().getDownloadingCount()).c_str());
        gtk_label_set_label(m_builder.get<GtkLabel>("completedCountLabel"), std::to_string(m_controller->getDownloadManager().getCompletedCount()).c_str());
    }

    void MainWindow::onDownloadProgressChanged(const DownloadProgressChangedEventArgs& args)
    {
        m_downloadRows[args.getId()]->setProgressState(args);
    }

    void MainWindow::onDownloadStopped(const ParamEventArgs<int>& args)
    {
        m_downloadRows[args.getParam()]->setStopState();
        GtkHelpers::moveFromBox(m_builder.get<GtkBox>("listDownloading"), m_builder.get<GtkBox>("listCompleted"), GTK_WIDGET(m_downloadRows[args.getParam()]->gobj()), true);
        GtkHelpers::moveFromBox(m_builder.get<GtkBox>("listQueued"), m_builder.get<GtkBox>("listCompleted"), GTK_WIDGET(m_downloadRows[args.getParam()]->gobj()), true);
        adw_view_stack_set_visible_child_name(m_builder.get<AdwViewStack>("downloadingViewStack"), m_controller->getDownloadManager().getDownloadingCount() > 0 ? "downloading" : "no-downloading");
        adw_view_stack_set_visible_child_name(m_builder.get<AdwViewStack>("queuedViewStack"), m_controller->getDownloadManager().getQueuedCount() > 0 ? "queued" : "no-queued");
        adw_view_stack_set_visible_child_name(m_builder.get<AdwViewStack>("completedViewStack"), "completed");
        gtk_label_set_label(m_builder.get<GtkLabel>("downloadingCountLabel"), std::to_string(m_controller->getDownloadManager().getDownloadingCount()).c_str());
        gtk_label_set_label(m_builder.get<GtkLabel>("queuedCountLabel"), std::to_string(m_controller->getDownloadManager().getQueuedCount()).c_str());
        gtk_label_set_label(m_builder.get<GtkLabel>("completedCountLabel"), std::to_string(m_controller->getDownloadManager().getCompletedCount()).c_str());
    }

    void MainWindow::onDownloadRetried(const ParamEventArgs<int>& args)
    {
        m_downloadRows[args.getParam()]->setStartFromQueueState();
        GtkHelpers::moveFromBox(m_builder.get<GtkBox>("listCompleted"), m_builder.get<GtkBox>("listDownloading"), GTK_WIDGET(m_downloadRows[args.getParam()]->gobj()), true);
        adw_view_stack_set_visible_child_name(m_builder.get<AdwViewStack>("completedViewStack"), m_controller->getDownloadManager().getCompletedCount() > 0 ? "completed" : "no-completed");
        adw_view_stack_set_visible_child_name(m_builder.get<AdwViewStack>("downloadingViewStack"), "downloading");
        gtk_label_set_label(m_builder.get<GtkLabel>("downloadingCountLabel"), std::to_string(m_controller->getDownloadManager().getDownloadingCount()).c_str());
        gtk_label_set_label(m_builder.get<GtkLabel>("completedCountLabel"), std::to_string(m_controller->getDownloadManager().getCompletedCount()).c_str());
    }

    void MainWindow::onDownloadStartedFromQueue(const ParamEventArgs<int>& args)
    {
        m_downloadRows[args.getParam()]->setStartFromQueueState();
        GtkHelpers::moveFromBox(m_builder.get<GtkBox>("listQueued"), m_builder.get<GtkBox>("listDownloading"), GTK_WIDGET(m_downloadRows[args.getParam()]->gobj()), true);
        adw_view_stack_set_visible_child_name(m_builder.get<AdwViewStack>("queuedViewStack"), m_controller->getDownloadManager().getQueuedCount() > 0 ? "queued" : "no-queued");
        adw_view_stack_set_visible_child_name(m_builder.get<AdwViewStack>("downloadingViewStack"), "downloading");
        gtk_label_set_label(m_builder.get<GtkLabel>("downloadingCountLabel"), std::to_string(m_controller->getDownloadManager().getDownloadingCount()).c_str());
        gtk_label_set_label(m_builder.get<GtkLabel>("queuedCountLabel"), std::to_string(m_controller->getDownloadManager().getQueuedCount()).c_str());
    }

    void MainWindow::quit()
    {
        if(!onCloseRequested())
        {
            g_application_quit(G_APPLICATION(m_app));
        }
    }

    void MainWindow::preferences()
    {
        DialogPtr<PreferencesDialog> dialog{ m_controller->createPreferencesViewController(), GTK_WINDOW(m_window) };
        dialog->present();
    }

    void MainWindow::keyboardShortcuts()
    {
        Builder builderHelp{ "shortcuts_dialog" };
        GtkShortcutsWindow* shortcuts{ builderHelp.get<GtkShortcutsWindow>("root") };
        gtk_window_set_transient_for(GTK_WINDOW(shortcuts), GTK_WINDOW(m_window));
        gtk_window_set_icon_name(GTK_WINDOW(shortcuts), m_controller->getAppInfo().getId().c_str());
        gtk_window_present(GTK_WINDOW(shortcuts));
    }

    void MainWindow::help()
    {
        std::string helpUrl{ m_controller->getHelpUrl() };
        gtk_show_uri(GTK_WINDOW(m_window), helpUrl.c_str(), GDK_CURRENT_TIME);
    }

    void MainWindow::about()
    {
        std::string extraDebug;
        extraDebug += "GTK " + std::to_string(gtk_get_major_version()) + "." + std::to_string(gtk_get_minor_version()) + "." + std::to_string(gtk_get_micro_version()) + "\n";
        extraDebug += "libadwaita " + std::to_string(adw_get_major_version()) + "." + std::to_string(adw_get_minor_version()) + "." + std::to_string(adw_get_micro_version()) + "\n";
        AdwAboutDialog* dialog{ ADW_ABOUT_DIALOG(adw_about_dialog_new()) };
        adw_about_dialog_set_application_name(dialog, m_controller->getAppInfo().getShortName().c_str());
        adw_about_dialog_set_application_icon(dialog, std::string(m_controller->getAppInfo().getId() + (m_controller->getAppInfo().getVersion().getVersionType() == VersionType::Preview  ? "-devel" : "")).c_str());
        adw_about_dialog_set_developer_name(dialog, "Nickvision");
        adw_about_dialog_set_version(dialog, m_controller->getAppInfo().getVersion().str().c_str());
        adw_about_dialog_set_release_notes(dialog, m_controller->getAppInfo().getHtmlChangelog().c_str());
        adw_about_dialog_set_debug_info(dialog, m_controller->getDebugInformation(extraDebug).c_str());
        adw_about_dialog_set_comments(dialog, m_controller->getAppInfo().getDescription().c_str());
        adw_about_dialog_set_license_type(dialog, GTK_LICENSE_GPL_3_0);
        adw_about_dialog_set_copyright(dialog, "© Nickvision 2021-2025");
        adw_about_dialog_set_website(dialog, "https://nickvision.org/");
        adw_about_dialog_set_issue_url(dialog, m_controller->getAppInfo().getIssueTracker().c_str());
        adw_about_dialog_set_support_url(dialog, m_controller->getAppInfo().getSupportUrl().c_str());
        adw_about_dialog_add_link(dialog, _("GitHub Repo"), m_controller->getAppInfo().getSourceRepo().c_str());
        for(const std::pair<std::string, std::string>& pair : m_controller->getAppInfo().getExtraLinks())
        {
            adw_about_dialog_add_link(dialog, pair.first.c_str(), pair.second.c_str());
        }
        std::vector<const char*> urls;
        std::vector<std::string> developers{ AppInfo::convertUrlMapToVector(m_controller->getAppInfo().getDevelopers()) };
        for(const std::string& developer : developers)
        {
            urls.push_back(developer.c_str());
        }
        urls.push_back(nullptr);
        adw_about_dialog_set_developers(dialog, &urls[0]);
        urls.clear();
        std::vector<std::string> designers{ AppInfo::convertUrlMapToVector(m_controller->getAppInfo().getDesigners()) };
        for(const std::string& designer : designers)
        {
            urls.push_back(designer.c_str());
        }
        urls.push_back(nullptr);
        adw_about_dialog_set_designers(dialog, &urls[0]);
        urls.clear();
        std::vector<std::string> artists{ AppInfo::convertUrlMapToVector(m_controller->getAppInfo().getArtists()) };
        for(const std::string& artist : artists)
        {
            urls.push_back(artist.c_str());
        }
        urls.push_back(nullptr);
        adw_about_dialog_set_artists(dialog, &urls[0]);
        adw_about_dialog_set_translator_credits(dialog, m_controller->getAppInfo().getTranslatorCredits().c_str());
        adw_dialog_present(ADW_DIALOG(dialog), GTK_WIDGET(m_window));
    }

    void MainWindow::clearHistory()
    {
        m_controller->getDownloadManager().clearHistory();
    }

    void MainWindow::stopAllDownloads()
    {
        m_controller->getDownloadManager().stopAllDownloads();
    }

    void MainWindow::clearQueuedDownloads()
    {
        for(int id : m_controller->getDownloadManager().clearQueuedDownloads())
        {
            gtk_box_remove(m_builder.get<GtkBox>("listQueued"), GTK_WIDGET(m_downloadRows[id]->gobj()));
            m_downloadRows.erase(id);
        }
        adw_view_stack_set_visible_child_name(m_builder.get<AdwViewStack>("queuedViewStack"), "no-queued");
        gtk_label_set_label(m_builder.get<GtkLabel>("queuedCountLabel"), std::to_string(m_controller->getDownloadManager().getQueuedCount()).c_str());
    }

    void MainWindow::clearCompletedDownloads()
    {
        for(int id : m_controller->getDownloadManager().clearCompletedDownloads())
        {
            gtk_box_remove(m_builder.get<GtkBox>("listCompleted"), GTK_WIDGET(m_downloadRows[id]->gobj()));
            m_downloadRows.erase(id);
        }
        adw_view_stack_set_visible_child_name(m_builder.get<AdwViewStack>("completedViewStack"), "no-completed");
        gtk_label_set_label(m_builder.get<GtkLabel>("completedCountLabel"), std::to_string(m_controller->getDownloadManager().getCompletedCount()).c_str());
    }

    void MainWindow::retryFailedDownloads()
    {
        m_controller->getDownloadManager().retryFailedDownloads();
    }
}
