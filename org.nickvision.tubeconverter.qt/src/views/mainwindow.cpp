#include "views/mainwindow.h"
#include "ui_mainwindow.h"
#include <QCheckBox>
#include <QDesktopServices>
#include <QFileDialog>
#include <QMessageBox>
#include <QMimeData>
#include <libnick/helpers/codehelpers.h>
#include <libnick/localization/gettext.h>
#include <libnick/notifications/shellnotification.h>
#include "controls/aboutdialog.h"
#include "controls/historyrow.h"
#include "helpers/qthelpers.h"
#include "views/adddownloaddialog.h"
#include "views/credentialdialog.h"
#include "views/keyringpage.h"
#include "views/settingspage.h"

using namespace Nickvision::App;
using namespace Nickvision::Events;
using namespace Nickvision::Helpers;
using namespace Nickvision::Keyring;
using namespace Nickvision::Notifications;
using namespace Nickvision::TubeConverter::Qt::Controls;
using namespace Nickvision::TubeConverter::Qt::Helpers;
using namespace Nickvision::TubeConverter::Shared::Controllers;
using namespace Nickvision::TubeConverter::Shared::Events;
using namespace Nickvision::TubeConverter::Shared::Models;
using namespace Nickvision::Update;

namespace Nickvision::TubeConverter::Qt::Views
{
    enum Page
    {
        Home = 0,
        Keyring,
        History,
        Downloading,
        Queued,
        Completed,
        NoDownloading,
        NoQueued,
        NoCompleted,
        Settings
    };

