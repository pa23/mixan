QT += core gui
TARGET = mixan
TEMPLATE = app
SOURCES += main.cpp \
    mainwindow.cpp \
    numcompfuns.cpp \
    mixfuns.cpp \
    settingsdialog.cpp \
    material.cpp \
    mix.cpp \
    mixanerror.cpp
HEADERS += mainwindow.h \
    constants.h \
    numcompfuns.h \
    mixfuns.h \
    settingsdialog.h \
    material.h \
    mix.h \
    mixanerror.h
FORMS += mainwindow.ui \
    settingsdialog.ui
unix: {
    INCLUDEPATH += . \
        /usr/include/qwt
    LIBS += -L/usr/lib \
        -lqwt \
        -Wl,-rpath,.
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
    INCLUDEPATH += . \
        c:\\qwt\\src
    LIBS += -Lc:\\qwt\\lib \
        -lqwt \
        -Wl,-rpath,.
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
RESOURCES += mixan_icons.qrc
RC_FILE += mixan_icon.rc
