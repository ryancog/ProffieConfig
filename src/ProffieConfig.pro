TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

QMAKE_CXXFLAGS += $$system(wx-config --cxxflags)
LIBS += $$system(wx-config --libs all)

SOURCES += main.cpp \
        sources/initializer.cpp \
        sources/mainwindow.cpp \
        sources/misc.cpp \
        sources/configuration.cpp \
        sources/generalpage.cpp \
        sources/presetspage.cpp \
        sources/bladespage.cpp \
        sources/hardwarepage.cpp

HEADERS += include/generalpage.h \
    include/presetspage.h \
    include/bladespage.h \
    include/hardwarepage.h \
    include/defines.h \
    include/misc.h \
    include/configuration.h \
    include/appstate.h \
    include/initializer.h \
    include/mainwindow.h

INCLUDEPATH += /usr/local/include/wx-3.3 \
            include/


