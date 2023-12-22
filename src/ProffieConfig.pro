TEMPLATE = app
CONFIG += c++20
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
RESOURCE.path = Contents/Resources
QMAKE_BUNDLE_DATA += RESOURCE

# Windows resource handling
RC_ICONS = ../resources/icons/icon.ico

LIBS += $$system(wx-config --libs all)

SOURCES += \
    core/appstate.cpp \
    core/fileparse.cpp \
    core/propfile.cpp \
    main.cpp \
    core/mainwindow.cpp \
    pages/generalpage.cpp \
    pages/presetspage.cpp \
    pages/bladespage.cpp \
    pages/bladeidpage.cpp \
    pages/proppage.cpp \
    tools/arduino.cpp \
    tools/serialmonitor.cpp \
    elements/misc.cpp \
    elements/progress.cpp \
    config/configuration.cpp

HEADERS += \
    core/appstate.h \
    core/fileparse.h \
    core/propfile.h \
    pages/generalpage.h \
    pages/presetspage.h \
    pages/bladespage.h \
    pages/bladeidpage.h \
    pages/proppage.h \
    elements/misc.h \
    elements/threadrunner.h \
    elements/progress.h \
    core/defines.h \
    core/mainwindow.h \
    config/configuration.h \
    tools/arduino.h \
    tools/serialmonitor.h
