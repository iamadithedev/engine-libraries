project(stb)

add_library(${PROJECT_NAME} INTERFACE)

target_include_directories(${PROJECT_NAME} INTERFACE .)
target_compile_definitions(${PROJECT_NAME} INTERFACE STB_IMAGE_IMPLEMENTATION)