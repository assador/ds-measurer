cmake_minimum_required(VERSION 3.20)
project(ds-measurer LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(ENABLE_WLR_SCREENCOPY "Enable wlr-screencopy protocol support (wlroots)" ON)
option(ENABLE_KDE_SCREENSHOT "Enable org.kde.KWin.ScreenShot2 support (KDE Plasma)" OFF)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK4 REQUIRED gtk4)
pkg_check_modules(LAYER_SHELL REQUIRED gtk4-layer-shell-0)
find_package(yaml-cpp REQUIRED)

set(SOURCES
	src/config/config.cpp
	src/core/app.cpp
	src/core/measurement.cpp
	src/main.cpp
	src/platform/screenshot.cpp
	src/ui/shortcuts.cpp
	src/ui/ui.cpp
	src/ui/window.cpp
	src/vendor/tinyexpr.c
)

set(EXTRA_INCLUDES "")
set(EXTRA_LIBS "")

if (ENABLE_WLR_SCREENCOPY)
	pkg_check_modules(WAYLAND_CLIENT REQUIRED wayland-client)
	find_program(WAYLAND_SCANNER wayland-scanner REQUIRED)

	set(PROTOCOL_XML "${CMAKE_CURRENT_SOURCE_DIR}/protocols/wlr-screencopy-unstable-v1.xml")
	set(GEN_PROTO_HEADER "${CMAKE_CURRENT_BINARY_DIR}/wlr-screencopy-v1-protocol.h")
	set(GEN_PROTO_CODE "${CMAKE_CURRENT_BINARY_DIR}/wlr-screencopy-v1-protocol.c")

	add_custom_command(
		OUTPUT ${GEN_PROTO_HEADER}
		COMMAND ${WAYLAND_SCANNER} client-header ${PROTOCOL_XML} ${GEN_PROTO_HEADER}
		DEPENDS ${PROTOCOL_XML}
	)

	add_custom_command(
		OUTPUT ${GEN_PROTO_CODE}
		COMMAND ${WAYLAND_SCANNER} private-code ${PROTOCOL_XML} ${GEN_PROTO_CODE}
		DEPENDS ${PROTOCOL_XML}
	)

	list(APPEND EXTRA_INCLUDES ${WAYLAND_CLIENT_INCLUDE_DIRS})
	list(APPEND EXTRA_LIBS ${WAYLAND_CLIENT_LIBRARIES})

	list(APPEND SOURCES src/platform/wayland/screenshot.cpp ${GEN_PROTO_CODE} ${GEN_PROTO_HEADER})
	add_compile_definitions(HAS_WLR_SCREENCOPY=1)
endif()

if (ENABLE_KDE_SCREENSHOT)
	pkg_check_modules(GIO REQUIRED gio-2.0 gio-unix-2.0)
	list(APPEND SOURCES src/platform/kde/screenshot.cpp)
	list(APPEND EXTRA_INCLUDES ${GIO_INCLUDE_DIRS})
	list(APPEND EXTRA_LIBS ${GIO_LIBRARIES})
	add_compile_definitions(HAS_KDE_SCREENSHOT=1)
endif()

add_executable(${PROJECT_NAME} ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}/src
	${CMAKE_CURRENT_BINARY_DIR}
	${GTK4_INCLUDE_DIRS}
	${LAYER_SHELL_INCLUDE_DIRS}
	${EXTRA_INCLUDES}
)

target_link_libraries(${PROJECT_NAME} PRIVATE
	${GTK4_LIBRARIES}
	${LAYER_SHELL_LIBRARIES}
	${EXTRA_LIBS}
	yaml-cpp::yaml-cpp
)

if (ENABLE_WLR_SCREENCOPY)
	target_compile_definitions(${PROJECT_NAME} PRIVATE HAS_WLR_SCREENCOPY=1)
endif()

set(CONFIG_SRC "${CMAKE_CURRENT_SOURCE_DIR}/config/ds-measurer.yaml")
set(CONFIG_DST "${CMAKE_CURRENT_BINARY_DIR}/ds-measurer.yaml")

add_custom_command(
	OUTPUT ${CONFIG_DST}
	COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CONFIG_SRC} ${CONFIG_DST}
	DEPENDS ${CONFIG_SRC}
	COMMENT "Copying updated ds-measurer.yaml to build directory"
)
target_sources(${PROJECT_NAME} PRIVATE ${CONFIG_DST})
