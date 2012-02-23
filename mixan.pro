#
#
#    mixan
#    Analysis of granular material mixes and emulsions.
#
#    File: mixan.pro
#
#    Copyright (C) 2011-2012 Artem Petrov <pa2311@gmail.com>
#
#    This program is free software: you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation, either version 3 of the License, or
#    (at your option) any later version.
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
#    GNU General Public License for more details.
#    You should have received a copy of the GNU General Public License
#    along with this program. If not, see <http://www.gnu.org/licenses/>.
#

QT += core gui
TARGET = mixan
TEMPLATE = app
SOURCES += sources/main.cpp \
    sources/mainwindow.cpp \
    sources/numcompfuns.cpp \
    sources/mixfuns.cpp \
    sources/settingsdialog.cpp \
    sources/material.cpp \
    sources/mix.cpp \
    sources/mixanerror.cpp
HEADERS += sources/mainwindow.h \
    sources/constants.h \
    sources/numcompfuns.h \
    sources/mixfuns.h \
    sources/settingsdialog.h \
    sources/material.h \
    sources/mix.h \
    sources/mixanerror.h
FORMS += sources/mainwindow.ui \
    sources/settingsdialog.ui
unix: {
    INCLUDEPATH += . \
        /usr/include/qwt
    LIBS += -L/usr/lib \
        -lqwt \
        -Wl,-rpath,.
    DESTDIR = build/unix/bin
    MOC_DIR = build/unix/moc
    RCC_DIR = build/unix/rc
    UI_HEADERS_DIR = build/unix/ui_h
    CONFIG (debug, debug|release) {
        OBJECTS_DIR = build/unix/debug
    }
    else {
        OBJECTS_DIR = build/unix/release
    }
    target.path = $$PREFIX/bin
    INSTALLS += target
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
    UI_HEADERS_DIR = build\\win\\ui_h
    CONFIG (debug, debug|release) {
        OBJECTS_DIR = build\\win\\debug
    }
    else {
        OBJECTS_DIR = build\\win\\release
    }
}
RESOURCES += sources/mixan_icons.qrc
RC_FILE += sources/mixan_icon.rc
