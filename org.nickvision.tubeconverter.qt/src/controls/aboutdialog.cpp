#include "controls/aboutdialog.h"
#include <format>
#include <QApplication>
#include <QClipboard>
#include <QGroupBox>
#include <QLabel>
#include <QPixmap>
#include <QPushButton>
#include <QScrollArea>
#include <QSpacerItem>
#include <QTabWidget>
#include <QVBoxLayout>
#include <libnick/helpers/codehelpers.h>
#include <libnick/helpers/stringhelpers.h>
#include <libnick/localization/gettext.h>
#include "helpers/qthelpers.h"

using namespace Nickvision::App;
using namespace Nickvision::Helpers;
using namespace Nickvision::Update;

namespace Ui
{
    class AboutDialog
    {
    public:
        void setupUi(Nickvision::TubeConverter::Qt::Controls::AboutDialog* parent)
        {
            QFont boldFont;
            boldFont.setBold(true);
            //About Tab
            lblAppIcon = new QLabel(parent);
            lblAppIcon->setAlignment(Qt::AlignmentFlag::AlignCenter);
            lblAppName = new QLabel(parent);
            lblAppName->setFont(boldFont);
            lblAppName->setAlignment(Qt::AlignmentFlag::AlignCenter);
            lblAppDescription = new QLabel(parent);
            lblAppDescription->setAlignment(Qt::AlignmentFlag::AlignCenter);
            lblAppVersion = new QLabel(parent);
            lblAppVersion->setAlignment(Qt::AlignmentFlag::AlignCenter);
            QWidget* tabAbout{ new QWidget(parent) };
            QVBoxLayout* layoutAbout{ new QVBoxLayout(parent) };
            layoutAbout->setSpacing(12);
            layoutAbout->addStretch();
            layoutAbout->addWidget(lblAppIcon);
            layoutAbout->addWidget(lblAppName);
            layoutAbout->addWidget(lblAppDescription);
            layoutAbout->addWidget(lblAppVersion);
            layoutAbout->addStretch();
            tabAbout->setLayout(layoutAbout);
            //Changelog Tab
            lblChangelog = new QLabel(parent);
            lblChangelog->setAlignment(Qt::AlignmentFlag::AlignLeading | Qt::AlignmentFlag::AlignLeft | Qt::AlignmentFlag::AlignTop);
            lblChangelog->setWordWrap(true);
            lblChangelog->setTextInteractionFlags(Qt::TextInteractionFlag::LinksAccessibleByMouse | Qt::TextInteractionFlag::TextSelectableByMouse);
            QWidget* tabChangelog{ new QWidget(parent) };
            QVBoxLayout* layoutChangelog{ new QVBoxLayout(parent) };
            layoutChangelog->addWidget(lblChangelog);
            tabChangelog->setLayout(layoutChangelog);
            //Credits Tab
            lblDevelopers = new QLabel(parent);
            lblDevelopers->setAlignment(Qt::AlignmentFlag::AlignLeading | Qt::AlignmentFlag::AlignLeft | Qt::AlignmentFlag::AlignTop);
            lblDesigners = new QLabel(parent);
            lblDesigners->setAlignment(Qt::AlignmentFlag::AlignLeading | Qt::AlignmentFlag::AlignLeft | Qt::AlignmentFlag::AlignTop);
            lblArtists = new QLabel(parent);
            lblArtists->setAlignment(Qt::AlignmentFlag::AlignLeading | Qt::AlignmentFlag::AlignLeft | Qt::AlignmentFlag::AlignTop);
            QGroupBox* groupDevelopers{ new QGroupBox(parent) };
            QVBoxLayout* layoutDevelopers{ new QVBoxLayout(parent) };
            groupDevelopers->setTitle(_("Developers"));
            layoutDevelopers->addWidget(lblDevelopers);
            groupDevelopers->setLayout(layoutDevelopers);
            QGroupBox* groupDesigners{ new QGroupBox(parent) };
            QVBoxLayout* layoutDesigners{ new QVBoxLayout(parent) };
            groupDesigners->setTitle(_("Designers"));
            layoutDesigners->addWidget(lblDesigners);
            groupDesigners->setLayout(layoutDesigners);
            QGroupBox* groupArtists{ new QGroupBox(parent) };
            QVBoxLayout* layoutArtists{ new QVBoxLayout(parent) };
            groupArtists->setTitle(_("Artists"));
            layoutArtists->addWidget(lblArtists);
            groupArtists->setLayout(layoutArtists);
            QWidget* tabCredits{ new QWidget(parent) };
            QVBoxLayout* layoutCredits{ new QVBoxLayout(parent) };
            layoutCredits->addWidget(groupDevelopers);
            layoutCredits->addWidget(groupDesigners);
            layoutCredits->addWidget(groupArtists);
            tabCredits->setLayout(layoutCredits);
            //Debug Tab
            btnCopyDebugInformation = new QPushButton(parent);
            btnCopyDebugInformation->setText(_("Copy Debug Information"));
            btnCopyDebugInformation->setIcon(QLEMENTINE_ICON(Action_Copy));
            btnCopyDebugInformation->setAutoDefault(false);
            lblDebug = new QLabel(parent);
            lblDebug->setAlignment(Qt::AlignmentFlag::AlignLeading | Qt::AlignmentFlag::AlignLeft | Qt::AlignmentFlag::AlignTop);
            lblDebug->setWordWrap(true);
            lblDebug->setTextInteractionFlags(Qt::TextInteractionFlag::LinksAccessibleByMouse | Qt::TextInteractionFlag::TextSelectableByMouse);
            QWidget* tabDebug{ new QWidget(parent) };
            QScrollArea* scrollDebug{ new QScrollArea(parent) };
            scrollDebug->setWidgetResizable(true);
            scrollDebug->setWidget(lblDebug);
            QVBoxLayout* layoutDebug{ new QVBoxLayout(parent) };
            layoutDebug->addWidget(btnCopyDebugInformation);
            layoutDebug->addWidget(scrollDebug);
            tabDebug->setLayout(layoutDebug);
            //Main Layout
            QVBoxLayout* layoutMain{ new QVBoxLayout(parent) };
            QTabWidget* tabWidget{ new QTabWidget(parent) };
            tabWidget->addTab(tabAbout, QLEMENTINE_ICON(Misc_Info), _("About"));
            tabWidget->addTab(tabChangelog, QLEMENTINE_ICON(Misc_ItemsList), _("Changelog"));
            tabWidget->addTab(tabCredits, QLEMENTINE_ICON(Misc_Users), _("Credits"));
            tabWidget->addTab(tabDebug, QLEMENTINE_ICON(Misc_Debug), _("Debugging"));
            tabWidget->setCurrentIndex(0);
            layoutMain->addWidget(tabWidget);
            parent->setLayout(layoutMain);
        }

