
QT       -= core gui
CONFIG   -= console

TARGET = Daemon
TEMPLATE = app

MAIN_APP_TARGET_DESTDIR = SmartGateway/bin

DebugBuild {
    TARGET = $${TARGET}d
    DESTDIR  = $${PWD}/../$${MAIN_APP_TARGET_DESTDIR}/debug
} else {
    DESTDIR  = $${PWD}/../$${MAIN_APP_TARGET_DESTDIR}/release
}


win32 {
    !equals(QT_MAJOR_VERSION, 5) | greaterThan(QT_MINOR_VERSION, 6) {
       error("Unsupported Qt version, 5.6.x or lesser is required")
    } else {
       message(the Qt version is $$[QT_VERSION])
    }

SOURCES += \
    mian.cpp
}


