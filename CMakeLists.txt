cmake_minimum_required(VERSION 3.5)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)

# Only set EXTRA_COMPONENT_DIRS if the directory exists
# set(EXTRA_COMPONENT_DIRS
#     ../../components
#     )

add_compile_options(-fdiagnostics-color=always)

project(stepAI)

