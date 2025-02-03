TEMPLATE = app
CONFIG += c++17
CONFIG -= qt

QMAKE_CXXFLAGS += $$system(wx-config --cxxflags)
QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.14

VERSION = 1.6.16
DEFINES += VERSION=\\\"$$VERSION\\\"
win32: DEFINES += wxMessageDialog=wxGenericMessageDialog
win32: DEFINES += wxProgressDialog=wxGenericProgressDialog
win32: DEFINES += wxAboutBox=wxGenericAboutBox

# macOS resource handling
ICON = ../resources/icons/icon.icns
RESOURCE.files += ../resources/macOS/arduino-cli
RESOURCE.files += ../resources/ProffieOS
RESOURCE.files += ../resources/StyleEditor
RESOURCE.files += ../resources/props
RESOURCE.path = Contents/Resources
QMAKE_BUNDLE_DATA += RESOURCE
QMAKE_INFO_PLIST = Info.plist.in
QMAKE_SUBSTITUTES += Info.plist.in

# Windows resource handling
RC_ICONS = ../resources/icons/icon.ico
RC_DEFINES += VERSION=\\\"$$VERSION\\0\\\" WIN_VERSION=$$replace(VERSION, "\.", ","),0
RC_INCLUDEPATH = /opt/mxe/usr/x86_64-w64-mingw32.static/include/wx-3.3/
RC_FILE += ./ProffieConfig_resource.rc

LIBS += $$system(wx-config --libs)

SOURCES += \
    editor/dialogs/bladearraydlg.cpp \
    editor/dialogs/customoptionsdlg.cpp \
    editor/editorwindow.cpp \
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
    ui/pcchoice.cpp \
    ui/pccombobox.cpp \
    ui/pcspinctrl.cpp \
    ui/pcspinctrldouble.cpp \
    ui/pctextctrl.cpp

HEADERS += \
    core/appstate.h \
    core/defines.h \
    core/config/configuration.h \
    core/config/settings.h \
    core/config/propfile.h \
    core/utilities/fileparse.h \
    core/utilities/misc.h \
    core/utilities/progress.h \
    editor/dialogs/bladearraydlg.h \
    editor/dialogs/customoptionsdlg.h \
    editor/editorwindow.h \
    editor/pages/generalpage.h \
    editor/pages/presetspage.h \
    editor/pages/bladespage.h \
    editor/pages/propspage.h \
    mainmenu/dialogs/addconfig.h \
    mainmenu/mainmenu.h \
    onboard/onboard.h \
    tools/arduino.h \
    tools/serialmonitor.h \
    ui/pcchoice.h \
    ui/pccombobox.h \
    ui/pcspinctrl.h \
    ui/pcspinctrldouble.h \
    ui/pctextctrl.h
