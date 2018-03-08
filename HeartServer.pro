TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt


SOURCES += main.cpp \
    epollserver.cpp \
    threadpool.cpp \
    json/json_reader.cpp \
    json/json_value.cpp \
    json/json_writer.cpp \
    redis.cpp \
    config/readFile.cpp

HEADERS += \
    epollserver.h \
    singleton.h \
    threadpool.h \
    global.h \
    json/autolink.h \
    json/config.h \
    json/features.h \
    json/forwards.h \
    json/json.h \
    json/reader.h \
    json/value.h \
    json/writer.h \
    json/json_batchallocator.h \
    include/hiredis.h \
    redis.h \
    config/readFile.h

INCLUDEPATH += . \
            include \
            lib \
            config
DEPENDPATH += . \
            include \
            lib \
            config

LIBS += -L$$PWD/lib -lhiredis -lpthread -lz


