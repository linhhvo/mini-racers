cmake_minimum_required (VERSION 3.22)
project (MicroMachines)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include directories
include_directories(
    "${CMAKE_SOURCE_DIR}/src"
    "${CMAKE_SOURCE_DIR}/libs/json"
    "${CMAKE_SOURCE_DIR}/libs/tmxlite/tmxlite/include"
)
# Set the name of the executable to be produced
set(EXECUTABLE_NAME MicroMachines)

# Add all .cpp - files under src/ directory
file(GLOB_RECURSE  SOURCES src/*.cpp)
add_executable(${EXECUTABLE_NAME} ${SOURCES})
set(CMAKE_BUILD_TYPE Debug)

# Find Box2D library
add_subdirectory(libs/box2d)
target_link_libraries(${EXECUTABLE_NAME} box2d)

# Add tmxlite library
add_subdirectory(libs/tmxlite/tmxlite)
target_link_libraries(${EXECUTABLE_NAME} tmxlite)

# Add SFML 2
add_subdirectory(libs/SFML)
target_link_libraries(${EXECUTABLE_NAME} sfml-system sfml-window sfml-graphics sfml-audio)

#Installing targets 
install (TARGETS ${EXECUTABLE_NAME} DESTINATION bin)

