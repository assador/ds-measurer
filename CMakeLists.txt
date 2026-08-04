cmake_minimum_required(VERSION 3.20)
project(ds-measurer LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK4 REQUIRED gtk4)
pkg_check_modules(LAYER_SHELL REQUIRED gtk4-layer-shell-0)

find_package(yaml-cpp REQUIRED)

set(SOURCES
	src/config/config.cpp
	src/core/app.cpp
	src/core/measurement.cpp
	src/main.cpp
	src/ui/shortcuts.cpp
	src/ui/ui.cpp
	src/ui/window.cpp
	src/vendor/tinyexpr.c
)
add_executable(${PROJECT_NAME} ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}/src
	${GTK4_INCLUDE_DIRS}
	${LAYER_SHELL_INCLUDE_DIRS}
)
target_link_libraries(${PROJECT_NAME} PRIVATE
	${GTK4_LIBRARIES}
	${LAYER_SHELL_LIBRARIES}
	yaml-cpp::yaml-cpp
)

set(CONFIG_SRC "${CMAKE_CURRENT_SOURCE_DIR}/config/ds-measurer.yaml")
set(CONFIG_DST "${CMAKE_CURRENT_BINARY_DIR}/ds-measurer.yaml")

add_custom_command(
	OUTPUT ${CONFIG_DST}
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CONFIG_SRC} ${CONFIG_DST}
    DEPENDS ${CONFIG_SRC}
    COMMENT "Copying updated ds-measurer.yaml to build directory"
)
target_sources(${PROJECT_NAME} PRIVATE ${CONFIG_DST})
