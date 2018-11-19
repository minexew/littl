cmake_minimum_required(VERSION 3.1)
project(littl)

# Also add headers so that they're included in generated projects
file(GLOB sources
    ${PROJECT_SOURCE_DIR}/littl-extra/*.cpp

    ${PROJECT_SOURCE_DIR}/littl/*.hpp
)

add_library(${PROJECT_NAME} STATIC ${sources})

target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR})
target_compile_features(${PROJECT_NAME} PUBLIC cxx_static_assert)

if (WIN32)
	target_compile_definitions(${PROJECT_NAME} PUBLIC -D_CRT_SECURE_NO_WARNINGS=1)
	target_compile_definitions(${PROJECT_NAME} PRIVATE -D_WINSOCK_DEPRECATED_NO_WARNINGS=1)
	target_link_libraries(${PROJECT_NAME} ws2_32)
else()
	target_link_libraries(${PROJECT_NAME} pthread)
endif()
