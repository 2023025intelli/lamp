cmake_minimum_required(VERSION 3.20)
project(lamp C)

set(CMAKE_C_STANDARD 99)

set(SRC main.c lamp.h playlistitem.c playlistitem.h utils.c utils.h playback.c playback.h constants.h data_types.h)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(PkgConfig REQUIRED)
find_package(Threads REQUIRED)
find_package(SDL2 REQUIRED)

pkg_check_modules(
        LIBAV REQUIRED IMPORTED_TARGET
        libavdevice
        libavfilter
        libavformat
        libavcodec
        libswresample
        libswscale
        libavutil
)

find_program(GLIB_COMPILE_RESOURCES NAMES glib-compile-resources REQUIRED)

set(GRESOURCE_C resources.c)
set(GRESOURCE_XML resources.xml)
set(GRESOURCE_DEPENDENCIES)

execute_process(
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMAND ${GLIB_COMPILE_RESOURCES} --generate-dependencies ${GRESOURCE_XML}
        OUTPUT_VARIABLE GRESOURCE_DEPENDENCIES
)

add_custom_command(
        OUTPUT ${GRESOURCE_C}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMAND ${GLIB_COMPILE_RESOURCES}
        ARGS
        --target=${CMAKE_CURRENT_BINARY_DIR}/${GRESOURCE_C} --generate-source
        ${GRESOURCE_XML}
        VERBATIM
        MAIN_DEPENDENCY ${GRESOURCE_XML}
        DEPENDS ${GRESOURCE_DEPENDENCIES}
)

add_custom_target(
        resources
        DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${GRESOURCE_C}
)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK4 REQUIRED gtk4)

include_directories(${GTK4_INCLUDE_DIRS})
include_directories(${SDL2_INCLUDE_DIRS})
include_directories(${LIBAV_INCLUDE_DIRS})

link_directories(${LIBAV_LIBRARY_DIRS})
link_directories(${GTK4_LIBRARY_DIRS})

add_executable(${PROJECT_NAME} ${SRC} ${CMAKE_CURRENT_BINARY_DIR}/${GRESOURCE_C})

set_source_files_properties(
        ${CMAKE_CURRENT_BINARY_DIR}/${GRESOURCE_C}
        PROPERTIES GENERATED TRUE
)

add_dependencies(${PROJECT_NAME} resources)

add_definitions(${GTK4_CFLAGS_OTHER})
target_link_libraries(${PROJECT_NAME} ${GTK4_LIBRARIES})
target_link_libraries(${PROJECT_NAME} ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(${PROJECT_NAME} ${CMAKE_DL_LIBS})
target_link_libraries(${PROJECT_NAME} ${LIBAV_LIBRARIES})
target_link_libraries(${PROJECT_NAME} ${SDL2_LIBRARIES})
target_link_libraries(${PROJECT_NAME} m)

# copy res data to build
add_custom_command(TARGET ${PROJECT_NAME} PRE_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/res/ ${CMAKE_BINARY_DIR}/res/)