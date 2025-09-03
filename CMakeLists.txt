# Forwarding CMakeLists.txt - DO NOT EDIT
# The actual CMake configuration is in cmake/CMakeLists.txt

cmake_minimum_required(VERSION 3.21)

# Parse Sail.toml for project name and version
include(cmake/build/sail_toml.cmake)
sail_parse_toml()

project(${SAIL_PROJECT_NAME} VERSION ${SAIL_PROJECT_VERSION})

include(cmake/CMakeLists.txt)