    MainWindow::MainWindow(const std::shared_ptr<MainWindowController>& controller, oclero::qlementine::ThemeManager* themeManager, QWidget* parent) 
        : QMainWindow{ parent },
        m_ui{ new Ui::MainWindow() },
        m_navigationBar{ new NavigationBar(this) },
        m_controller{ controller },
        m_themeManager{ themeManager }
    {
        m_ui->setupUi(this);
        m_ui->mainLayout->insertLayout(0, m_navigationBar);
        setWindowTitle(m_controller->getAppInfo().getVersion().getVersionType() == VersionType::Stable ? _("Parabolic") : _("Parabolic (Preview)"));
        //Navigation Bar
        QMenu* helpMenu{ new QMenu(this) };
        helpMenu->addAction(_("Check for Updates"), this, &MainWindow::checkForUpdates);
        helpMenu->addSeparator();
        helpMenu->addAction(_("Documentation"), this, &MainWindow::documentation);
        helpMenu->addAction(_("GitHub Repo"), this, &MainWindow::gitHubRepo);
        helpMenu->addAction(_("Report a Bug"), this, &MainWindow::reportABug);
        helpMenu->addAction(_("Discussions"), this, &MainWindow::discussions);
        helpMenu->addSeparator();
        helpMenu->addAction(_("About"), this, &MainWindow::about);
        m_navigationBar->addTopItem("home", _("Home"), QIcon::fromTheme(QIcon::ThemeIcon::GoHome));
        m_navigationBar->addTopItem("keyring", _("Keyring"), QIcon::fromTheme(QIcon::ThemeIcon::DialogPassword));
        m_navigationBar->addTopItem("history", _("History"), QIcon::fromTheme(QIcon::ThemeIcon::EditFind));
        m_navigationBar->addTopItem("downloading", _("Downloading"), QIcon::fromTheme("emblem-downloads"));
        m_navigationBar->addTopItem("queued", _("Queued"), QIcon::fromTheme(QIcon::ThemeIcon::ListAdd));
        m_navigationBar->addTopItem("completed", _("Completed"), QIcon::fromTheme(QIcon::ThemeIcon::DocumentSave));
        m_navigationBar->addBottomItem("help", _("Help"), QIcon::fromTheme(QIcon::ThemeIcon::HelpAbout), helpMenu);
#ifdef _WIN32
        m_navigationBar->addBottomItem("settings", _("Settings"), QIcon::fromTheme("document-properties"));
#else
        m_navigationBar->addBottomItem("settings", _("Settings"), QIcon::fromTheme(QIcon::ThemeIcon::DocumentProperties));
#endif
        //Localize Home Page
        m_ui->lblHomeGreeting->setText(_("Download Media"));
        m_ui->lblHomeDescription->setText(_("Add a video, audio, or playlist URL to start downloading"));
        m_ui->btnHomeAddDownload->setText(_("Add Download"));
        //Localize History Page
        m_ui->lblNoHIstoryIcon->setPixmap(QIcon::fromTheme(QIcon::ThemeIcon::EditFind).pixmap(64, 64));
        m_ui->lblNoHistory->setText(_("No history available"));
        m_ui->btnClearHistory->setText(_("Clear"));
        //Localize Downloading Page
        m_ui->lblNoDownloading->setText(_("No Downloads Running"));
        m_ui->lblNoDownloadingIcon->setPixmap(QIcon::fromTheme("emblem-downloads").pixmap(64, 64));
        m_ui->btnNoDownloadingAddDownload->setText(_("Add Download"));
        m_ui->btnStopAllDownloads->setText(_("Stop All"));
        m_ui->btnStopAllDownloads->setToolTip(_("Stop All Downloads"));
        //Localize Queued Page
        m_ui->lblNoQueued->setText(_("No Downloads Queued"));
        m_ui->lblNoQueuedIcon->setPixmap(QIcon::fromTheme(QIcon::ThemeIcon::ListAdd).pixmap(64, 64));
        m_ui->btnNoQueuedAddDownload->setText(_("Add Download"));
        m_ui->btnClearQueuedDownloads->setText(_("Clear"));
        m_ui->btnClearQueuedDownloads->setToolTip(_("Clear Queued Downloads"));
        //Localize Completed Page
        m_ui->lblNoCompleted->setText(_("No Downloads Completed"));
        m_ui->lblNoCompletedIcon->setPixmap(QIcon::fromTheme(QIcon::ThemeIcon::DocumentSave).pixmap(64, 64));
        m_ui->btnNoCompletedAddDownload->setText(_("Add Download"));
        m_ui->btnRetryFailedDownloads->setText(_("Retry Failed"));
        m_ui->btnRetryFailedDownloads->setToolTip(_("Retry Failed Downloads"));
        m_ui->btnClearCompletedDownloads->setText(_("Clear"));
        m_ui->btnClearCompletedDownloads->setToolTip(_("Clear Completed Downloads"));
        //Localize Log Dock
        m_dockLogCloseEventFilter = new CloseEventFilter(m_ui->dockLog);
        m_ui->dockLog->installEventFilter(m_dockLogCloseEventFilter);
        m_ui->dockLog->setWindowTitle(_("Log"));
        //Signals
        connect(m_navigationBar, &NavigationBar::itemSelected, this, &MainWindow::onNavigationItemSelected);
        connect(m_ui->btnHomeAddDownload, &QPushButton::clicked, [this]() { addDownload(); });
        connect(m_ui->btnClearHistory, &QPushButton::clicked, this, &MainWindow::clearHistory);
        connect(m_ui->btnStopAllDownloads, &QPushButton::clicked, this, &MainWindow::stopAllDownloads);
        connect(m_ui->listDownloading, &QListWidget::itemSelectionChanged, this, &MainWindow::onDownloadListSelectionChanged);
        connect(m_ui->btnNoDownloadingAddDownload, &QPushButton::clicked, [this]() { addDownload(); });
        connect(m_ui->btnClearQueuedDownloads, &QPushButton::clicked, this, &MainWindow::clearQueuedDownloads);
        connect(m_ui->listQueued, &QListWidget::itemSelectionChanged, this, &MainWindow::onDownloadListSelectionChanged);
        connect(m_ui->btnNoQueuedAddDownload, &QPushButton::clicked, [this]() { addDownload(); });
        connect(m_ui->btnRetryFailedDownloads, &QPushButton::clicked, this, &MainWindow::retryFailedDownloads);
        connect(m_ui->btnClearCompletedDownloads, &QPushButton::clicked, this, &MainWindow::clearCompletedDownloads);
        connect(m_ui->listCompleted, &QListWidget::itemSelectionChanged, this, &MainWindow::onDownloadListSelectionChanged);
        connect(m_ui->btnNoCompletedAddDownload, &QPushButton::clicked, [this]() { addDownload(); });
        connect(m_dockLogCloseEventFilter, &CloseEventFilter::closed, this, &MainWindow::onDockLogClosed);
        //Events
        m_controller->notificationSent() += [&](const NotificationSentEventArgs& args) { QtHelpers::dispatchToMainThread([this, args]() { onNotificationSent(args); }); };
        m_controller->shellNotificationSent() += [&](const ShellNotificationSentEventArgs& args) { onShellNotificationSent(args); };
        m_controller->getDownloadManager().historyChanged() += [&](const ParamEventArgs<std::vector<HistoricDownload>>& args) { QtHelpers::dispatchToMainThread([this, args]() { onHistoryChanged(args); }); };
        m_controller->getDownloadManager().downloadCredentialNeeded() += [&](const DownloadCredentialNeededEventArgs& args) { onDownloadCredentialNeeded(args); };
        m_controller->getDownloadManager().downloadAdded() += [&](const DownloadAddedEventArgs& args) { QtHelpers::dispatchToMainThread([this, args]() { onDownloadAdded(args); }); };
        m_controller->getDownloadManager().downloadCompleted() += [&](const DownloadCompletedEventArgs& args) { QtHelpers::dispatchToMainThread([this, args]() { onDownloadCompleted(args); }); };
        m_controller->getDownloadManager().downloadProgressChanged() += [&](const DownloadProgressChangedEventArgs& args) { QtHelpers::dispatchToMainThread([this, args]() { onDownloadProgressChanged(args); }); };
        m_controller->getDownloadManager().downloadStopped() += [&](const ParamEventArgs<int>& args) { QtHelpers::dispatchToMainThread([this, args]() { onDownloadStopped(args); }); };
        m_controller->getDownloadManager().downloadRetried() += [&](const ParamEventArgs<int>& args) { QtHelpers::dispatchToMainThread([this, args]() { onDownloadRetried(args); }); };
        m_controller->getDownloadManager().downloadStartedFromQueue() += [&](const ParamEventArgs<int>& args) { QtHelpers::dispatchToMainThread([this, args]() { onDownloadStartedFromQueue(args); }); };
    }

