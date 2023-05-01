project(stb)

add_library(${PROJECT_NAME})

target_include_directories(${PROJECT_NAME} PUBLIC .)
target_sources(${PROJECT_NAME}            PRIVATE stb_image.cpp)