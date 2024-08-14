cmake_minimum_required(VERSION 3.10)

project(coro_actor_client)

set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g -ggdb -Wall")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -Wno-unused -Wno-sign-compare -Wno-unused-variable")

add_library(coro_actor_client SHARED
    src/coro_actor_client.cpp
)

target_include_directories(coro_actor_client PUBLIC
    ${PROJECT_SOURCE_DIR}/third-party/asio-1.30.2/include
    ${PROJECT_SOURCE_DIR}/include
)

target_link_libraries(coro_actor_client PUBLIC
    pthread
)

if (ENABLE_TEST)
    add_executable(echo example/echo.cpp)
    target_link_libraries(echo PUBLIC coro_actor_client)
endif()

include(GNUInstallDirs)

install(TARGETS coro_actor_client
    LIBRARY
        DESTINATION lib
    RUNTIME
        DESTINATION bin
)

install(DIRECTORY ${PROJECT_SOURCE_DIR}/include/ DESTINATION include/)