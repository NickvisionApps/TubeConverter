#include "views/preferencesdialog.h"
#include <libnick/localization/gettext.h>

using namespace Nickvision::Events;
using namespace Nickvision::TubeConverter::Shared::Controllers;
using namespace Nickvision::TubeConverter::Shared::Models;

namespace Nickvision::TubeConverter::GNOME::Views
{
    PreferencesDialog::PreferencesDialog(const std::shared_ptr<PreferencesViewController>& controller, GtkWindow* parent)
        : DialogBase{ parent, "preferences_dialog" },
        m_controller{ controller }
    {
        //Load
        adw_combo_row_set_selected(ADW_COMBO_ROW(gtk_builder_get_object(m_builder, "themeRow")), static_cast<unsigned int>(m_controller->getTheme()));
        adw_combo_row_set_selected(ADW_COMBO_ROW(gtk_builder_get_object(m_builder, "completedNotificationTriggerRow")), static_cast<unsigned int>(m_controller->getCompletedNotificationPreference()));
        adw_switch_row_set_active(ADW_SWITCH_ROW(gtk_builder_get_object(m_builder, "preventSuspendRow")), m_controller->getPreventSuspendWhenDownloading());
        adw_switch_row_set_active(ADW_SWITCH_ROW(gtk_builder_get_object(m_builder, "overwriteExistingFilesRow")), m_controller->getOverwriteExistingFiles());
        adw_spin_row_set_value(ADW_SPIN_ROW(gtk_builder_get_object(m_builder, "maxNumberOfActiveDownloadsRow")), static_cast<double>(m_controller->getMaxNumberOfActiveDownloads()));
        adw_switch_row_set_active(ADW_SWITCH_ROW(gtk_builder_get_object(m_builder, "limitCharactersRow")), m_controller->getLimitCharacters());
        adw_switch_row_set_active(ADW_SWITCH_ROW(gtk_builder_get_object(m_builder, "includeAutoGeneratedSubtitlesRow")), m_controller->getIncludeAutoGeneratedSubtitles());
        adw_spin_row_set_value(ADW_SPIN_ROW(gtk_builder_get_object(m_builder, "speedLimitRow")), static_cast<double>(m_controller->getSpeedLimit()));
        adw_switch_row_set_active(ADW_SWITCH_ROW(gtk_builder_get_object(m_builder, "sponsorBlockRow")), m_controller->getYouTubeSponsorBlock());
        gtk_editable_set_text(GTK_EDITABLE(gtk_builder_get_object(m_builder, "proxyUrlRow")), m_controller->getProxyUrl().c_str());
        adw_action_row_set_subtitle(ADW_ACTION_ROW(gtk_builder_get_object(m_builder, "cookiesFileRow")), m_controller->getCookiesPath().empty() ? _("No file selected") : m_controller->getCookiesPath().string().c_str());
        adw_switch_row_set_active(ADW_SWITCH_ROW(gtk_builder_get_object(m_builder, "disallowConversionsRow")), m_controller->getDisallowConversions());
        adw_switch_row_set_active(ADW_SWITCH_ROW(gtk_builder_get_object(m_builder, "embedMetadataRow")), m_controller->getEmbedMetadata());
        adw_switch_row_set_active(ADW_SWITCH_ROW(gtk_builder_get_object(m_builder, "embedChaptersRow")), m_controller->getEmbedChapters());
        adw_switch_row_set_active(ADW_SWITCH_ROW(gtk_builder_get_object(m_builder, "embedSubtitlesRow")), m_controller->getEmbedSubtitles());
        adw_switch_row_set_active(ADW_SWITCH_ROW(gtk_builder_get_object(m_builder, "cropAudioThumbnailRow")), m_controller->getCropAudioThumbnails());
        adw_switch_row_set_active(ADW_SWITCH_ROW(gtk_builder_get_object(m_builder, "removeSourceDataRow")), m_controller->getRemoveSourceData());
        adw_switch_row_set_active(ADW_SWITCH_ROW(gtk_builder_get_object(m_builder, "useAriaRow")), m_controller->getUseAria());
        adw_spin_row_set_value(ADW_SPIN_ROW(gtk_builder_get_object(m_builder, "ariaMaxConnectionsPerServerRow")), static_cast<double>(m_controller->getAriaMaxConnectionsPerServer()));
        adw_spin_row_set_value(ADW_SPIN_ROW(gtk_builder_get_object(m_builder, "ariaMinSplitSizeRow")), static_cast<double>(m_controller->getAriaMinSplitSize()));
        //Signals
        m_closed += [&](const EventArgs&) { onClosed(); };
        g_signal_connect(gtk_builder_get_object(m_builder, "themeRow"), "notify::selected-item", G_CALLBACK(+[](GObject*, GParamSpec* pspec, gpointer data){ reinterpret_cast<PreferencesDialog*>(data)->onThemeChanged(); }), this);
        g_signal_connect(gtk_builder_get_object(m_builder, "selectCookiesFileButton"), "clicked", G_CALLBACK(+[](GtkButton*, gpointer data){ reinterpret_cast<PreferencesDialog*>(data)->selectCookiesFile(); }), this);
        g_signal_connect(gtk_builder_get_object(m_builder, "clearCookiesFileButton"), "clicked", G_CALLBACK(+[](GtkButton*, gpointer data){ reinterpret_cast<PreferencesDialog*>(data)->clearCookiesFile(); }), this);
        g_signal_connect(gtk_builder_get_object(m_builder, "chromeCookiesButton"), "clicked", G_CALLBACK(+[](GtkButton*, gpointer data){ reinterpret_cast<PreferencesDialog*>(data)->cookiesExtension(Browsers::Chrome); }), this);
        g_signal_connect(gtk_builder_get_object(m_builder, "firefoxCookiesButton"), "clicked", G_CALLBACK(+[](GtkButton*, gpointer data){ reinterpret_cast<PreferencesDialog*>(data)->cookiesExtension(Browsers::Firefox); }), this);
    }