    MainWindow::~MainWindow()
    {
        delete m_ui;
    }

    void MainWindow::show()
    {
        QMainWindow::show();
        m_ui->dockLog->hide();
#ifdef _WIN32
        const StartupInformation& info{ m_controller->startup(reinterpret_cast<HWND>(winId())) };
#elif defined(__linux__)
        const StartupInformation& info{ m_controller->startup(m_controller->getAppInfo().getId() + ".desktop") };
#else
        const StartupInformation& info{ m_controller->startup() };
#endif
        setGeometry(QWidget::geometry().x(), QWidget::geometry().y(), info.getWindowGeometry().getWidth(), info.getWindowGeometry().getHeight());
        if(info.getWindowGeometry().isMaximized())
        {
            showMaximized();
        }
        m_navigationBar->selectItem("home");
        m_ui->btnHomeAddDownload->setEnabled(info.canDownload());
        m_ui->btnNoDownloadingAddDownload->setEnabled(info.canDownload());
        m_ui->btnNoQueuedAddDownload->setEnabled(info.canDownload());
        m_ui->btnNoCompletedAddDownload->setEnabled(info.canDownload());
        if(info.showDisclaimer())
        {
            QMessageBox msgBox{ QMessageBox::Icon::Information, _("Disclaimer"), _("The authors of Nickvision Parabolic are not responsible/liable for any misuse of this program that may violate local copyright/DMCA laws. Users use this application at their own risk."), QMessageBox::StandardButton::Ok, this };
            QCheckBox* checkBox{ new QCheckBox(_("Don't show this message again"), &msgBox) };
            msgBox.setCheckBox(checkBox);
            msgBox.exec();
            m_controller->setShowDisclaimerOnStartup(!checkBox->isChecked());
        }
        if(!info.getUrlToValidate().empty())
        {
            addDownload(info.getUrlToValidate());
        }
    }

