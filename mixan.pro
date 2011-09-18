QT += core gui
TARGET = mixan
TEMPLATE = app
SOURCES += main.cpp \
    mainwindow.cpp \
    numcompfuns.cpp \
    granularmaterial.cpp \
    granularmix.cpp
HEADERS += mainwindow.h \
    constants.h \
    numcompfuns.h \
    granularmaterial.h \
    granularmix.h
FORMS += mainwindow.ui
unix: {
    DESTDIR = build/unix/bin
    MOC_DIR = build/unix/moc
    RCC_DIR = build/unix/rc
    CONFIG (debug, debug|release) {
        OBJECTS_DIR = build/unix/debug
    }
    else {
        OBJECTS_DIR = build/unix/release
    }
}
win32: {
    DESTDIR = build\\win\\bin
    MOC_DIR = build\\win\\moc
    RCC_DIR = build\\win\\rc
    CONFIG (debug, debug|release) {
        OBJECTS_DIR = build\\win\\debug
    }
    else {
        OBJECTS_DIR = build\\win\\release
    }
}
