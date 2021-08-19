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
           Location.h

INCLUDEPATH += 3rdparty/json/include \
               3rdparty/ordered-map/include \

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

OTHER_FILES += \
  bin/index.js \
  scripts/package-for-mac.sh \
  scripts/package-for-linux.sh \
  sandbox/test_function_multiple_args.qml \
  sandbox/test_function_no_arg.qml \
  sandbox/test_function_single_arg.qml \
  sandbox/test_object_attached.qml \
  sandbox/test_object_binding.qml \
  sandbox/test_object_binding_grouped.qml \
  sandbox/test_object_nested.qml \
  sandbox/test_object_on.qml \
  sandbox/test_property.qml \
  sandbox/test_property_alias.qml \
  sandbox/test_property_default.qml \
  sandbox/test_property_list.qml \
  sandbox/test_property_object.qml \
  sandbox/test_property_readonly.qml \
  sandbox/test_signal_multiple_args.qml \
  sandbox/test_signal_no_arg.qml \
  sandbox/test_signal_single_arg.qml \
  sandbox/test_namespace.qml \
  sandbox/test_object_binding_array.qml \
  sandbox/test_object_binding_block.qml
