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
        DownloaderOptions options{ m_controller->getDownloaderOptions() };
        adw_combo_row_set_selected(m_builder.get<AdwComboRow>("themeRow"), static_cast<unsigned int>(m_controller->getTheme()));
        adw_combo_row_set_selected(m_builder.get<AdwComboRow>("completedNotificationTriggerRow"), static_cast<unsigned int>(m_controller->getCompletedNotificationPreference()));
        adw_switch_row_set_active(m_builder.get<AdwSwitchRow>("preventSuspendRow"), m_controller->getPreventSuspend());
        adw_combo_row_set_selected(m_builder.get<AdwComboRow>("historyLengthRow"), static_cast<unsigned int>(m_controller->getHistoryLengthIndex()));
        adw_switch_row_set_active(m_builder.get<AdwSwitchRow>("overwriteExistingFilesRow"), options.getOverwriteExistingFiles());
        adw_spin_row_set_value(m_builder.get<AdwSpinRow>("maxNumberOfActiveDownloadsRow"), static_cast<double>(options.getMaxNumberOfActiveDownloads()));
        adw_switch_row_set_active(m_builder.get<AdwSwitchRow>("limitCharactersRow"), options.getLimitCharacters());
        adw_switch_row_set_active(m_builder.get<AdwSwitchRow>("includeAutoGeneratedSubtitlesRow"), options.getIncludeAutoGeneratedSubtitles());
        adw_spin_row_set_value(m_builder.get<AdwSpinRow>("speedLimitRow"), static_cast<double>(options.getSpeedLimit()));
        adw_switch_row_set_active(m_builder.get<AdwSwitchRow>("sponsorBlockRow"), options.getYouTubeSponsorBlock());
        gtk_editable_set_text(m_builder.get<GtkEditable>("proxyUrlRow"), options.getProxyUrl().c_str());
        adw_combo_row_set_selected(m_builder.get<AdwComboRow>("cookiesBrowserRow"), static_cast<unsigned int>(options.getCookiesBrowser()));
        adw_switch_row_set_active(m_builder.get<AdwSwitchRow>("embedMetadataRow"), options.getEmbedMetadata());
        adw_switch_row_set_active(m_builder.get<AdwSwitchRow>("embedChaptersRow"), options.getEmbedChapters());
        adw_switch_row_set_active(m_builder.get<AdwSwitchRow>("embedSubtitlesRow"), options.getEmbedSubtitles());
        adw_switch_row_set_active(m_builder.get<AdwSwitchRow>("cropAudioThumbnailRow"), options.getCropAudioThumbnails());
        adw_switch_row_set_active(m_builder.get<AdwSwitchRow>("removeSourceDataRow"), options.getRemoveSourceData());
        adw_switch_row_set_active(m_builder.get<AdwSwitchRow>("useAriaRow"), options.getUseAria());
        adw_spin_row_set_value(m_builder.get<AdwSpinRow>("ariaMaxConnectionsPerServerRow"), static_cast<double>(options.getAriaMaxConnectionsPerServer()));
        adw_spin_row_set_value(m_builder.get<AdwSpinRow>("ariaMinSplitSizeRow"), static_cast<double>(options.getAriaMinSplitSize()));
        //Signals
        m_closed += [&](const EventArgs&) { onClosed(); };
        g_signal_connect(m_builder.get<GObject>("themeRow"), "notify::selected-item", G_CALLBACK(+[](GObject*, GParamSpec* pspec, gpointer data){ reinterpret_cast<PreferencesDialog*>(data)->onThemeChanged(); }), this);
    }

    void PreferencesDialog::onClosed()
    {
        DownloaderOptions options{ m_controller->getDownloaderOptions() };
        m_controller->setCompletedNotificationPreference(static_cast<CompletedNotificationPreference>(adw_combo_row_get_selected(m_builder.get<AdwComboRow>("completedNotificationTriggerRow"))));
        m_controller->setPreventSuspend(adw_switch_row_get_active(m_builder.get<AdwSwitchRow>("preventSuspendRow")));
        m_controller->setHistoryLengthIndex(static_cast<size_t>(adw_combo_row_get_selected(m_builder.get<AdwComboRow>("historyLengthRow"))));
        options.setOverwriteExistingFiles(adw_switch_row_get_active(m_builder.get<AdwSwitchRow>("overwriteExistingFilesRow")));
        options.setMaxNumberOfActiveDownloads(static_cast<int>(adw_spin_row_get_value(m_builder.get<AdwSpinRow>("maxNumberOfActiveDownloadsRow"))));
        options.setLimitCharacters(adw_switch_row_get_active(m_builder.get<AdwSwitchRow>("limitCharactersRow")));
        options.setIncludeAutoGeneratedSubtitles(adw_switch_row_get_active(m_builder.get<AdwSwitchRow>("includeAutoGeneratedSubtitlesRow")));
        options.setSpeedLimit(static_cast<int>(adw_spin_row_get_value(m_builder.get<AdwSpinRow>("speedLimitRow"))));
        options.setYouTubeSponsorBlock(adw_switch_row_get_active(m_builder.get<AdwSwitchRow>("sponsorBlockRow")));
        options.setProxyUrl(gtk_editable_get_text(m_builder.get<GtkEditable>("proxyUrlRow")));
        options.setCookiesBrowser(static_cast<Browser>(adw_combo_row_get_selected(m_builder.get<AdwComboRow>("cookiesBrowserRow"))));
        options.setEmbedMetadata(adw_switch_row_get_active(m_builder.get<AdwSwitchRow>("embedMetadataRow")));
        options.setEmbedChapters(adw_switch_row_get_active(m_builder.get<AdwSwitchRow>("embedChaptersRow")));
        options.setEmbedSubtitles(adw_switch_row_get_active(m_builder.get<AdwSwitchRow>("embedSubtitlesRow")));
        options.setCropAudioThumbnails(adw_switch_row_get_active(m_builder.get<AdwSwitchRow>("cropAudioThumbnailRow")));
        options.setRemoveSourceData(adw_switch_row_get_active(m_builder.get<AdwSwitchRow>("removeSourceDataRow")));
        options.setUseAria(adw_switch_row_get_active(m_builder.get<AdwSwitchRow>("useAriaRow")));
        options.setAriaMaxConnectionsPerServer(static_cast<int>(adw_spin_row_get_value(m_builder.get<AdwSpinRow>("ariaMaxConnectionsPerServerRow"))));
        options.setAriaMinSplitSize(static_cast<int>(adw_spin_row_get_value(m_builder.get<AdwSpinRow>("ariaMinSplitSizeRow"))));
        m_controller->setDownloaderOptions(options);
        m_controller->saveConfiguration();
    }

    void PreferencesDialog::onThemeChanged()
    {
        m_controller->setTheme(static_cast<Theme>(adw_combo_row_get_selected(m_builder.get<AdwComboRow>("themeRow"))));
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
}
