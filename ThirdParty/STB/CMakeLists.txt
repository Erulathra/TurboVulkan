project(STB VERSION 1.0.0)

add_library(${PROJECT_NAME} Private/Implementations.cpp)
target_include_directories(${PROJECT_NAME} PUBLIC Public)

setup_project_directory(${PROJECT_NAME})

disable_rtti(${PROJECT_NAME})
disable_exceptions(${PROJECT_NAME})