    void PreferencesDialog::onClosed()
    {
        m_controller->setCompletedNotificationPreference(static_cast<CompletedNotificationPreference>(adw_combo_row_get_selected(ADW_COMBO_ROW(gtk_builder_get_object(m_builder, "completedNotificationTriggerRow")))));
        m_controller->setPreventSuspendWhenDownloading(adw_switch_row_get_active(ADW_SWITCH_ROW(gtk_builder_get_object(m_builder, "preventSuspendRow"))));
        m_controller->setOverwriteExistingFiles(adw_switch_row_get_active(ADW_SWITCH_ROW(gtk_builder_get_object(m_builder, "overwriteExistingFilesRow"))));
        m_controller->setMaxNumberOfActiveDownloads(static_cast<int>(adw_spin_row_get_value(ADW_SPIN_ROW(gtk_builder_get_object(m_builder, "maxNumberOfActiveDownloadsRow")))));
        m_controller->setLimitCharacters(adw_switch_row_get_active(ADW_SWITCH_ROW(gtk_builder_get_object(m_builder, "limitCharactersRow"))));
        m_controller->setIncludeAutoGeneratedSubtitles(adw_switch_row_get_active(ADW_SWITCH_ROW(gtk_builder_get_object(m_builder, "includeAutoGeneratedSubtitlesRow"))));
        m_controller->setSpeedLimit(static_cast<int>(adw_spin_row_get_value(ADW_SPIN_ROW(gtk_builder_get_object(m_builder, "speedLimitRow")))));
        m_controller->setYouTubeSponsorBlock(adw_switch_row_get_active(ADW_SWITCH_ROW(gtk_builder_get_object(m_builder, "sponsorBlockRow"))));
        m_controller->setProxyUrl(gtk_editable_get_text(GTK_EDITABLE(gtk_builder_get_object(m_builder, "proxyUrlRow"))));
        m_controller->setDisallowConversions(adw_switch_row_get_active(ADW_SWITCH_ROW(gtk_builder_get_object(m_builder, "disallowConversionsRow"))));
        m_controller->setEmbedMetadata(adw_switch_row_get_active(ADW_SWITCH_ROW(gtk_builder_get_object(m_builder, "embedMetadataRow"))));
        m_controller->setEmbedChapters(adw_switch_row_get_active(ADW_SWITCH_ROW(gtk_builder_get_object(m_builder, "embedChaptersRow"))));
        m_controller->setEmbedSubtitles(adw_switch_row_get_active(ADW_SWITCH_ROW(gtk_builder_get_object(m_builder, "embedSubtitlesRow"))));
        m_controller->setCropAudioThumbnails(adw_switch_row_get_active(ADW_SWITCH_ROW(gtk_builder_get_object(m_builder, "cropAudioThumbnailRow"))));
        m_controller->setRemoveSourceData(adw_switch_row_get_active(ADW_SWITCH_ROW(gtk_builder_get_object(m_builder, "removeSourceDataRow"))));
        m_controller->setUseAria(adw_switch_row_get_active(ADW_SWITCH_ROW(gtk_builder_get_object(m_builder, "useAriaRow"))));
        m_controller->setAriaMaxConnectionsPerServer(static_cast<int>(adw_spin_row_get_value(ADW_SPIN_ROW(gtk_builder_get_object(m_builder, "ariaMaxConnectionsPerServerRow")))));
        m_controller->setAriaMinSplitSize(static_cast<int>(adw_spin_row_get_value(ADW_SPIN_ROW(gtk_builder_get_object(m_builder, "ariaMinSplitSizeRow")))));
        m_controller->saveConfiguration();
    }

