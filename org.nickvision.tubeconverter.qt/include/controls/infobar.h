#ifndef INFOBAR_H
#define INFOBAR_H

#include <functional>
#include <QDockWidget>
#include <QString>
#include <libnick/notifications/notificationsenteventargs.h>

namespace Ui { class InfoBar; }

namespace Nickvision::TubeConverter::Qt::Controls
{
    /**
     * @brief A widget for displaying a notification within a window.
     */
    class InfoBar : public QDockWidget
    {
    Q_OBJECT

    public:
        /**
         * @brief Constructs an InfoBar.
         * @param parent The parent widget
         */
        InfoBar(QWidget* parent = nullptr);
        /**
         * @brief Destructs an InfoBar.
         */
        ~InfoBar();
        /**
         * @brief Shows the InfoBar with the provided notification.
         * @param args NotificationSentEventArgs
         * @param actionButtonText Optional text for the action button
         * @param callback An optional function to call when the action button is clicked
         */
        void show(const Notifications::NotificationSentEventArgs& args, const QString& actionButtonText = {}, const std::function<void()>& callback = {});

    private Q_SLOTS:
        /**
         * @brief Handles when the action button is clicked.
         */
        void action();

    private:
        Ui::InfoBar* m_ui;
        std::function<void()> m_callback;
    };
}

#endif //INFOBAR_H
