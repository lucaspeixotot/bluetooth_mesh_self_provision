set(AUTO_GEN_MESSAGE "/* File was auto-generated, do not modify! */")
set(MAJOR 0)
set(MINOR 1)

set(VERSION_FILE "${PROJECT_SOURCE_DIR}/build.version")

file(READ  ${VERSION_FILE} BUILD)
math(EXPR BUILD "${BUILD}+1")
file(WRITE ${VERSION_FILE} "${BUILD}")

configure_file (
        "${PROJECT_SOURCE_DIR}/src/version.h.in"
        "${PROJECT_BINARY_DIR}/include/generated/firmware_version.h"
    )
