QT       += core gui widgets

CONFIG += c++17

TARGET = OS
TEMPLATE = app

DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
    src/main.cpp \
    src/mainwindow.cpp \
    src/filesystem.cpp \
    src/cli.cpp

HEADERS += \
    include/filesystem.h \
    include/mainwindow.h \
    include/cli.h

FORMS += \
    forms/mainwindow.ui

INCLUDEPATH += $$PWD/include

# Build output directory
win32 {
    BUILD_DIR = $$PWD/build
} else {
    BUILD_DIR = $$PWD/build
}

DESTDIR = $$BUILD_DIR
OBJECTS_DIR = $$BUILD_DIR/obj
MOC_DIR = $$BUILD_DIR/moc
RCC_DIR = $$BUILD_DIR/rcc
UI_DIR = $$BUILD_DIR/ui

# Qt6 on Windows with MinGW
win32 {
    INCLUDEPATH += $$PWD
    LIBS += -lwinmm -lcomdlg32 -lole32 -luuid
}

!win32 {
    LIBS += -lm
}
