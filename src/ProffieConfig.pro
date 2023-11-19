TEMPLATE = app
CONFIG += c++20
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

QMAKE_CXXFLAGS += $$system(wx-config --cxxflags)
LIBS += $$system(wx-config --libs all)

SOURCES += \
        main.cpp \
        sources/arduino.cpp \
        sources/mainwindow.cpp \
        sources/misc.cpp \
        sources/configuration.cpp \
        sources/generalpage.cpp \
        sources/presetspage.cpp \
        sources/bladespage.cpp \
        sources/hardwarepage.cpp \
        sources/progress.cpp \
        sources/proppage.cpp \
        sources/serialmonitor.cpp \
        sources/styleeditor.cpp

HEADERS += \
    include/arduino.h \
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
    include/styleeditor.h \
    include/threadrunner.h

INCLUDEPATH += ./include
