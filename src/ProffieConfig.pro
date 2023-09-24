TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

#PRECOMPILED_HEADER += \
#    include/misc.h \
#    include/defines.h \
#    include/configuration.h \
#    include/appstate.h

#precompile_header:!isEmpty(PRECOMPILED_HEADER) {
#DEFINES += USING_PCH
#}

QMAKE_CXXFLAGS += $$system(wx-config --cxxflags)
LIBS += $$system(wx-config --libs all)

SOURCES += \
        main.cpp \
        sources/initializer.cpp \
        sources/mainwindow.cpp \
        sources/misc.cpp \
        sources/configuration.cpp \
        sources/generalpage.cpp \
        sources/presetspage.cpp \
        sources/bladespage.cpp \
        sources/hardwarepage.cpp

HEADERS += \
    include/misc.h \
    include/defines.h \
    include/configuration.h \
    include/appstate.h \
    include/initializer.h \
    include/mainwindow.h \
    include/generalpage.h \
    include/presetspage.h \
    include/bladespage.h \
    include/hardwarepage.h

INCLUDEPATH += ./include \
                /opt/mxe/usr/i686-w64-mingw32.static/include/wx-3.1/ \
                #/opt/mxe/usr/i686-w64-mingw32.static/include/wx-3.1/wx/ \
                /opt/mxe/usr/i686-w64-mingw32.static/lib/wx/include/i686-w64-mingw32.static-msw-unicode-static-3.1/

