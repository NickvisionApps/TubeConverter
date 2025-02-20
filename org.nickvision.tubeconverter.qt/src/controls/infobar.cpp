#include "controls/infobar.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <libnick/localization/gettext.h>
#include <oclero/qlementine/widgets/StatusBadgeWidget.hpp>
#include "helpers/qthelpers.h"

using namespace Nickvision::Notifications;
using namespace oclero::qlementine;

namespace Ui
{
    class InfoBar
    {
    public:
        void setupUi(Nickvision::TubeConverter::Qt::Controls::InfoBar* parent)
        {
            QFont boldFont;
            boldFont.setBold(true);
            //Main Widgets
            sbwIcon = new StatusBadgeWidget(parent);
            lblMessage = new QLabel(parent);
            lblMessage->setFont(boldFont);
            btnAction = new QPushButton(parent);
            //Main Layout
            QWidget* mainWidget{ new QWidget(parent) };
            QHBoxLayout* layout{ new QHBoxLayout(parent) };
            layout->addWidget(sbwIcon);
            layout->addWidget(lblMessage);
            layout->addStretch();
            layout->addWidget(btnAction);
            mainWidget->setLayout(layout);
            parent->setWidget(mainWidget);
        }

        StatusBadgeWidget* sbwIcon;
        QLabel* lblMessage;
        QPushButton* btnAction;
    };
}

namespace Nickvision::TubeConverter::Qt::Controls
{
    InfoBar::InfoBar(QWidget* parent)
        : QDockWidget{ parent },
        m_ui{ new Ui::InfoBar() }
    {
        //Window Settings
        setWindowTitle(_("Notification"));
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
        setFeatures(QDockWidget::DockWidgetFeature::DockWidgetClosable);
        setAllowedAreas(::Qt::DockWidgetArea::BottomDockWidgetArea | ::Qt::DockWidgetArea::TopDockWidgetArea);
        hide();
        //Load Ui
        m_ui->setupUi(this);
        //Signals
        connect(m_ui->btnAction, &QPushButton::clicked, this, &InfoBar::action);
    }

    InfoBar::~InfoBar()
    {
        delete m_ui;
    }

    void InfoBar::action()
    {
        hide();
        m_callback();
    }

    void InfoBar::show(const NotificationSentEventArgs& args, const QString& actionButtonText, const std::function<void()>& callback)
    {
        QDockWidget::show();
        if(!actionButtonText.isEmpty() && callback)
        {
            m_ui->btnAction->setText(actionButtonText);
            m_ui->btnAction->setVisible(true);
            m_callback = callback;
        }
        else
        {
            m_ui->btnAction->setVisible(false);
            m_callback = {};
        }
        m_ui->lblMessage->setText(QString::fromStdString(args.getMessage()));
        switch(args.getSeverity())
        {
        case NotificationSeverity::Informational:
            m_ui->sbwIcon->setBadge(StatusBadge::Info);
            break;
        case NotificationSeverity::Success:
            m_ui->sbwIcon->setBadge(StatusBadge::Success);
            break;
        case NotificationSeverity::Warning:
            m_ui->sbwIcon->setBadge(StatusBadge::Warning);
            break;
        case NotificationSeverity::Error:
            m_ui->sbwIcon->setBadge(StatusBadge::Error);
            break;
        }
    }
}