        QLabel* lblAppIcon;
        QLabel* lblAppName;
        QLabel* lblAppDescription;
        QLabel* lblAppVersion;
        QLabel* lblChangelog;
        QLabel* lblDevelopers;
        QLabel* lblDesigners;
        QLabel* lblArtists;
        QPushButton* btnCopyDebugInformation;
        QLabel* lblDebug;
    };
}

namespace Nickvision::TubeConverter::Qt::Controls
{
    static std::vector<std::string> keys(const std::unordered_map<std::string, std::string>& m)
    {
        std::vector<std::string> k;
        for(const std::pair<const std::string, std::string>& pair : m)
        {
            k.push_back(pair.first);
        }
        return k;
    }

    AboutDialog::AboutDialog(const AppInfo& appInfo, const std::string& debugInformation, QWidget* parent)
        : QDialog{ parent },
        m_ui{ new Ui::AboutDialog() },
        m_debugInformation{ QString::fromStdString(debugInformation) }
    {
        //Dialog Settings
        setWindowTitle(_("About Parabolic"));
        setMinimumSize(420, 410);
        setModal(true);
        //Load Ui
        m_ui->setupUi(this);
        m_ui->lblAppIcon->setPixmap({ appInfo.getVersion().getVersionType() == VersionType::Stable ? ":/icon.svg" : ":/icon-devel.svg" });
        m_ui->lblAppName->setText(QString::fromStdString(appInfo.getShortName()));
        m_ui->lblAppDescription->setText(QString::fromStdString(appInfo.getDescription()));
        m_ui->lblAppVersion->setText(QString::fromStdString(appInfo.getVersion().str()));
        m_ui->lblChangelog->setText(QString::fromStdString(appInfo.getChangelog()));
        m_ui->lblDevelopers->setText(QString::fromStdString(StringHelpers::join(keys(appInfo.getDevelopers()), "\n")));
        m_ui->lblDesigners->setText(QString::fromStdString(StringHelpers::join(keys(appInfo.getDesigners()), "\n")));
        m_ui->lblArtists->setText(QString::fromStdString(StringHelpers::join(keys(appInfo.getArtists()), "\n")));
        m_ui->btnCopyDebugInformation->setText(QString::fromStdString(_("Copy Debug Information")));
        m_ui->lblDebug->setText(m_debugInformation);
        //Signals
        connect(m_ui->btnCopyDebugInformation, &QPushButton::clicked, this, &AboutDialog::copyDebugInformation);
    }

    AboutDialog::~AboutDialog()
    {
        delete m_ui;
    }

    void AboutDialog::copyDebugInformation()
    {
        QApplication::clipboard()->setText(m_debugInformation);
    }
}