#ifndef ADDDOWNLOADDIALOG_H
#define ADDDOWNLOADDIALOG_H

#include "includes.h"
#include <memory>
#include "Controls/ViewStack.g.h"
#include "Controls/ViewStackPage.g.h"
#include "AddDownloadDialog.g.h"
#include "controllers/adddownloaddialogcontroller.h"

namespace winrt::Nickvision::TubeConverter::WinUI::implementation 
{
    /**
     * @brief The settings page for the application. 
     */
    class AddDownloadDialog : public AddDownloadDialogT<AddDownloadDialog>
    {
    public:
        /**
         * @brief Constructs a AddDownloadDialog.
         */
        AddDownloadDialog();
        /**
         * @brief Sets the controller for the dialog.
         * @param controller The AddDownloadDialogController 
         * @param hwnd The window handle
         */
        void SetController(const std::shared_ptr<::Nickvision::TubeConverter::Shared::Controllers::AddDownloadDialogController>& controller, HWND hwnd);
        /**
         * @brief Shows the dialog.
         * @return ContentDialogResult
         */
        Windows::Foundation::IAsyncOperation<Microsoft::UI::Xaml::Controls::ContentDialogResult> ShowAsync();
        /**
         * @brief Handles when the dialog is closing.
         * @param sender ContentDialog
         * @param args ContentDialogClosingEventArgs
         */
        void OnClosing(const Microsoft::UI::Xaml::Controls::ContentDialog& sender, const Microsoft::UI::Xaml::Controls::ContentDialogClosingEventArgs& args);
        /**
         * @brief Handles when the url textbox is changed.
         * @param sender IInspectable
         * @param args TextChangedEventArgs
         */
        void OnTxtUrlChanged(const IInspectable& sender, const Microsoft::UI::Xaml::Controls::TextChangedEventArgs& args);
        /**
         * @brief Handles when the authenticate combobox is changed.
         * @param sender IInspectable
         * @param args SelectionChangedEventArgs
         */
        void OnCmbAuthenticateChanged(const IInspectable& sender, const Microsoft::UI::Xaml::Controls::SelectionChangedEventArgs& args);
        /**
         * @brief Handles when the url is validated.
         * @param args ParamEventArgs<bool>
         */
        void OnUrlValidated(const ::Nickvision::Events::ParamEventArgs<bool>& args);
        /**
         * @brief Handles when the single file type combobox is changed.
         * @param sender IInspectable
         * @param args SelectionChangedEventArgs
         */
        void OnCmbFileTypeSingleChanged(const IInspectable& sender, const Microsoft::UI::Xaml::Controls::SelectionChangedEventArgs& args);
        /**
         * @brief Prompts the user to select a folder to save a single download.
         * @param sender IInspectable
         * @param args RoutedEventArgs
         */
        Windows::Foundation::IAsyncAction SelectSaveFolderSingle(const IInspectable& sender, const Microsoft::UI::Xaml::RoutedEventArgs& args);
        /**
         * @brief Reverts the filename for a single download to its original title.
         * @param sender IInspectable
         * @param args RoutedEventArgs
         */
        void RevertFilenameSingle(const IInspectable& sender, const Microsoft::UI::Xaml::RoutedEventArgs& args);
        /**
         * @brief Prompts the user to select a folder to save a playlist download.
         * @param sender IInspectable
         * @param args RoutedEventArgs
         */
        Windows::Foundation::IAsyncAction SelectSaveFolderPlaylist(const IInspectable& sender, const Microsoft::UI::Xaml::RoutedEventArgs& args);
        /**
         * @brief Handles when the playlist number titles toggle is changed.
         * @param sender IInspectable
         * @param args RoutedEventArgs
         */
        void OnTglNumberTitlesPlaylistToggled(const IInspectable& sender, const Microsoft::UI::Xaml::RoutedEventArgs& args);

    private:
        std::shared_ptr<::Nickvision::TubeConverter::Shared::Controllers::AddDownloadDialogController> m_controller;
        HWND m_hwnd;
    };
}

namespace winrt::Nickvision::TubeConverter::WinUI::factory_implementation 
{
    class AddDownloadDialog : public AddDownloadDialogT<AddDownloadDialog, implementation::AddDownloadDialog>
    {

    };
}

#endif //ADDDOWNLOADDIALOG_H