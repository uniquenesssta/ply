cmake_minimum_required(VERSION 3.30.5)

project(
    ModularMpvPlayer
    VERSION 0.1.0
    DESCRIPTION "A modular Qt 6 desktop player shell prepared for libmpv integration"
    LANGUAGES CXX
)

option(PLAYER_BUILD_TESTS "Build player tests" ON)

if(PLAYER_BUILD_TESTS)
    include(CTest)
    enable_testing()
endif()

add_subdirectory(cmake)
