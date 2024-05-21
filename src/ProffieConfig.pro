TEMPLATE = app
CONFIG += c++17
CONFIG -= qt

QMAKE_CXXFLAGS += $$system(wx-config --cxxflags)
QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.14

VERSION = 2.0.0
DEFINES += VERSION=\\\"$$VERSION\\\"

linux: {
RESOURCEPATH=\\\"/resources/\\\"
}
macx: {
RESOURCEPATH=\\\"/../Resources/\\\"
}
win32: {
RESOURCEPATH=\\\"/resources/\\\"
DEFINES += wxProgressDialog=wxGenericProgressDialog
DEFINES += wxMessageDialog=wxGenericMessageDialog
DEFINES += wxAboutBox=wxGenericAboutBox
INCLUDEPATH += /opt/mxe/usr/x86_64-w64-mingw32.static/include
}

INCLUDEPATH += /usr/local/include/wx-3.3

DEFINES += RESOURCEPATH=$$RESOURCEPATH
DEFINES += PROPPATH="$$RESOURCEPATH\ \\\"props/\\\""
DEFINES += CONFIGPATH="$$RESOURCEPATH\ \\\"ProffieOS/config/\\\""
DEFINES += STYLE_EDITORPATH="$$RESOURCEPATH\ \\\"StyleEditor/style_editor.html\\\""


# macOS resource handling
ICON = ../resources/icons/icon.icns
RESOURCE.files += ../resources/macOS/arduino
RESOURCE.files += ../resources/ProffieOS
RESOURCE.files += ../resources/StyleEditor
RESOURCE.files += ../resources/props
RESOURCE.path = Contents/Resources
QMAKE_BUNDLE_DATA += RESOURCE

# Windows resource handling
RC_ICONS = ../resources/icons/icon.ico
RC_DEFINES += VERSION=\\\"$$VERSION\\0\\\" WIN_VERSION=$$replace(VERSION, "\.", ","),0
RC_INCLUDEPATH = /opt/mxe/usr/x86_64-w64-mingw32.static/include/wx-3.3/
RC_FILE += ./ProffieConfig_resource.rc

LIBS += $$system(wx-config --libs all)

SOURCES += \
    main.cpp \
    appcore/state.cpp \
    config/config.cpp \
    config/defaults.cpp \
    config/settings.cpp \
    log/logger.cpp \
    pconf/pconf.cpp \
    prop/propfile.cpp \
    proffieconstructs/range.cpp \
    proffieconstructs/vector3d.cpp \
    stylepreview/webview.cpp \
    stylepreview/blade.cpp \
    styleeditor/styleeditor.cpp \
    styleeditor/blocks/bitsctrl.cpp \
    styleeditor/blocks/block.cpp \
    styleeditor/blocks/styleblock.cpp \
    stylemanager/stylemanager.cpp \
    styles/bladestyle.cpp \
    styles/parse.cpp \
    styles/elements/args.cpp \
    styles/elements/builtin.cpp \
    styles/elements/colors.cpp \
    styles/elements/colorstyles.cpp \
    styles/elements/effects.cpp \
    styles/elements/functions.cpp \
    styles/elements/layers.cpp \
    styles/elements/lockuptype.cpp \
    styles/elements/timefunctions.cpp \
    styles/elements/transitions.cpp \
    styles/elements/wrappers.cpp \
    styles/documentation/styledocs.cpp \
    test/uitest.cpp \
    utility/time.cpp \
    ui/bool.cpp \
    ui/combobox.cpp \
    ui/drawer.cpp \
    ui/frame.cpp \
    ui/movable.cpp \
    ui/numeric.cpp \
    ui/numericdec.cpp \
    ui/text.cpp \
    ui/toggle.cpp \
    ui/selection.cpp \

HEADERS += \
    appcore/interfaces.h \
    appcore/state.h \
    config/button.h \
    config/config.h \
    config/defaults.h \
    config/settings.h \
    log/logger.h \
    pconf/pconf.h \
    prop/propfile.h \
    proffieconstructs/range.h \
    proffieconstructs/utilfuncs.h \
    proffieconstructs/vector3d.h \
    stylepreview/webview.h \
    stylepreview/blade.h \
    styleeditor/styleeditor.h \
    styleeditor/blocks/block.h \
    styleeditor/blocks/styleblock.h \
    stylemanager/stylemanager.h \
    styles/bladestyle.h \
    styles/elements/args.h \
    styles/elements/builtin.h \
    styles/elements/colors.h \
    styles/elements/colorstyles.h \
    styles/elements/effects.h \
    styles/elements/functions.h \
    styles/elements/layers.h \
    styles/elements/lockuptype.h \
    styles/elements/timefunctions.h \
    styles/elements/transitions.h \
    styles/elements/wrappers.h \
    styles/generator.h \
    styles/parse.h \
    styles/documentation/styledocs.h \
    test/uitest.h \
    utility/time.h \
    ui/bool.h \
    ui/combobox.h \
    ui/drawer.h \
    ui/frame.h \
    ui/movable.h \
    ui/numeric.h \
    ui/numericdec.h \
    ui/text.h \
    ui/toggle.h \
    ui/selection.h \
