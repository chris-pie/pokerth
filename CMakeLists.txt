cmake_minimum_required(VERSION 3.16)
project(pokerth-neuro)

set(CMAKE_CXX_STANDARD 17)

# Add all include directories
include_directories(
    ${CMAKE_SOURCE_DIR}/src/third_party/qtsingleapplication
    ${CMAKE_SOURCE_DIR}/src/third_party/qtlockedfile
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/engine
    ${CMAKE_SOURCE_DIR}/src/gui
    ${CMAKE_SOURCE_DIR}/src/net
    ${CMAKE_SOURCE_DIR}/src/engine/local_engine
    ${CMAKE_SOURCE_DIR}/src/engine/network_engine
    ${CMAKE_SOURCE_DIR}/src/config
    ${CMAKE_SOURCE_DIR}/src/gui/qt
    ${CMAKE_SOURCE_DIR}/src/gui/qt/connecttoserverdialog
    ${CMAKE_SOURCE_DIR}/src/core
    ${CMAKE_SOURCE_DIR}/src/gui/qt/sound
    ${CMAKE_SOURCE_DIR}/src/gui/qt/qttools
    ${CMAKE_SOURCE_DIR}/src/gui/qt/chattools
    ${CMAKE_SOURCE_DIR}/src/gui/qt/qttools/qthelper
    ${CMAKE_SOURCE_DIR}/src/gui/qt/gametable
    ${CMAKE_SOURCE_DIR}/src/gui/qt/gametable/startsplash
    ${CMAKE_SOURCE_DIR}/src/gui/qt/gametable/log
    ${CMAKE_SOURCE_DIR}/src/gui/qt/aboutpokerth
    ${CMAKE_SOURCE_DIR}/src/gui/qt/createnetworkgamedialog
    ${CMAKE_SOURCE_DIR}/src/gui/qt/createinternetgamedialog
    ${CMAKE_SOURCE_DIR}/src/gui/qt/joinnetworkgamedialog
    ${CMAKE_SOURCE_DIR}/src/gui/qt/newlocalgamedialog
    ${CMAKE_SOURCE_DIR}/src/gui/qt/settingsdialog
    ${CMAKE_SOURCE_DIR}/src/gui/qt/settingsdialog/selectavatardialog
    ${CMAKE_SOURCE_DIR}/src/gui/qt/settingsdialog/manualblindsorderdialog
    ${CMAKE_SOURCE_DIR}/src/gui/qt/startnetworkgamedialog
    ${CMAKE_SOURCE_DIR}/src/gui/qt/startwindow
    ${CMAKE_SOURCE_DIR}/src/gui/qt/serverlistdialog
    ${CMAKE_SOURCE_DIR}/src/gui/qt/styles
    ${CMAKE_SOURCE_DIR}/src/gui/qt/changecontentdialog
    ${CMAKE_SOURCE_DIR}/src/gui/qt/changecompleteblindsdialog
    ${CMAKE_SOURCE_DIR}/src/gui/qt/internetgamelogindialog
    ${CMAKE_SOURCE_DIR}/src/gui/qt/mymessagedialog
    ${CMAKE_SOURCE_DIR}/src/gui/qt/gamelobbydialog
    ${CMAKE_SOURCE_DIR}/src/gui/qt/timeoutmsgbox
    ${CMAKE_SOURCE_DIR}/src/gui/qt/logfiledialog
    ${CMAKE_SOURCE_DIR}/src/gui/qt/mymessagebox
    ${CMAKE_SOURCE_DIR}/src/third_party/websocketpp
    ${CMAKE_SOURCE_DIR}/src/third_party/NeuroIntegration
    ${CMAKE_SOURCE_DIR}/src/third_party/NeuroIntegration/Websocketpp
    ${CMAKE_SOURCE_DIR}/mocs
    ${CMAKE_SOURCE_DIR}/uics
)

# Add system/external includes (customize these for your build environment)
include_directories(
    ../boost/
    ../SDL/include
    ../SDL/include/SDL
    ../SDL_mixer/include
    ../openssl/include
    ../gsasl/include
    ../curl/include
    ../sqlite
    ../zlib
    /opt/mingw/usr/i686-w64-mingw32.static/qt5/include
    /opt/mingw/usr/i686-w64-mingw32.static/qt5/include/QtWidgets
    /opt/mingw/usr/i686-w64-mingw32.static/qt5/include/QtGui
    /opt/mingw/usr/i686-w64-mingw32.static/qt5/include/QtNetwork
    /opt/mingw/usr/i686-w64-mingw32.static/qt5/include/QtSql
    /opt/mingw/usr/i686-w64-mingw32.static/qt5/include/QtCore
    /opt/mingw/usr/i686-w64-mingw32.static/qt5/mkspecs/win32-g++
)

# Add source files
file(GLOB_RECURSE SOURCES
    src/*.cpp
    src/*.c
    src/*.h
)

add_executable(pokerth-neuro ${SOURCES})

# Optionally link to libraries here if needed, e.g., target_link_libraries(pokerth-neuro ...)
