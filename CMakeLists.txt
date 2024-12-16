cmake_minimum_required(VERSION 3.28)
project(NESlidesEditor)

set(CMAKE_CXX_STANDARD 20)

include(ExternalProject)


set (CC65_DIR ${CMAKE_BINARY_DIR}/cc65)
set (NES_PROJ_DIR ${CMAKE_BINARY_DIR}/neslides)
set (MAKE_DIR ${CMAKE_BINARY_DIR}/make)

if (WIN32)
    set (MAKE_BUILD_COMMAND ${MAKE_DIR}/build_w32.bat)
    set (MAKE_CONFIGURE_COMMAND "")
else ()
    set (MAKE_BUILD_COMMAND ${MAKE_DIR}/build.sh)
    set (MAKE_CONFIGURE_COMMAND ./configure)
endif ()

ExternalProject_Add(
        cc65
        GIT_REPOSITORY https://github.com/cc65/cc65.git
        GIT_TAG 852b622c433f71f815f005e7e2d3e0409ca81e28
        SOURCE_DIR ${CC65_DIR}
        CONFIGURE_COMMAND ""
        BUILD_COMMAND make -C ${CC65_DIR} bin
        INSTALL_COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_BINARY_DIR}/cc65/bin ${CMAKE_BINARY_DIR}/bin
)

ExternalProject_Add(
        gnu_make
        URL https://ftp.gnu.org/gnu/make/make-4.4.tar.gz
        SOURCE_DIR ${MAKE_DIR}
        BINARY_DIR ${MAKE_DIR}
        CONFIGURE_COMMAND ${MAKE_CONFIGURE_COMMAND}
        BUILD_COMMAND ${MAKE_BUILD_COMMAND}
        INSTALL_COMMAND ${CMAKE_COMMAND} -E copy ${MAKE_DIR}/make ${CMAKE_BINARY_DIR}/bin
)

add_executable(${CMAKE_PROJECT_NAME} main.cpp)

set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/shippable)
install(TARGETS ${PROJECT_NAME} DESTINATION .)
install(DIRECTORY ${CMAKE_BINARY_DIR}/bin/ DESTINATION bin)

add_dependencies(${PROJECT_NAME} cc65)

add_custom_target(nes_proj ALL
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/neslides ${NES_PROJ_DIR}
)

add_custom_target(shippable ALL
        COMMAND ${CMAKE_COMMAND} --install ${CMAKE_BINARY_DIR} --prefix ${CMAKE_INSTALL_PREFIX}
                && ${CMAKE_COMMAND} -E copy_directory ${NES_PROJ_DIR} ${CMAKE_INSTALL_PREFIX}/neslides
        DEPENDS ${PROJECT_NAME} cc65 nes_proj gnu_make
)