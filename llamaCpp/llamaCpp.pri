SOURCES += \
        llamaCpp/common/build-info.cpp \
        llamaCpp/common/common.cpp \
        llamaCpp/common/console.cpp \
        llamaCpp/common/grammar-parser.cpp \
        llamaCpp/common/sampling.cpp \
        llamaCpp/common/train.cpp

HEADERS += \
    llamaCpp/common/base64.hpp \
    llamaCpp/common/common.h \
    llamaCpp/common/console.h \
    llamaCpp/common/grammar-parser.h \
    llamaCpp/common/log.h \
    llamaCpp/common/sampling.h \
    llamaCpp/common/stb_image.h \
    llamaCpp/common/train.h \
    llamaCpp/ggml-alloc.h \
    llamaCpp/ggml-backend-impl.h \
    llamaCpp/ggml-backend.h \
    llamaCpp/ggml-impl.h \
    llamaCpp/ggml-quants.h \
    llamaCpp/ggml.h \
    llamaCpp/llama.h \
    llamaCpp/unicode.h

INCLUDEPATH += llamaCpp/
INCLUDEPATH += llamaCpp/common

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/./ -lggml_shared
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/./ -lggml_shared

INCLUDEPATH += $$PWD/llamaCpp
DEPENDPATH += $$PWD/llamaCpp

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/./ -lllama
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/./ -lllamad

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/./ -lggml_static
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/./ -lggml_staticd
