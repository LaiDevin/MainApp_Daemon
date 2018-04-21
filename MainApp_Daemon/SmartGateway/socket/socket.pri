INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD
DEFINES += HAVE_CONFIG_H

SOURCES += \
		$$PWD/TcpSocket.cpp \
        $$PWD/UdpSocket.cpp


HEADERS += \
        $$PWD/Socket.h \
		$$PWD/TcpSocket.h \
		$$PWD/UdpSocket.h 
