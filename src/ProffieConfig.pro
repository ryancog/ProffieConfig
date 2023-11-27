TEMPLATE = app
CONFIG += c++20
CONFIG -= qt

QMAKE_CXXFLAGS += $$system(wx-config --cxxflags)
QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.14

VERSION = 1.5.0-dev
DEFINES += VERSION=\\\"$$VERSION\\\"

# macOS resource handling
ICON = ../resources/icons/icon.icns
RESOURCE.files += ../resources/macOS/arduino-cli
RESOURCE.files += ../resources/ProffieOS
RESOURCE.files += ../resources/StyleEditor
RESOURCE.path = Contents/Resources
QMAKE_BUNDLE_DATA += RESOURCE

# Windows resource handling
RC_ICONS = ../resources/icons/icon.ico

LIBS += $$system(wx-config --libs all)

SOURCES += \
        main.cpp \
        sources/arduino.cpp \
        sources/bladeidpage.cpp \
        sources/mainwindow.cpp \
        sources/misc.cpp \
        sources/configuration.cpp \
        sources/generalpage.cpp \
        sources/presetspage.cpp \
        sources/bladespage.cpp \
        sources/hardwarepage.cpp \
        sources/progress.cpp \
        sources/proppage.cpp \
        sources/serialmonitor.cpp

HEADERS += \
    include/arduino.h \
    include/bladeidpage.h \
    include/misc.h \
    include/defines.h \
    include/configuration.h \
    include/appstate.h \
    include/mainwindow.h \
    include/generalpage.h \
    include/presetspage.h \
    include/bladespage.h \
    include/hardwarepage.h \
    include/progress.h \
    include/proppage.h \
    include/serialmonitor.h \
    include/threadrunner.h

INCLUDEPATH += ./include
