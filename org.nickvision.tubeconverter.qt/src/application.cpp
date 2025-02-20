#include "application.h"
#include <QStyleHints>
#include <oclero/qlementine/icons/QlementineIcons.hpp>

using namespace Nickvision::TubeConverter::Shared::Controllers;
using namespace Nickvision::TubeConverter::Shared::Models;

namespace Nickvision::TubeConverter::Qt
{
    Application::Application(int argc, char* argv[])
        : QApplication{ argc, argv },
        m_controller{ std::make_shared<MainWindowController>(std::vector<std::string>(argv, argv + argc)) },
        m_mainWindow{ nullptr },
        m_style{ new oclero::qlementine::QlementineStyle(this) },
        m_themeManager{ new oclero::qlementine::ThemeManager(m_style) }
    {
        //Style
        m_style->setAnimationsEnabled(true);
        m_style->setAutoIconColor(oclero::qlementine::AutoIconColor::TextColor);
        QApplication::setStyle(m_style);
        //Icons
        oclero::qlementine::icons::initializeIconTheme();
        QIcon::setThemeName("qlementine");
        //Themes
        m_themeManager->loadDirectory(":/");
    }

    int Application::exec()
    {
        switch (m_controller->getTheme())
        {
        case Theme::Light:
            QApplication::styleHints()->setColorScheme(::Qt::ColorScheme::Light);
            m_themeManager->setCurrentTheme("Light");
            break;
        case Theme::Dark:
            QApplication::styleHints()->setColorScheme(::Qt::ColorScheme::Dark);
            m_themeManager->setCurrentTheme("Dark");
            break;
        default:
            QApplication::styleHints()->unsetColorScheme();
            m_themeManager->setCurrentTheme(QApplication::styleHints()->colorScheme() == ::Qt::ColorScheme::Light ? "Light" : "Dark");
            break;
        }
        m_mainWindow = std::make_shared<Views::MainWindow>(m_controller, m_themeManager);
        m_mainWindow->show();
        return QApplication::exec();
    }
}
