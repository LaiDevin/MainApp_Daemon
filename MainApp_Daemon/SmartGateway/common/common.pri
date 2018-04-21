INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD
DEFINES += HAVE_CONFIG_H

SOURCES += \
        $$PWD/Tool.cpp \
    $$PWD/config.cpp \
    $$PWD/IniFile.cpp \
    $$PWD/log.cpp \
    $$PWD/crc.cpp


HEADERS += \
        $$PWD/AgreementDefine.h \
	$$PWD/Common.h \
	$$PWD/Tool.h \  
    $$PWD/config.h \
    $$PWD/IniFile.h \
    $$PWD/log.h \
    $$PWD/crc.h

DEFINES += LOG
