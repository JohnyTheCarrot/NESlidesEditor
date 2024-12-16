cmake_minimum_required(VERSION 3.28)
project(NESlidesEditor)

set(CMAKE_CXX_STANDARD 20)

include(ExternalProject)

set (CC65_DIR ${CMAKE_BINARY_DIR}/cc65)
set (NES_PROJ_DIR ${CMAKE_BINARY_DIR}/neslides)
set (MAKE_DIR ${CMAKE_BINARY_DIR}/make)

ExternalProject_Add(
        cc65
        GIT_REPOSITORY https://github.com/cc65/cc65.git
        GIT_TAG 852b622c433f71f815f005e7e2d3e0409ca81e28
        SOURCE_DIR ${CC65_DIR}
        CONFIGURE_COMMAND ""
        BUILD_COMMAND make -C ${CC65_DIR} bin
        INSTALL_COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_BINARY_DIR}/cc65/bin ${CMAKE_BINARY_DIR}/bin
)

set (MAKE_TARGET gnu_make)
set (MAKE_URL https://ftp.gnu.org/gnu/make/make-4.4.tar.gz)

if (WIN32)
    ExternalProject_Add(
            ${MAKE_TARGET}
            URL ${MAKE_URL}
            SOURCE_DIR ${MAKE_DIR}
            BINARY_DIR ${MAKE_DIR}
            CONFIGURE_COMMAND ""
            BUILD_COMMAND ${MAKE_DIR}/build_w32.bat
            INSTALL_COMMAND ${CMAKE_COMMAND} -E copy ${MAKE_DIR}/WinRel/gnumake.exe ${CMAKE_BINARY_DIR}/bin/make.exe
    )
else ()
    ExternalProject_Add(
            ${MAKE_TARGET}
            URL ${MAKE_URL}
            SOURCE_DIR ${MAKE_DIR}
            BINARY_DIR ${MAKE_DIR}
            CONFIGURE_COMMAND ./configure
            BUILD_COMMAND ${MAKE_DIR}/build.sh
            INSTALL_COMMAND ${CMAKE_COMMAND} -E copy ${MAKE_DIR}/make ${CMAKE_BINARY_DIR}/bin
    )
endif ()

add_executable(${CMAKE_PROJECT_NAME} src/main.cpp
        src/subprocess.h)

set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/shippable)
install(TARGETS ${PROJECT_NAME} DESTINATION .)
install(DIRECTORY ${CMAKE_BINARY_DIR}/bin/ DESTINATION bin)

add_custom_target(nes_proj ALL
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/neslides ${NES_PROJ_DIR}
)

add_custom_target(shippable ALL
        COMMAND ${CMAKE_COMMAND} --install ${CMAKE_BINARY_DIR} --prefix ${CMAKE_INSTALL_PREFIX}
                && ${CMAKE_COMMAND} -E copy_directory ${NES_PROJ_DIR} ${CMAKE_INSTALL_PREFIX}/neslides
        DEPENDS ${PROJECT_NAME} gnu_make cc65 nes_proj
)