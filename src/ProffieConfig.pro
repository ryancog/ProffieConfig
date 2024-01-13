TEMPLATE = app
CONFIG += c++17
CONFIG -= qt

QMAKE_CXXFLAGS += $$system(wx-config --cxxflags)
QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.14

VERSION = 1.6.0
DEFINES += VERSION=\\\"$$VERSION\\\"

# macOS resource handling
ICON = ../resources/icons/icon.icns
RESOURCE.files += ../resources/macOS/arduino-cli
RESOURCE.files += ../resources/ProffieOS
RESOURCE.files += ../resources/StyleEditor
RESOURCE.files += ../resources/props
RESOURCE.path = Contents/Resources
QMAKE_BUNDLE_DATA += RESOURCE

# Windows resource handling
RC_ICONS = ../resources/icons/icon.ico
RC_DEFINES += VERSION=\\\"$$VERSION\\0\\\" WIN_VERSION=$$replace(VERSION, "\.", ","),0
RC_INCLUDEPATH = /opt/mxe/usr/i686-w64-mingw32.static/include/wx-3.3/
RC_FILE += ./ProffieConfig_resource.rc

LIBS += $$system(wx-config --libs all)

SOURCES += \
    editor/editorwindow.cpp \
    editor/pages/bladearraydlg.cpp \
    editor/pages/propspage.cpp \
    main.cpp \
    core/appstate.cpp \
    core/utilities/fileparse.cpp \
    core/utilities/misc.cpp \
    core/utilities/progress.cpp \
    core/config/configuration.cpp \
    core/config/settings.cpp \
    core/config/propfile.cpp \
    editor/pages/generalpage.cpp \
    editor/pages/presetspage.cpp \
    editor/pages/bladespage.cpp \
    mainmenu/dialogs/addconfig.cpp \
    mainmenu/mainmenu.cpp \
    onboard/onboard.cpp \
    onboard/pages/dependencypage.cpp \
    onboard/pages/overviewpage.cpp \
    onboard/pages/welcomepage.cpp \
    tools/arduino.cpp \
    tools/serialmonitor.cpp \
    ui/pccombobox.cpp

HEADERS += \
    core/appstate.h \
    core/defines.h \
    core/config/configuration.h \
    core/config/settings.h \
    core/config/propfile.h \
    core/utilities/fileparse.h \
    core/utilities/misc.h \
    core/utilities/threadrunner.h \
    core/utilities/progress.h \
    editor/editorwindow.h \
    editor/pages/bladearraydlg.h \
    editor/pages/generalpage.h \
    editor/pages/presetspage.h \
    editor/pages/bladespage.h \
    editor/pages/propspage.h \
    mainmenu/dialogs/addconfig.h \
    mainmenu/mainmenu.h \
    onboard/onboard.h \
    tools/arduino.h \
    tools/serialmonitor.h \
    ui/pccombobox.h
