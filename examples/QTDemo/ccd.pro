TEMPLATE = app
TARGET = ccd
QT += core \
    gui
CONFIG(debug, debug|release):DSTDIR = debug
else:DSTDIR = release
OBJECTS_DIR = $$DSTDIR/.obj
MOC_DIR = $$DSTDIR/.moc
UI_DIR = $$DSTDIR/.ui
RCC_DIR = $$DSTDIR/.rcc

# QMAKE_CFLAGS += -std=c++0x
# QMAKE_CXXFLAGS += -pedantic
HEADERS += include/device_ui/cam_ui_qhy5.h \
    include/devices/cam_qhy5.h \
    include/devices/qhy5_core.h \
    include/debayer.h \
    include/device_ui/cam_ui_qhy8l.h \
    include/devices/cam_qhy8l.h \
    include/histogramwnd.h \
    include/maindef.h \
    include/about.h \
    include/device_ui/cam_ui_qhy9.h \
    include/devices/cam_qhy9.h \
    include/lusb.h \
    include/devices/qhy6_core.h \
    include/devices/cam_qhy6.h \
    include/device_ui/cam_ui_qhy6.h \
    include/device_ui/cam_ui_null.h \
    include/devices/cam_null.h \
    include/params.h \
    include/configwnd.h \
    include/ui_device_base.h \
    include/sessionswnd.h \
    include/timer.h \
    include/camera.h \
    include/ccd.h \
    include/dib.h \
    include/graph.h \
    include/image.h \
    include/photovideownd.h \
    include/tiff.h \
    include/utils.h \
    include/device_ui/cam_ui_qhy5lii.h \
    include/devices/cam_qhy5lii.h \
    include/devices/qhy5lii_core.h

SOURCES += src/device_ui/cam_ui_qhy5.cpp \
    src/devices/cam_qhy5.cpp \
    src/devices/qhy5_core.cpp \
    src/debayer.cpp \
    src/device_ui/cam_ui_qhy8l.cpp \
    src/devices/cam_qhy8l.cpp \
    src/histogramwnd.cpp \
    src/maindef.cpp \
    src/about.cpp \
    src/device_ui/cam_ui_qhy9.cpp \
    src/devices/cam_qhy9.cpp \
    src/lusb.cpp \
    src/devices/qhy6_core.cpp \
    src/devices/cam_qhy6.cpp \
    src/device_ui/cam_ui_qhy6.cpp \
    src/device_ui/cam_ui_null.cpp \
    src/devices/cam_null.cpp \
    src/params.cpp \
    src/configwnd.cpp \
    src/ui_device_base.cpp \
    src/sessionswnd.cpp \
    src/camera.cpp \
    src/ccd.cpp \
    src/dib.cpp \
    src/graph.cpp \
    src/image.cpp \
    src/main.cpp \
    src/photovideownd.cpp \
    src/tiff.cpp \
    src/utils.cpp \
    src/device_ui/cam_ui_qhy5lii.cpp \
    src/devices/cam_qhy5lii.cpp \
    src/devices/qhy5lii_core.cpp
FORMS += ui/configwnd.ui \
    ui/histogramwnd.ui \
    ui/about.ui \
    ui/device_ui/cam_ui_qhy5.ui \
    ui/device_ui/cam_ui_qhy8l.ui \
    ui/device_ui/cam_ui_qhy9.ui \
    ui/device_ui/cam_ui_qhy6.ui \
    ui/device_ui/cam_ui_null.ui \
    ui/sessionswnd.ui \
    ui/ccd.ui \
    ui/devicewnd.ui \
    ui/photovideownd.ui \
    ui/device_ui/cam_ui_qhy5lii.ui
RESOURCES += 
INCLUDEPATH += include/ \
    include/devices/ \
    include/device_ui/ \
    ./
LIBS += -ltiff \
    -lrt \
    -lraw \
    -lgomp \
    -lqhyccd \
    -lusb-1.0 \
    -llcms \
    `pkg-config --libs --cflags opencv`
