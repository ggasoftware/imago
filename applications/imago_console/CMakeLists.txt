project(ImagoConsole)

file(GLOB SRC src/*.cpp)
file(GLOB HEADERS "src/*.h")

list(APPEND SRC ${HEADERS})

include_directories(${ImagoLib_SOURCE_DIR}/src)

add_executable(imago_console ${SRC})

if(UNIX AND NOT APPLE)
	find_package(Freetype REQUIRED)
	target_link_libraries(imago_console freetype fontconfig)
endif()

target_link_libraries(imago_console ${INDIGO_LIBS})
target_link_libraries(imago_console imago)

INSTALL(TARGETS imago_console DESTINATION . COMPONENT console)

