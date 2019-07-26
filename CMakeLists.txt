cmake_minimum_required(VERSION 3.8)
project(pog
	LANGUAGES CXX
	VERSION 0.1.0
)

## Module path for custom finders.
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

## Options.
option(POG_DOWNLOAD_RE2 "Download RE2 during build process" OFF)
option(POG_EXAMPLES     "Build examples"                    OFF)
option(POG_TESTS        "Build tests"                       OFF)

## Includes.
include(CMakePackageConfigHelpers)
include(GNUInstallDirs)

## Require C++17 support.
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

## Variables.
set(POG_INCLUDE_DIR          "${CMAKE_CURRENT_SOURCE_DIR}/include")
set(POG_INSTALL_CMAKE_DIR    "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_DATADIR}/cmake/pog")
set(POG_INSTALL_LIB_DIR      "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}")
set(POG_INSTALL_INCLUDE_DIR  "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_INCLUDEDIR}")
set(POG_CMAKE_CONFIG_FILE_IN "${CMAKE_CURRENT_SOURCE_DIR}/share/pog-config.cmake.in")
set(POG_CMAKE_CONFIG_FILE    "${CMAKE_CURRENT_BINARY_DIR}/share/pog-config.cmake")
set(POG_CMAKE_VERSION_FILE   "${CMAKE_CURRENT_BINARY_DIR}/share/pog-config-version.cmake")
set(POG_PC_CONFIG_FILE_IN    "${CMAKE_CURRENT_SOURCE_DIR}/share/pog.pc.in")
set(POG_PC_CONFIG_FILE       "${CMAKE_CURRENT_BINARY_DIR}/share/pog.pc")
set(POG_CMAKE_INSTALL_FILES  "${POG_CMAKE_CONFIG_FILE}" "${POG_CMAKE_VERSION_FILE}")
set(POG_PC_INSTALL_FILES     "${POG_PC_CONFIG_FILE}")

## Requirements
### Threads - because of RE2
### fmt - does not contain reporting thus we report manually
find_package(Threads REQUIRED)
find_package(fmt REQUIRED)
if(fmt_FOUND)
	message(STATUS "Found libfmt")
endif()

## If RE2 is not being downloaded, try to find it using find_package().
if(NOT POG_DOWNLOAD_RE2)
	find_package(re2 REQUIRED)
	if(RE2_FOUND)
		message(STATUS "RE2 include directory: ${RE2_INCLUDE_DIR}")
		message(STATUS "RE2 library: ${RE2_LIBRARY}")
	endif()
	## Install our custom finder in case user system does not contain one.
	list(APPEND POG_CMAKE_INSTALL_FILES "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Findre2.cmake")
else()
	add_subdirectory(deps)
endif()

## Library.
add_library(pog INTERFACE)
target_sources(pog INTERFACE ${POG_FILES})
target_include_directories(pog INTERFACE
	"$<BUILD_INTERFACE:${POG_INCLUDE_DIR}>"
	"$<INSTALL_INTERFACE:${POG_INSTALL_INCLUDE_DIR}>"
)
target_link_libraries(pog INTERFACE re2::re2 fmt::fmt)

## Build examples if requested.
if(POG_EXAMPLES)
	add_subdirectory(examples)
endif()

## We need to set custom RE2 include directory and library path if we were one who downloaded it.
## Also we need to set proper requirements for pkg-config file.
if(POG_DOWNLOAD_RE2)
	set(RE2_INCLUDE_DIR "${POG_INSTALL_INCLUDE_DIR}/pog")
	set(RE2_LIBRARY "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}/libpog_re2.a")
	set(POG_PC_REQUIREMENT "Libs: -L\$\{libdir\} -lpog_re2 -lpthread -lfmt")
else()
	set(RE2_INCLUDE_DIR "<NOT_SET>")
	set(RE2_LIBRARY "<NOT_SET>")
	set(POG_PC_REQUIREMENT "Requires: re2\nLibs: -L\$\{libdir\} -lfmt")
endif()

## Create pog-config.cmake
configure_package_config_file(
	${POG_CMAKE_CONFIG_FILE_IN}
	${POG_CMAKE_CONFIG_FILE}
	INSTALL_DESTINATION ${POG_INSTALL_CMAKE_DIR}
	PATH_VARS
		RE2_INCLUDE_DIR RE2_LIBRARY
)

## Create pog-config-version.cmake
write_basic_package_version_file(
	${POG_CMAKE_VERSION_FILE}
	VERSION ${PROJECT_VERSION}
	COMPATIBILITY AnyNewerVersion
)

## Create pog.pc
configure_file(
	${POG_PC_CONFIG_FILE_IN}
	${POG_PC_CONFIG_FILE}
	@ONLY
)

## Install.
### Include directory.
install(
	DIRECTORY ${POG_INCLUDE_DIR}/pog
	DESTINATION ${POG_INSTALL_INCLUDE_DIR}
)

### Export pog::pog target into pog-targets.cmake
install(TARGETS pog EXPORT pog)
install(
	EXPORT pog
	FILE pog-targets.cmake
	NAMESPACE pog::
	DESTINATION ${POG_INSTALL_CMAKE_DIR}
)

### Install CMake related files (cmake-config.cmake, cmake-config-version.cmake)
install(
	FILES ${POG_CMAKE_INSTALL_FILES}
	DESTINATION ${POG_INSTALL_CMAKE_DIR}
)

### Install pkg-config related files (pog.pc)
install(
	FILES ${POG_PC_CONFIG_FILE}
	DESTINATION ${POG_INSTALL_LIB_DIR}/pkgconfig
)
