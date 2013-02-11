cmake_minimum_required(VERSION 2.8.6)

project(Imago)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake/)

set(LIBS_LOCATION "http://ggasoftware.com/downloads/repo/" CACHE STRING "URL to directory with imago third party libs")

include(GetSystemVersion)
include(BuildOptions)
include(imago-version.cmake)

message(STATUS "Subsystem: ${SUBSYSTEM_NAME}")

INCLUDE(CTest)                        	

add_subdirectory(imago)
add_subdirectory(applications/imago_console)
add_subdirectory(bindings/c)

set(CPACK_ARCHIVE_COMPONENT_INSTALL 1)

SET(CPACK_COMPONENTS_ALL shared console static)

SET(CPACK_GENERATOR "ZIP")
SET(CPACK_PACKAGE_FILE_NAME "imago-${IMAGO_VERSION}-${PACKAGE_SUFFIX}") 

SET(CMAKE_INSTALL_PREFIX ${Imago_SOURCE_DIR}/dist)

INCLUDE(CPack)
