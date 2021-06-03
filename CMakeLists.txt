cmake_minimum_required(VERSION 3.17)
project(lamp C)

set(CMAKE_C_STANDARD 99)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

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
pkg_check_modules(GTK3 REQUIRED gtk+-3.0)
pkg_check_modules(LIBAO REQUIRED ao)
pkg_check_modules(MPG123 REQUIRED libmpg123)

include_directories(${GTK3_INCLUDE_DIRS})
link_directories(${GTK3_LIBRARY_DIRS})
link_directories(${LIBAO_LIBRARY_DIRS})
link_directories(${MPG123_LIBRARY_DIRS})

set(SOURCE_FILES main.c lamp.h)

add_executable(lamp ${SOURCE_FILES} ${CMAKE_CURRENT_BINARY_DIR}/${GRESOURCE_C})

set_source_files_properties(
        ${CMAKE_CURRENT_BINARY_DIR}/${GRESOURCE_C}
        PROPERTIES GENERATED TRUE
)

add_dependencies(lamp resources)

add_definitions(${GTK3_CFLAGS_OTHER})
target_link_libraries(lamp ${GTK3_LIBRARIES})
target_link_libraries(lamp ${LIBAO_LIBRARIES})
target_link_libraries(lamp ${MPG123_LIBRARIES})
target_link_libraries(lamp ${CMAKE_THREAD_LIBS_INIT})

# copy glade data to build
add_custom_command(TARGET lamp PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/glade/ ${CMAKE_BINARY_DIR}/glade/
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/res/ ${CMAKE_BINARY_DIR}/res/)