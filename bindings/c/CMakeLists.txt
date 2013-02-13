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

# Shared library
add_library(imago_c_shared SHARED ${SRC})
target_link_libraries(imago_c_shared imago)
set_target_properties(imago_c_shared PROPERTIES
	OUTPUT_NAME imago_c
	LIBRARY_OUTPUT_DIRECTORY imago_shared
	LINK_INTERFACE_LIBRARIES "")

if(UNIX AND NOT APPLE)
	# Additional check that there are no undefined symbols in the shared library 
	target_link_libraries(imago_c_shared "-Wl,-no-undefined")
endif()

install(TARGETS imago_c_shared EXPORT imago DESTINATION shared/${SUBSYSTEM_PATH} COMPONENT shared)
install(FILES src/imago_c.h DESTINATION . RENAME imago.h  COMPONENT shared)
install(FILES ${CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS} DESTINATION shared/${SUBSYSTEM_PATH} COMPONENT shared)

# Static library
add_library(imago_c_static STATIC ${SRC})
target_link_libraries(imago_c_static imago)

set_target_properties(imago_c_static PROPERTIES 
	OUTPUT_NAME imago_c
	ARCHIVE_OUTPUT_DIRECTORY imago_static)

install(TARGETS imago_c_static EXPORT imago DESTINATION static/${SUBSYSTEM_PATH} COMPONENT static)
install(FILES src/imago_c.h DESTINATION . RENAME imago.h COMPONENT static)
IF(MSVC)
	INSTALL (FILES ${PROJECT_BINARY_DIR}/Debug/imago_c.pdb DESTINATION static/${SUBSYSTEM_PATH} COMPONENT static CONFIGURATIONS Debug)
ENDIF(MSVC)

# Test project for static and shared library
include_directories(${ImagoC_SOURCE_DIR}/src)
add_executable(imago_c_test_static test/imago_c_test.c)
target_link_libraries(imago_c_test_static imago_c_static)

add_executable(imago_c_test_shared test/imago_c_test.c)
target_link_libraries(imago_c_test_shared imago_c_shared)

add_executable(indigo_compare_molecules test/indigo_compare_molecules.c)
target_link_libraries(indigo_compare_molecules ${Indigo_LIBRARIES})
if(UNIX)
	target_link_libraries(indigo_compare_molecules "-lstdc++ -lpthread")
endif()

# CTest tests to verify that recognition procedure works
set(TEST_IMAGE_NAMES
	0000041_rnd   0000191_rnd   0000230_rnd
	0000415_rnd   0000603_rnd   0000931_rnd
	0001018_rnd   0001608_rnd   0002057_rnd
	0002708_rnd)
foreach(img ${TEST_IMAGE_NAMES})
	add_test(imago_c_test_static_${img} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/imago_c_test_static "${Imago_SOURCE_DIR}/examples/${img}.png" "${img}_out_static.mol")
	add_test(imago_c_test_static_verify_${img} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/indigo_compare_molecules "${Imago_SOURCE_DIR}/examples/${img}.mol" "${img}_out_static.mol")

	add_test(imago_c_test_shared_${img} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/imago_c_test_shared "${Imago_SOURCE_DIR}/examples/${img}.png" "${img}_out_shared.mol")
	add_test(imago_c_test_shared_verify_${img} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/indigo_compare_molecules "${Imago_SOURCE_DIR}/examples/${img}.mol" "${img}_out_shared.mol")
endforeach()
