QT += core
QT += qml-private
QT -= gui

CONFIG += c++11

TARGET = qml-parser
DESTDIR = dist
CONFIG += console
CONFIG += no_lflags_merge
CONFIG += app_bundle

TEMPLATE = app

DEFINES += PARSER_H \
           QT_CREATOR

SOURCES += parser.h \
           parser.cpp \
           main.cpp \
           AstGenerator.cpp \
           AstGenerator.h \
           AstGeneratorBase.cpp \
           AstGeneratorBase.h \
           AstGeneratorJavascriptBlock.cpp \
           AstGeneratorJavascriptBlock.h \
           Location.cpp \
           Location.h \
           3rdparty/lz-string/src/lzstring.h \
           3rdparty/lz-string/src/lzstring.cpp

INCLUDEPATH += 3rdparty/json/include \
               3rdparty/ordered-map/include

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0