    void PreferencesDialog::onThemeChanged()
    {
        m_controller->setTheme(static_cast<Theme>(adw_combo_row_get_selected(ADW_COMBO_ROW(gtk_builder_get_object(m_builder, "themeRow")))));
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
    }

    void PreferencesDialog::selectCookiesFile()
    {
        GtkFileDialog* fileDialog{ gtk_file_dialog_new() };
        gtk_file_dialog_set_title(fileDialog, _("Select Cookies File"));
        GtkFileFilter* filter{ gtk_file_filter_new() };
        gtk_file_filter_set_name(filter, _("Text File (*.txt)"));
        gtk_file_filter_add_pattern(filter, "*.txt");
        gtk_file_filter_add_pattern(filter, "*.txt");
        GListStore* filters{ g_list_store_new(gtk_file_filter_get_type()) };
        g_list_store_append(filters, G_OBJECT(filter));
        gtk_file_dialog_set_filters(fileDialog, G_LIST_MODEL(filters));
        gtk_file_dialog_open(fileDialog, m_parent, nullptr, GAsyncReadyCallback(+[](GObject* self, GAsyncResult* res, gpointer data) 
        {
            GFile* file{ gtk_file_dialog_open_finish(GTK_FILE_DIALOG(self), res, nullptr) };
            if(file)
            {
                reinterpret_cast<PreferencesDialog*>(data)->m_controller->setCookiesPath(g_file_get_path(file));
                adw_action_row_set_subtitle(ADW_ACTION_ROW(gtk_builder_get_object(reinterpret_cast<PreferencesDialog*>(data)->m_builder, "cookiesFileRow")), g_file_get_path(file));
            }
        }), this);
    }

    void PreferencesDialog::clearCookiesFile()
    {
        m_controller->setCookiesPath("");
        adw_action_row_set_subtitle(ADW_ACTION_ROW(gtk_builder_get_object(m_builder, "cookiesFileRow")), _("No file selected"));
    }

    void PreferencesDialog::cookiesExtension(Browsers browser)
    {
        gtk_popover_popdown(GTK_POPOVER(gtk_builder_get_object(m_builder, "cookiesPopover")));
        GtkUriLauncher* launcher{ gtk_uri_launcher_new(m_controller->getCookiesExtensionUrl(browser).c_str()) };
        gtk_uri_launcher_launch(launcher, m_parent, nullptr, GAsyncReadyCallback(+[](GObject* source, GAsyncResult* res, gpointer)
        { 
            gtk_uri_launcher_launch_finish(GTK_URI_LAUNCHER(source), res, nullptr); 
            g_object_unref(source);
        }), nullptr);
    }
}
