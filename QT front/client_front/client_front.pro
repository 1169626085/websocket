QT       += core gui
QT       += core gui network
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    global.cpp \
    httpmgr.cpp \
    loginlog.cpp \
    main.cpp \
    mainwindow.cpp \
    registerlog.cpp \
    tcpmgr.cpp \
    timerbtn.cpp

HEADERS += \
    global.h \
    httpmgr.h \
    loginlog.h \
    mainwindow.h \
    registerlog.h \
    singleton.h \
    tcpmgr.h \
    timerbtn.h

FORMS += \
    loginlog.ui \
    mainwindow.ui \
    registerlog.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
