cmake_minimum_required(VERSION 3.5)
project(littl)

set(library ${PROJECT_NAME})

# Also add headers so that they're included in generated projects
file(GLOB sources
    ${PROJECT_SOURCE_DIR}/littl-extra/*.cpp
    ${PROJECT_SOURCE_DIR}/src/bleb/*.hpp

    ${PROJECT_SOURCE_DIR}/littl/*.hpp
    ${PROJECT_SOURCE_DIR}/littl+bleb/*.hpp
    ${PROJECT_SOURCE_DIR}/littl+reflector/*.hpp
)

add_library(${library} ${sources})

target_include_directories(${library} PUBLIC ${PROJECT_SOURCE_DIR})
target_compile_definitions(${library} PUBLIC __STDC_FORMAT_MACROS=1)
target_compile_features(${library} PUBLIC cxx_std_17)

if (WIN32)
	target_compile_definitions(${library} PUBLIC -D_CRT_SECURE_NO_WARNINGS=1)
	target_compile_definitions(${library} PRIVATE -D_WINSOCK_DEPRECATED_NO_WARNINGS=1)
	target_link_libraries(${library} ws2_32)
endif()
