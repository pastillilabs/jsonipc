TEMPLATE = lib

QT = core network
CONFIG += plugin
DEFINES += JSONIPC_LIBRARY

INCLUDEPATH += $$PWD/include

HEADERS += $$files($$PWD/include/*.h)
HEADERS += $$files($$PWD/src/*.h)
SOURCES += $$files($$PWD/src/*.cpp)

OTHER_FILES += \
    .gitignore \
    AUTHORS \
    LICENSE \
    README.md \

android {
    target.path = /libs/$${ANDROID_TARGET_ARCH}
    INSTALLS += target
}

sailfish {
    target.path = /usr/share/$${HARBOUR_NAME}/lib
    INSTALLS += target
}
