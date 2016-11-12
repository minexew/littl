cmake_minimum_required(VERSION 3.1)
project(littl)

set(CMAKE_CXX_STANDARD 14)
set(library ${PROJECT_NAME})

# Also add headers so that they're included in generated projects
file(GLOB sources
    ${PROJECT_SOURCE_DIR}/littl-extra/*.cpp
    ${PROJECT_SOURCE_DIR}/src/bleb/*.hpp

    ${PROJECT_SOURCE_DIR}/littl/*.hpp
    ${PROJECT_SOURCE_DIR}/littl+bleb/*.hpp
    ${PROJECT_SOURCE_DIR}/littl+reflector/*.hpp
)

if (WIN32)
	add_definitions(-D_CRT_SECURE_NO_WARNINGS=1)
	add_definitions(-D_WINSOCK_DEPRECATED_NO_WARNINGS=1)
endif()

add_library(${library} ${sources})

target_include_directories(${library} PUBLIC ${PROJECT_SOURCE_DIR})

if (WIN32)
	target_link_libraries(${library} ws2_32)
endif()