    void MainWindow::closeEvent(QCloseEvent* event)
    {
        if(!m_controller->canShutdown())
        {
            event->ignore();
            QMessageBox msgBox{ QMessageBox::Icon::Warning, QString::fromStdString(m_controller->getAppInfo().getShortName()), _("There are downloads in progress. Are you sure you want to exit?"), QMessageBox::StandardButton::Yes | QMessageBox::StandardButton::No, this };
            if(msgBox.exec() == QMessageBox::StandardButton::Yes)
            {
                m_controller->getDownloadManager().stopAllDownloads();
                close();
            }
            return;
        }
        m_controller->shutdown({ geometry().width(), geometry().height(), isMaximized() });
        event->accept();
    }

    void MainWindow::changeEvent(QEvent* event)
    {
        if(event->type() == QEvent::WindowStateChange)
        {
            m_controller->setIsWindowActive(isActiveWindow());
        }
        QMainWindow::changeEvent(event);
    }

    void MainWindow::onNavigationItemSelected(const QString& id)
    {
        //Ensure new KeyringPage
        if(m_ui->viewStack->widget(Page::Keyring))
        {
            KeyringPage* oldKeyring{ qobject_cast<KeyringPage*>(m_ui->viewStack->widget(Page::Keyring)) };
            if(oldKeyring)
            {
                m_ui->viewStack->removeWidget(oldKeyring);
                delete oldKeyring;
            }
        }
        m_ui->viewStack->insertWidget(Page::Keyring, new KeyringPage(m_controller->createKeyringDialogController(), this));
        //Save and ensure new SettingsPage
        if(m_ui->viewStack->widget(Page::Settings))
        {
            SettingsPage* oldSettings{ qobject_cast<SettingsPage*>(m_ui->viewStack->widget(Page::Settings)) };
            oldSettings->close();
            m_ui->viewStack->removeWidget(oldSettings);
            delete oldSettings;
        }
        m_ui->viewStack->insertWidget(Page::Settings, new SettingsPage(m_controller->createPreferencesViewController(), m_themeManager, this));
        //Navigate to new page
        if(id == "home")
        {
            m_ui->viewStack->setCurrentIndex(Page::Home);
        }
        else if(id == "keyring")
        {
            m_ui->viewStack->setCurrentIndex(Page::Keyring);
        }
        else if(id == "history")
        {
            m_ui->viewStack->setCurrentIndex(Page::History);
        }
        else if(id == "downloading")
        {
            m_ui->viewStack->setCurrentIndex(m_controller->getDownloadManager().getDownloadingCount() > 0 ? Page::Downloading : Page::NoDownloading);
            m_ui->listDownloading->setCurrentItem(nullptr);
        }
        else if(id == "queued")
        {
            m_ui->viewStack->setCurrentIndex(m_controller->getDownloadManager().getQueuedCount() > 0 ? Page::Queued : Page::NoQueued);
            m_ui->listQueued->setCurrentItem(nullptr);
        }
        else if(id == "completed")
        {
            m_ui->viewStack->setCurrentIndex(m_controller->getDownloadManager().getCompletedCount() > 0 ? Page::Completed : Page::NoCompleted);
            m_ui->listCompleted->setCurrentItem(nullptr);
        }
        else if(id == "settings")
        {
            m_ui->viewStack->setCurrentIndex(Page::Settings);
        }
    }

    void MainWindow::checkForUpdates()
    {
        m_controller->checkForUpdates();
    }

#ifdef _WIN32
    void MainWindow::windowsUpdate()
    {
        m_controller->windowsUpdate();
    }
#endif

    void MainWindow::documentation()
    {
        QDesktopServices::openUrl(QString::fromStdString(m_controller->getHelpUrl()));
    }

    void MainWindow::gitHubRepo()
    {
        QDesktopServices::openUrl(QString::fromStdString(m_controller->getAppInfo().getSourceRepo()));
    }

    void MainWindow::reportABug()
    {
        QDesktopServices::openUrl(QString::fromStdString(m_controller->getAppInfo().getIssueTracker()));
    }

    void MainWindow::discussions()
    {
        QDesktopServices::openUrl(QString::fromStdString(m_controller->getAppInfo().getSupportUrl()));
    }

