cmake_minimum_required(VERSION 2.8.4)
project(churny)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")
aux_source_directory(./src SOURCE_FILES)
find_package(libgit2 REQUIRED)
include_directories(${LIBGIT2_INCLUDE_DIR})
set(LIBS ${LIBS} ${LIBGIT2_LIBRARIES})
add_executable(${PROJECT_NAME} ${SOURCE_FILES})
target_link_libraries(${PROJECT_NAME} ${LIBS})

add_compile_options(-Wall -Wextra -pedantic -Werror)
