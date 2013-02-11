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

target_link_libraries(imago_console imago)

INSTALL(TARGETS imago_console DESTINATION . COMPONENT console)

# Add test to evaluate images from the examples directory
add_test(imago_console_test ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/imago_console "-test" "${Imago_SOURCE_DIR}/examples")
set_tests_properties (imago_console_test PROPERTIES PASS_REGULAR_EXPRESSION "Result: 10 vaild, 10 ok, 100 average score")
