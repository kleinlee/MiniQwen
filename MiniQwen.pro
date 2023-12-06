QT += qml quick sql quickcontrols2

CONFIG += qmltypes
QML_IMPORT_NAME = AAAAA
QML_IMPORT_MAJOR_VERSION = 1

HEADERS += \
    humanassets.h \
    llmmodel.h \
    sqlconversationmodel.h
SOURCES += main.cpp \
    humanassets.cpp \
    llmmodel.cpp \
    sqlconversationmodel.cpp

RESOURCES += \
    shader.qrc\
    images.qrc

include(llamaCpp/llamaCpp.pri)

INSTALLS += target

OTHER_FILES += \
    main.qml
