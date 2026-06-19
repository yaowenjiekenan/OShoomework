QT       += core gui widgets

CONFIG += c++17 console

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

DESTDIR = $$PWD
OBJECTS_DIR = build/obj
MOC_DIR = build/moc
RCC_DIR = build/rcc
UI_DIR = build/ui

# Qt6 on Windows with MinGW
win32 {
    INCLUDEPATH += $$PWD
    LIBS += -lwinmm -lcomdlg32 -lole32 -luuid
    QMAKE_CXXFLAGS += -finput-charset=UTF-8 -fexec-charset=UTF-8
}

!win32 {
    LIBS += -lm
}
