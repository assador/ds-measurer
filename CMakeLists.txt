cmake_minimum_required(VERSION 3.20)
project(ds-measurer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK4 REQUIRED gtk4)
pkg_check_modules(LAYER_SHELL REQUIRED gtk4-layer-shell-0)

find_package(yaml-cpp REQUIRED)

add_executable(${PROJECT_NAME}
	src/main.cpp
	src/config.cpp
	src/cursor.cpp
	src/measurement.cpp
	src/ui.cpp
)
target_include_directories(${PROJECT_NAME} PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}/include
	${GTK4_INCLUDE_DIRS}
	${LAYER_SHELL_INCLUDE_DIRS}
)
target_link_libraries(${PROJECT_NAME} PRIVATE
	${GTK4_LIBRARIES}
	${LAYER_SHELL_LIBRARIES}
	yaml-cpp::yaml-cpp
)