    void MainWindow::about()
    {
        std::string extraDebug;
        extraDebug += "Qt " + std::string(qVersion()) + "\n";
        AboutDialog dialog{ m_controller->getAppInfo(), m_controller->getDebugInformation(extraDebug), this };
        dialog.exec();
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
            for(int i = 0; i < m_ui->listQueued->count(); i++)
            {
                if(m_ui->listQueued->itemWidget(m_ui->listQueued->item(i)) == m_downloadRows[id])
                {
                    delete m_ui->listQueued->takeItem(i);
                    m_downloadRows.erase(id);
                    break;
                }
            }
        }
        m_navigationBar->selectItem("queued");
    }

    void MainWindow::retryFailedDownloads()
    {
        m_controller->getDownloadManager().retryFailedDownloads();
    }


    void MainWindow::clearCompletedDownloads()
    {
        for(int id : m_controller->getDownloadManager().clearCompletedDownloads())
        {
            for(int i = 0; i < m_ui->listCompleted->count(); i++)
            {
                if(m_ui->listCompleted->itemWidget(m_ui->listCompleted->item(i)) == m_downloadRows[id])
                {
                    delete m_ui->listCompleted->takeItem(i);
                    m_downloadRows.erase(id);
                    break;
                }
            }
        }
        m_navigationBar->selectItem("completed");
    }

    void MainWindow::onDownloadListSelectionChanged()
    {
        QListWidget* list{ qobject_cast<QListWidget*>(sender()) };
        if(!list)
        {
            return;
        }
        QListWidgetItem* item{ list->currentItem() };
        if(!item)
        {
            m_ui->lblLog->setText("");
            m_ui->dockLog->hide();
            return;
        }
        DownloadRow* row{ static_cast<DownloadRow*>(list->itemWidget(item)) };
        m_ui->dockLog->show();
        m_ui->lblLog->setText(QString::fromStdString(m_controller->getDownloadManager().getDownloadCommand(row->getId())) + "\n" + QString::fromStdString(m_controller->getDownloadManager().getDownloadLog(row->getId())));
    }

    void MainWindow::onDockLogClosed(QObject* obj)
    {
        if(obj != m_ui->dockLog)
        {
            return;
        }
        m_ui->listDownloading->setCurrentItem(nullptr);
        m_ui->listQueued->setCurrentItem(nullptr);
        m_ui->listCompleted->setCurrentItem(nullptr);
    }

    void MainWindow::addDownload(const std::string& url)
    {
        AddDownloadDialog dialog{ m_controller->createAddDownloadDialogController(), url, this };
        dialog.exec();
    }

    void MainWindow::onNotificationSent(const NotificationSentEventArgs& args)
    {
        QMessageBox::Icon icon{ QMessageBox::Icon::NoIcon };
        switch(args.getSeverity())
        {
        case NotificationSeverity::Informational:
        case NotificationSeverity::Success:
            icon = QMessageBox::Icon::Information;
            break;
        case NotificationSeverity::Warning:
            icon = QMessageBox::Icon::Warning;
            break;
        case NotificationSeverity::Error:
            icon = QMessageBox::Icon::Critical;
            break;
        }
        QMessageBox msgBox{ icon, QString::fromStdString(m_controller->getAppInfo().getShortName()), QString::fromStdString(args.getMessage()), QMessageBox::StandardButton::Ok, this };
#ifdef _WIN32
        if(args.getAction() == "update")
        {
            QPushButton* updateButton{ msgBox.addButton(_("Update"), QMessageBox::ButtonRole::ActionRole) };
            connect(updateButton, &QPushButton::clicked, this, &MainWindow::windowsUpdate);
        }
#endif
        msgBox.exec();
    }

    void MainWindow::onShellNotificationSent(const ShellNotificationSentEventArgs& args)
    {
#ifdef _WIN32
        ShellNotification::send(args, reinterpret_cast<HWND>(winId()));
#elif defined(__linux__)
        ShellNotification::send(args, m_controller->getAppInfo().getId(), _("Open"));
#else
        ShellNotification::send(args);
#endif
    }

    void MainWindow::onHistoryChanged(const ParamEventArgs<std::vector<HistoricDownload>>& args)
    {
        m_ui->viewStackHistory->setCurrentIndex(args.getParam().empty() ? 0 : 1);
        m_ui->listHistory->clear();
        for(const HistoricDownload& download : args.getParam())
        {
            HistoryRow* row{ new HistoryRow(download) };
            connect(row, &HistoryRow::downloadAgain, [this](const std::string& url) { addDownload(url); });
            connect(row, &HistoryRow::deleteItem, [this](const HistoricDownload& download) { m_controller->getDownloadManager().removeHistoricDownload(download); });
            QListWidgetItem* item{ new QListWidgetItem() };
            item->setSizeHint(row->sizeHint() + QSize(0, 10));
            m_ui->listHistory->insertItem(0, item);
            m_ui->listHistory->setItemWidget(item, row);
        }
    }

    void MainWindow::onDownloadCredentialNeeded(const DownloadCredentialNeededEventArgs& args)
    {
        CredentialDialog dialog{ m_controller->createCredentialDialogController(args), this };
        dialog.exec();
    }

    void MainWindow::onDownloadAdded(const DownloadAddedEventArgs& args)
    {
        m_navigationBar->selectItem("downloading");
        DownloadRow* row{ new DownloadRow(args) };
        connect(row, &DownloadRow::stop, [this, args]() { m_controller->getDownloadManager().stopDownload(args.getId()); });
        connect(row, &DownloadRow::retry, [this, args]() { m_controller->getDownloadManager().retryDownload(args.getId()); });
        m_downloadRows[args.getId()] = row;
        QListWidgetItem* item{ new QListWidgetItem() };
        item->setSizeHint(row->sizeHint() + QSize(0, 10));
        m_ui->listDownloading->insertItem(0, item);
        m_ui->listDownloading->setItemWidget(item, row);
    }

    void MainWindow::onDownloadCompleted(const DownloadCompletedEventArgs& args)
    {
        m_downloadRows[args.getId()]->setCompleteState(args);
        moveDownloadRow(args.getId(), m_ui->listDownloading, m_ui->listCompleted);
    }

    void MainWindow::onDownloadProgressChanged(const DownloadProgressChangedEventArgs& args)
    {
        m_downloadRows[args.getId()]->setProgressState(args);
        Q_EMIT m_ui->listDownloading->itemSelectionChanged();
    }

    void MainWindow::onDownloadStopped(const ParamEventArgs<int>& args)
    {
        m_downloadRows[args.getParam()]->setStopState();
        moveDownloadRow(args.getParam(), m_ui->listDownloading, m_ui->listCompleted);
    }

    void MainWindow::onDownloadRetried(const ParamEventArgs<int>& args)
    {
        for(int i = 0; i < m_ui->listCompleted->count(); i++)
        {
            if(m_ui->listCompleted->itemWidget(m_ui->listCompleted->item(i)) == m_downloadRows[args.getParam()])
            {
                delete m_ui->listCompleted->takeItem(i);
                m_downloadRows.erase(args.getParam());
                break;
            }
        }
    }

    void MainWindow::onDownloadStartedFromQueue(const ParamEventArgs<int>& args)
    {
        m_downloadRows[args.getParam()]->setStartFromQueueState();
        moveDownloadRow(args.getParam(), m_ui->listQueued, m_ui->listDownloading);
    }

    void MainWindow::moveDownloadRow(int id, QListWidget* from, QListWidget* to)
    {
        for(int i = 0; i < from->count(); i++)
        {
            if(from->itemWidget(from->item(i)) == m_downloadRows[id])
            {
                QListWidgetItem* item{ from->takeItem(i) };
                DownloadRow* clone{ new DownloadRow(*m_downloadRows[id]) };
                connect(clone, &DownloadRow::stop, [this, id]() { m_controller->getDownloadManager().stopDownload(id); });
                connect(clone, &DownloadRow::retry, [this, id]() { m_controller->getDownloadManager().retryDownload(id); });
                delete m_downloadRows[id];
                m_downloadRows[id] = clone;
                to->insertItem(0, item);
                to->setItemWidget(item, clone);
                break;
            }
        }
        m_navigationBar->selectItem(m_navigationBar->getSelectedItem());
    }
}
