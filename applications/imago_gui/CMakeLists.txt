cmake_minimum_required(VERSION 2.8.6)

project(ImagoGUI)

include(../../imago-version.cmake)

install(FILES "build/imago-gui.exe" DESTINATION "bin" COMPONENT bin)

install(FILES "../../dist/imago_console.exe" DESTINATION "bin" COMPONENT console)


FILE(GLOB EXAMPLES "${CMAKE_CURRENT_SOURCE_DIR}/../../examples/*.*")
foreach(f ${EXAMPLES})
	install(FILES "${f}" DESTINATION "examples" COMPONENT examples)
endforeach()

set(CPACK_PACKAGE_VENDOR "GGA Software Services LLC")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Imago GUI")
set(CPACK_PACKAGE_VERSION ${IMAGO_VERSION})
set(CPACK_PACKAGE_VERSION_MAJOR "2")
set(CPACK_PACKAGE_VERSION_MINOR "0")
set(CPACK_PACKAGE_VERSION_PATCH "0")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "")
set(CPACK_PACKAGE_EXECUTABLES "imago-gui;ImagoGUI")
set(CPACK_COMPONENTS_ALL_IN_ONE_PACKAGE 1)
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/../../LICENSE.GPL")
set(CPACK_PACKAGE_FILE_NAME "imago-gui-${IMAGO_VERSION}-installer-win") 
set(CPACK_NSIS_PACKAGE_NAME "ImagoGUI")
set(CPACK_NSIS_MUI_ICON "${CMAKE_CURRENT_SOURCE_DIR}/resources/imago-icon.ico")
set(CPACK_PACKAGE_NAME "ImagoGUI")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "ImagoGUI")

set(CPACK_COMPONENTS_ALL bin examples console)

set(CPACK_COMPONENT_CONSOLE_DISPLAY_NAME "Imago Console Application")
set(CPACK_COMPONENT_CONSOLE_DESCRIPTION "Imago chemical compounds OCR console application")

set(CPACK_COMPONENT_BIN_DISPLAY_NAME "ImagoGUI Application")
set(CPACK_COMPONENT_BIN_DESCRIPTION "ImagoGUI chemical compounds OCR application")

set(CPACK_COMPONENT_EXAMPLES_DISPLAY_NAME "Examples")
set(CPACK_COMPONENT_EXAMPLES_DESCRIPTION "Example pictures and compounds")

#set(CPACK_COMPONENT_BIN_REQUIRED 1)

include(CPack)

