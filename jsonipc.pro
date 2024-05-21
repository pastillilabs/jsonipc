TEMPLATE = subdirs

CONFIG += jsonipc_tests

SUBDIRS = lib

jsonipc_tests {
    SUBDIRS += \
        tests \

    tests.depends = lib
}

OTHER_FILES += \
    .gitignore \
    AUTHORS \
    LICENSE \
    README.md \
