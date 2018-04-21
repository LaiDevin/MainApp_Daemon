#-------------------------------------------------
#
# Project created by QtCreator 2018-02-06T14:16:40
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = SmartGateway
TEMPLATE = app

DebugBuild {
    TARGET = $${TARGET}d
    DESTDIR  = $${PWD}/bin/debug
} else {
    DESTDIR  = $${PWD}/bin/release
}


win32 {
    !equals(QT_MAJOR_VERSION, 5) | greaterThan(QT_MINOR_VERSION, 6) {
       error("Unsupported Qt version, 5.6.x or lesser is required")
    } else {
       message(the Qt version is $$[QT_VERSION])
    }
}


# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += \
        main.cpp \
        mainwindow.cpp \
         GatewayServerDlg.cpp \
         smartwin.cpp\
	linkageconfiguration.cpp \
        powerconfigdlg.cpp\
     gatewaylinkerdlg.cpp \
#    Translation.cpp

HEADERS += \
        mainwindow.h \
        GatewayServerDlg.h \
        smartwin.h \
        powerconfigdlg.h\
        linkageconfiguration.h\
    gatewaylinkerdlg.h \
#    Translation.h


FORMS += \
        mainwindow.ui \
        GatewayServerDlg.ui \
	linkageconfiguration.ui\
        powerconfigdlg.ui\
     gatewaylinkerdlg.ui

LIBS += -lAdvapi32 \
        -lwsock32

include (./common/common.pri)
include (./drivers/drivers.pri)
include (./socket/socket.pri)
include (./fas/fas.pri)
include (./serveragreement/serveragreement.pri)

TRANSLATIONS = gateway_en.ts gateway_zh.ts

RESOURCES += \
    systray.qrc



