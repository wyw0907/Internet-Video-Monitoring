TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.c \
    cJSON.c \
    PicDeal.c \
    TcpSocket.c \
    VideoCtrl.c

HEADERS += \
    include/PicDeal.h \
    include/TcpSocket.h \
    include/VideoCtrl.h \
    include/cJSON.h \
    include/curl/curl.h \
    include/curl/curlbuild.h \
    include/curl/curlrules.h \
    include/curl/curlver.h \
    include/curl/easy.h \
    include/curl/mprintf.h \
    include/curl/multi.h \
    include/curl/stdcheaders.h \
    include/curl/typecheck-gcc.h \
    include/config.h

DISTFILES += \
    Makefile \
    library/libcurl.a \
    library/libcurl.so

