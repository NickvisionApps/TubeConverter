#include "helpers/qthelpers.h"
#include <QApplication>
#include <QString>
#include <QTimer>

using namespace oclero::qlementine::icons;

namespace Nickvision::TubeConverter::Qt::Helpers
{
    void QtHelpers::dispatchToMainThread(const std::function<void()>& function)
    {
        QTimer* timer{ new QTimer() };
        timer->moveToThread(QApplication::instance()->thread());
        timer->setSingleShot(true);
        QObject::connect(timer, &QTimer::timeout, [=]()
        {
            function();
            timer->deleteLater();
        });
        QMetaObject::invokeMethod(timer, "start", ::Qt::ConnectionType::AutoConnection, Q_ARG(int, 0));
    }

    void QtHelpers::setComboBoxItems(QComboBox* comboBox, const std::vector<std::string>& items, const std::string& selected)
    {
        size_t selectedIndex{ 0 };
        comboBox->clear();
        for(size_t i = 0; i < items.size(); i++)
        {
            const std::string& item{ items[i] };
            comboBox->addItem(QString::fromStdString(item));
            if(item == selected)
            {
                selectedIndex = i;
            }
        }
        comboBox->setCurrentIndex(selectedIndex);
    }

    QIcon QtHelpers::getIcon(Icons16 icon)
    {
        return QIcon(iconPath(icon));
    }
}
