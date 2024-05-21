set(TARGET_NAME jsonipc)

project(${TARGET_NAME}-root LANGUAGES CXX)

add_subdirectory(lib)

add_custom_target(${PROJECT_NAME}
    SOURCES
        .gitignore
        AUTHORS
        LICENSE
        README.md
)
