option(USE_SYSTEM_INDIGO "Use system Indigo libs or download prebuilt from the server" OFF)


if(NOT USE_SYSTEM_INDIGO)
	set(INDIGO_DIR               "${THIRD_PARTY_DIR}/indigo")
	set(INDIGO_INCLUDE_DIR       "${INDIGO_DIR}/indigo")
	set(INDIGO_LIBS_RELEASE_DIR  "${INDIGO_DIR}/libs_release/${SYSTEM_NAME}/${SUBSYSTEM_NAME}")
	set(INDIGO_LIBS_DEBUG_DIR    "${INDIGO_DIR}/libs_debug/${SYSTEM_NAME}/${SUBSYSTEM_NAME}")

	set(INDIGO_LIBS_FILENAME "indigo-libs-${SYSTEM_NAME}-${SUBSYSTEM_NAME}.zip")

	set(INDIGO_LIBS_LOCATION     "${LIBS_LOCATION}/indigo/c_static/")
	
	if(NOT EXISTS ${INDIGO_LIBS_RELEASE_DIR})
		if(NOT EXISTS ${THIRD_PARTY_DIR}/${INDIGO_LIBS_FILENAME})
			message("Downloading Indigo libs from ${INDIGO_LIBS_LOCATION}/${INDIGO_LIBS_FILENAME}")
			file(DOWNLOAD ${INDIGO_LIBS_LOCATION}/${INDIGO_LIBS_FILENAME}
			              ${THIRD_PARTY_DIR}/${INDIGO_LIBS_FILENAME} 
			              STATUS indigo_libs_status SHOW_PROGRESS)
		endif()
		execute_process(COMMAND ${CMAKE_COMMAND} -E make_directory ${INDIGO_DIR})
		execute_process(COMMAND ${CMAKE_COMMAND} -E tar xf ${THIRD_PARTY_DIR}/${INDIGO_LIBS_FILENAME}
		                WORKING_DIRECTORY ${INDIGO_DIR})
	endif()

	include_directories(${INDIGO_INCLUDE_DIR})
	file(GLOB INDIGO_LIBS_RELEASE "${INDIGO_LIBS_RELEASE_DIR}/*${CMAKE_STATIC_LIBRARY_SUFFIX}")
	file(GLOB INDIGO_LIBS_DEBUG "${INDIGO_LIBS_DEBUG_DIR}/*${CMAKE_STATIC_LIBRARY_SUFFIX}")

	foreach(lib ${INDIGO_LIBS_DEBUG})
		set(Indigo_LIBRARIES ${Indigo_LIBRARIES} debug ${lib})
	endforeach()
	foreach(lib ${INDIGO_LIBS_RELEASE})
		set(Indigo_LIBRARIES ${Indigo_LIBRARIES} optimized ${lib})
	endforeach()

	# To avoid problem with library order we specify them twice
	set(Indigo_LIBRARIES ${Indigo_LIBRARIES} ${Indigo_LIBRARIES})
else()
	message(FATAL_ERROR "Not implemented yet!")
endif()
