cmake_minimum_required (VERSION 3.23)
project (MicroMachines)
# Include directories
include_directories(
    "${CMAKE_SOURCE_DIR}/src"
    "${CMAKE_SOURCE_DIR}/libs/json"
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

# Detect and add SFML
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake_modules" ${CMAKE_MODULE_PATH})
#Find any version 2.X of SFML
find_package(SFML 2 REQUIRED audio graphics window system)
if(SFML_FOUND)
    include_directories(${SFML_INCLUDE_DIR})
    target_link_libraries(${EXECUTABLE_NAME} sfml-system sfml-window sfml-graphics sfml-audio)
endif()

#Installing targets 
install (TARGETS ${EXECUTABLE_NAME} DESTINATION bin)

