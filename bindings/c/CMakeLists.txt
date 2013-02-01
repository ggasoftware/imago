project(ImagoC)

include_directories(${ImagoLib_SOURCE_DIR}/src)

include_directories(${Boost_INCLUDE_DIR})
include_directories(${INDIGO_INCLUDE_DIR})
include_directories(${OpenCV_INCLUDE_DIRS})

message(STATUS "Imago version is ${IMAGO_VERSION} (${IMAGO_VERSION_EXT})")
configure_file(src/imago_version.h.in ${CMAKE_CURRENT_SOURCE_DIR}/src/imago_version.h)

file(GLOB SRC src/*.cpp)
file(GLOB HEADERS src/*.h)
list(APPEND SRC ${HEADERS})

#SHARED
add_library(imago_c_shared SHARED ${SRC})
add_dependencies(imago_c_shared imago)
target_link_libraries(imago_c_shared imago)
set_target_properties(imago_c_shared PROPERTIES
	OUTPUT_NAME imago_c
	LIBRARY_OUTPUT_DIRECTORY imago_shared)

install(TARGETS imago_c_shared EXPORT imago DESTINATION . COMPONENT shared)
install(FILES src/imago_c.h DESTINATION . RENAME imago.h  COMPONENT shared)
install(FILES ${CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS} DESTINATION . COMPONENT shared)

#STATIC
add_library(imago_c_static STATIC ${SRC})
add_dependencies(imago_c_static imago)
target_link_libraries(imago_c_static imago)

set_target_properties(imago_c_static PROPERTIES 
	OUTPUT_NAME imago_c
	ARCHIVE_OUTPUT_DIRECTORY imago_static)

install(TARGETS imago_c_static EXPORT imago DESTINATION . COMPONENT static)
install(FILES src/imago_c.h DESTINATION . RENAME imago.h COMPONENT static)
IF(MSVC)
  INSTALL (FILES ${PROJECT_BINARY_DIR}/Debug/imago_c.pdb DESTINATION . COMPONENT static CONFIGURATIONS Debug)
ENDIF(MSVC)
