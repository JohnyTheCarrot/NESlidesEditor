cmake_minimum_required(VERSION 3.30)
project(NESlidesEditor)

set(CMAKE_CXX_STANDARD 20)

include(ExternalProject)

ExternalProject_Add(
        cc65
        GIT_REPOSITORY https://github.com/cc65/cc65.git
        GIT_TAG 852b622c433f71f815f005e7e2d3e0409ca81e28
        SOURCE_DIR ${CMAKE_BINARY_DIR}/cc65
        CONFIGURE_COMMAND ""
        BUILD_COMMAND make -C ${CMAKE_BINARY_DIR}/cc65 bin
        INSTALL_COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_BINARY_DIR}/cc65/bin ${CMAKE_BINARY_DIR}/bin
)

add_executable(${CMAKE_PROJECT_NAME} main.cpp)

set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/shippable)
install(TARGETS ${PROJECT_NAME} DESTINATION .)
install(DIRECTORY ${CMAKE_BINARY_DIR}/bin/ DESTINATION bin)

add_dependencies(${PROJECT_NAME} cc65)

add_custom_target(shippable ALL
        COMMAND ${CMAKE_COMMAND} --install ${CMAKE_BINARY_DIR} --prefix ${CMAKE_INSTALL_PREFIX}
        DEPENDS ${PROJECT_NAME} cc65
)