if(SYSTEM_NAME STREQUAL "Win")
	include(BuildOptionsWin)
elseif(SYSTEM_NAME STREQUAL "Linux")
	include(BuildOptionsLinux)
elseif(SYSTEM_NAME STREQUAL "Mac")
	include(BuildOptionsMac)
endif()

if(NOT LIBS_LOCATION)
	message(FATAL_ERROR "Please set LIBS_LOCATION variable")
endif()
set(THIRD_PARTY_DIR ${CMAKE_SOURCE_DIR}/third_party)

include(Boost)
include(OpenCV)
include(Indigo)

set(CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS_SKIP TRUE)
include(InstallRequiredSystemLibraries)
