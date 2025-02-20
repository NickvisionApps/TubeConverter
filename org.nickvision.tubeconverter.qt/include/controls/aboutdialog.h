#ifndef ABOUTDIALOG_H
#define ABOUTDIALOG_H

#include <string>
#include <QDialog>
#include <libnick/app/appinfo.h>

namespace Ui { class AboutDialog; }

namespace Nickvision::TubeConverter::Qt::Controls
{
    /**
     * @brief A dialog for displaying information about the application.
     */
    class AboutDialog : public QDialog
    {
    Q_OBJECT

    public:
        /**
         * @brief Constructs an AboutDialog.
         * @param appInfo The AppInfo object for the application
         * @param debugInformation The debug information for the application
         * @param parent The parent widget
         */
        AboutDialog(const App::AppInfo& appInfo, const std::string& debugInformation, QWidget* parent = nullptr);
        /**
         * @brief Destructs an AboutDialog.
         */
        ~AboutDialog();

    private Q_SLOTS:
        /**
         * @brief Copies the debug information to the clipboard.
         */
        void copyDebugInformation();

    private:
        Ui::AboutDialog* m_ui;
        QString m_debugInformation;
    };
}

#endif //ABOUTDIALOG_H
