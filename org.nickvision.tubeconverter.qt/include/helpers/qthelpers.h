#ifndef QTHELPERS_H
#define QTHELPERS_H

#include <functional>
#include <string>
#include <vector>
#include <QComboBox>
#include <QIcon>
#include <oclero/qlementine/icons/Icons16.hpp>

#define QLEMENTINE_ICON(NAME) ::Nickvision::TubeConverter::Qt::Helpers::QtHelpers::getIcon(oclero::qlementine::icons::Icons16::NAME)

namespace Nickvision::TubeConverter::Qt::Helpers::QtHelpers
{
    /**
     * @brief Runs the function on the main UI thread.
     * @param function The function to run
     */
    void dispatchToMainThread(const std::function<void()>& function);
    /**
     * @brief Sets the items of a QComboBox from a vector of strings.
     * @param comboBox The QComboBox to set the items of
     * @param items The items to set
     * @param selected An option string that should be selected
     */
    void setComboBoxItems(QComboBox* comboBox, const std::vector<std::string>& items, const std::string& selected = "");
    /**
     * @brief Gets a QIcon from a qlementine icon.
     * @param icon The qlementine icon
     * @return The QIcon
     */
    QIcon getIcon(oclero::qlementine::icons::Icons16 icon);
}

#endif //QTHELPERS_H