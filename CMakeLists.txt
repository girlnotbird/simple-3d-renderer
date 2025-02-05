cmake_minimum_required(VERSION 3.30)
project(TheBadDecadeGAME)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Source files & headers
file(GLOB_RECURSE TBDGAME_SOURCES CONFIGURE_DEPENDS ${CMAKE_SOURCE_DIR}/src/*.cpp)
file(GLOB_RECURSE TBDGAME_HEADERS CONFIGURE_DEPENDS ${CMAKE_SOURCE_DIR}/include/*.hpp)

# set the output directory for built objects.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/build")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/build")

add_subdirectory(${CMAKE_SOURCE_DIR}/libs/SDL3 EXCLUDE_FROM_ALL)
add_subdirectory(${CMAKE_SOURCE_DIR}/libs/GLM EXCLUDE_FROM_ALL)

find_package(Curses REQUIRED)

add_executable(Application
        ${TBDGAME_SOURCES}
        ${TBDGAME_HEADERS}
)
target_link_libraries(Application
        PRIVATE SDL3::SDL3
        PRIVATE glm::glm
        PRIVATE ${CURSES_LIBRARIES}
)
target_include_directories(Application
        PUBLIC ${CMAKE_SOURCE_DIR}/src
        PUBLIC ${CMAKE_SOURCE_DIR}/include
        PUBLIC ${CMAKE_SOURCE_DIR}/libs/SDL3/include
        PUBLIC ${CMAKE_SOURCE_DIR}/libs/GLM
        PUBLIC ${CURSES_INCLUDE_DIR}
)
