cmake_minimum_required(VERSION 3.30)
project(TheBadDecadeGAME)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Dirs
set(TBDGAME_SOURCE_DIR ${CMAKE_SOURCE_DIR}/src)
set(TBDGAME_HEADER_DIR ${CMAKE_SOURCE_DIR}/include)
set(SDL3_DIR ${CMAKE_SOURCE_DIR}/libs/SDL3)

# Include Directories
include_directories(
        ${TBDGAME_SOURCE_DIR}
        ${TBDGAME_HEADER_DIR}
        ${SDL3_DIR}/include
)

# Source files
file(GLOB_RECURSE TBDGAME_SOURCES CONFIGURE_DEPENDS ${TBDGAME_SOURCE_DIR}/*.cpp)
file(GLOB_RECURSE TBDGAME_HEADERS CONFIGURE_DEPENDS ${TBDGAME_HEADER_DIR}/*.hpp)

# Library Files
file(GLOB SDL_LIBS CONFIGURE_DEPENDS ${SDL3_DIR}/lib/*.dylib)

# set the output directory for built objects.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/build")

add_executable(Application
        ${TBDGAME_SOURCES}
        ${TBDGAME_HEADERS}
)
target_link_libraries(Application
        PRIVATE ${SDL3_LIBS}
)
target_include_directories(Application
        PUBLIC ${TBDGAME_SOURCE_DIR}
        PUBLIC ${TBDGAME_HEADER_DIR}
        PUBLIC ${SDL3_DIR}/include
)
