TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

QMAKE_CXXFLAGS += $$system(wx-config --cxxflags)
LIBS += $$system(wx-config --libs all)

SOURCES += \
        main.cpp

INCLUDEPATH += /usr/local/include/wx-3.3 \
            /usr/local/lib/wx/include/gtk3-unicode-3.3/

HEADERS += \
    defines.h
