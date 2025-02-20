#include "application.h"
#include <stdexcept>
#include <libnick/system/environment.h>

using namespace Nickvision::System;
using namespace Nickvision::TubeConverter::Shared::Controllers;
using namespace Nickvision::TubeConverter::Shared::Models;

namespace Nickvision::TubeConverter::GNOME
{
    Application::Application(int argc, char* argv[])
        : m_argc{ argc },
        m_argv{ argv }, 
        m_controller{ std::make_shared<MainWindowController>(std::vector<std::string>(argv, argv + argc)) },
        m_adw{ adw_application_new(m_controller->getAppInfo().getId().c_str(), G_APPLICATION_HANDLES_OPEN) },
        m_mainWindow{ nullptr }
    {
        std::filesystem::path resources{ Environment::getExecutableDirectory() / (m_controller->getAppInfo().getId() + ".gresource") };
        GError* resourceLoadError{ nullptr };
        GResource* resource{ g_resource_load(resources.string().c_str(), &resourceLoadError) };
        if(resourceLoadError)
        {
            throw std::runtime_error(resourceLoadError->message);
        }
        g_resources_register(resource);
        g_signal_connect(m_adw, "startup", G_CALLBACK(+[](GtkApplication* app, gpointer data){ reinterpret_cast<Application*>(data)->onStartup(app); }), this);
        g_signal_connect(m_adw, "activate", G_CALLBACK(+[](GtkApplication* app, gpointer data){ reinterpret_cast<Application*>(data)->onActivate(app); }), this);
        g_signal_connect(m_adw, "open", G_CALLBACK(+[](GtkApplication* app, void* files, int n, const char* hint, gpointer data){ reinterpret_cast<Application*>(data)->onOpen(app, files, n, hint); }), this);
    }

    int Application::run()
    {
        return g_application_run(G_APPLICATION(m_adw), m_argc, m_argv);
    }

    void Application::onStartup(GtkApplication* app)
    {
        switch (m_controller->getTheme())
        {
        case Theme::Light:
            adw_style_manager_set_color_scheme(adw_style_manager_get_default(), ADW_COLOR_SCHEME_FORCE_LIGHT);
            break;
        case Theme::Dark:
            adw_style_manager_set_color_scheme(adw_style_manager_get_default(), ADW_COLOR_SCHEME_FORCE_DARK);
            break;
        default:
            adw_style_manager_set_color_scheme(adw_style_manager_get_default(), ADW_COLOR_SCHEME_DEFAULT);
            break;
        }
        if(!m_mainWindow)
        {
            m_mainWindow = std::make_shared<Views::MainWindow>(m_controller, app);
        }
    }

    void Application::onActivate(GtkApplication* app)
    {
        m_mainWindow->show();
    }

    void Application::onOpen(GtkApplication* app, void* files, int n, const char* hint)
    {
        GFile** arr{ reinterpret_cast<GFile**>(files) };
        m_mainWindow->show();
        m_mainWindow->addDownload(g_file_get_uri(arr[0]));
    }
}
