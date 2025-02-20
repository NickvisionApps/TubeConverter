set(OUTPUT_NAME "${PROJECT_NAME}.qt")
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
set(GNOME_NOTIFICATIONS false)
set(STARTUP_NOTIFY false)
set(DBUS_ACTIVATABLE false)
add_definitions(-DQT_NO_KEYWORDS)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Svg)
find_package(qlementine CONFIG REQUIRED)
find_package(qlementine-icons CONFIG REQUIRED)
qt_standard_project_setup()

qt_add_executable(${OUTPUT_NAME}
    "include/controls/aboutdialog.h"
    "include/controls/downloadrow.h"
    "include/controls/historyrow.h"
    "include/controls/infobar.h"
    "include/controls/keyringrow.h"
    "include/controls/navigationbar.h"
    "include/helpers/closeeventfilter.h"
    "include/helpers/qthelpers.h"
    "include/views/adddownloaddialog.h"
    "include/views/credentialdialog.h"
    "include/views/keyringpage.h"
    "include/views/mainwindow.h"
    "include/views/settingspage.h"
    "include/application.h"
    "src/controls/aboutdialog.cpp"
    "src/controls/downloadrow.ui"
    "src/controls/downloadrow.cpp"
    "src/controls/historyrow.ui"
    "src/controls/historyrow.cpp"
    "src/controls/infobar.cpp"
    "src/controls/keyringrow.ui"
    "src/controls/keyringrow.cpp"
    "src/controls/navigationbar.cpp"
    "src/helpers/closeeventfilter.cpp"
    "src/helpers/qthelpers.cpp"
    "src/views/adddownloaddialog.ui"
    "src/views/adddownloaddialog.cpp"
    "src/views/credentialdialog.ui"
    "src/views/credentialdialog.cpp"
    "src/views/keyringpage.ui"
    "src/views/keyringpage.cpp"
    "src/views/mainwindow.ui"
    "src/views/mainwindow.cpp"
    "src/views/settingspage.ui"
    "src/views/settingspage.cpp"
    "src/application.cpp"
    "src/main.cpp"
    "resources/${OUTPUT_NAME}.qrc"
    "../resources/windows/${PROJECT_NAME}.rc")
target_include_directories(${OUTPUT_NAME} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_link_libraries(${OUTPUT_NAME} PRIVATE libparabolic Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Svg qlementine::qlementine qlementine-icons::qlementine-icons)
if(WIN32)
    set_target_properties(${OUTPUT_NAME} PROPERTIES WIN32_EXECUTABLE ON)
endif()

add_custom_target(qt_commands ALL DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/__qt.h")
add_custom_command(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/__qt.h" "${CMAKE_CURRENT_BINARY_DIR}/qt.h" COMMAND "${CMAKE_COMMAND}" -E touch "${CMAKE_CURRENT_BINARY_DIR}/qt.h")

if(WIN32)
    add_custom_command(TARGET qt_commands POST_BUILD COMMAND Qt6::windeployqt ARGS $<TARGET_FILE:${OUTPUT_NAME}> --no-quick-import --no-translations --no-compiler-runtime)
    add_custom_command(TARGET qt_commands POST_BUILD COMMAND "mt.exe" -manifest "${CMAKE_SOURCE_DIR}/resources/windows/${PROJECT_NAME}.manifest" -inputresource:\"$<TARGET_FILE:${OUTPUT_NAME}>\"\;\#1 -outputresource:\"$<TARGET_FILE:${OUTPUT_NAME}>\"\;\#1)
endif()

if(HAS_GETTEXT)
    foreach(LINGUA IN ITEMS ${LINGUAS})
        if(MSVC)
            add_custom_command(TARGET qt_commands PRE_BUILD COMMAND "${CMAKE_COMMAND}" -E make_directory "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/${LINGUA}/LC_MESSAGES")
            add_custom_command(TARGET qt_commands PRE_BUILD COMMAND "${GETTEXT_MSGFMT_EXECUTABLE}" "${CMAKE_SOURCE_DIR}/resources/po/${LINGUA}.po" --output-file="${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/${LINGUA}/LC_MESSAGES/${SHORT_NAME}.mo")
        else()
            add_custom_command(TARGET qt_commands PRE_BUILD COMMAND "${CMAKE_COMMAND}" -E make_directory "${CMAKE_CURRENT_BINARY_DIR}/${LINGUA}/LC_MESSAGES")
            add_custom_command(TARGET qt_commands PRE_BUILD COMMAND "${GETTEXT_MSGFMT_EXECUTABLE}" "${CMAKE_SOURCE_DIR}/resources/po/${LINGUA}.po" --output-file="${CMAKE_CURRENT_BINARY_DIR}/${LINGUA}/LC_MESSAGES/${SHORT_NAME}.mo")
        endif()
    endforeach()
endif()

if(LINUX)
    configure_file("${CMAKE_SOURCE_DIR}/resources/linux/${PROJECT_NAME}.desktop.in" "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.desktop" @ONLY)
    configure_file("${CMAKE_SOURCE_DIR}/resources/linux/${PROJECT_NAME}.in" "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}" @ONLY)
    configure_file("${CMAKE_SOURCE_DIR}/resources/linux/${PROJECT_NAME}.metainfo.xml.in" "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.metainfo.xml" @ONLY)
    install(DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/" DESTINATION "${CMAKE_INSTALL_LIBDIR}/${PROJECT_NAME}")
    install(FILES "${CMAKE_CURRENT_BINARY_DIR}/${OUTPUT_NAME}" PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE DESTINATION "${CMAKE_INSTALL_LIBDIR}/${PROJECT_NAME}")
    install(FILES "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}" PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE DESTINATION "${CMAKE_INSTALL_BINDIR}")
    install(FILES "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.desktop" DESTINATION "${CMAKE_INSTALL_DATADIR}/applications")
    install(FILES "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.metainfo.xml" DESTINATION "${CMAKE_INSTALL_DATADIR}/metainfo")
    install(FILES "${CMAKE_SOURCE_DIR}/resources/${PROJECT_NAME}.svg" "${CMAKE_SOURCE_DIR}/resources/${PROJECT_NAME}-devel.svg" DESTINATION "${CMAKE_INSTALL_DATADIR}/icons/hicolor/scalable/apps")
    install(FILES "${CMAKE_SOURCE_DIR}/resources/${PROJECT_NAME}-symbolic.svg" DESTINATION "${CMAKE_INSTALL_DATADIR}/icons/hicolor/symbolic/apps")

    find_program(UPDATE_DESKTOP_DATABASE_EXECUTABLE update-desktop-database)
    if(UPDATE_DESKTOP_DATABASE_EXECUTABLE)
        install(CODE "execute_process(COMMAND ${UPDATE_DESKTOP_DATABASE_EXECUTABLE})")
    else()
        message(WARNING "update-desktop-database not found, desktop database will not be updated")
    endif()
endif()
