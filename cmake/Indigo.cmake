option(USE_SYSTEM_INDIGO "Use system Indigo libs or download prebuilt from the server" OFF)

if(NOT USE_SYSTEM_INDIGO)
	set(INDIGO_INCLUDE_DIR      "${THIRD_PARTY_DIR}/indigo/indigo_headers")
	set(INDIGO_LIBS_DIR         "${THIRD_PARTY_DIR}/indigo/${SYSTEM_NAME}/${SUBSYSTEM_NAME}")
	set(INDIGO_LIBS_SUBDIR      "${INDIGO_LIBS_DIR}/indigo_libs_${SYSTEM_NAME}_${SUBSYSTEM_NAME}")
	set(INDIGO_LIBS_SUBDIR_DBG  "${INDIGO_LIBS_DIR}/indigo_libs_${SYSTEM_NAME}_${SUBSYSTEM_NAME}_dbg")

	set(INDIGO_HEADERS_FILENAME "indigo_headers.zip")
	set(INDIGO_LIBS_FILENAME    "indigo_libs_${SYSTEM_NAME}_${SUBSYSTEM_NAME}.zip")

	if(NOT EXISTS ${INDIGO_INCLUDE_DIR})
		if(NOT EXISTS ${THIRD_PARTY_DIR}/${INDIGO_HEADERS_FILENAME})
			message("Downloading Indigo headers")
			file(DOWNLOAD ${LIBS_LOCATION}/${INDIGO_HEADERS_FILENAME}
					      ${THIRD_PARTY_DIR}/${INDIGO_HEADERS_FILENAME} 
					      STATUS indigo_headers_status SHOW_PROGRESS)
		endif()
		execute_process(COMMAND ${CMAKE_COMMAND} -E make_directory ${INDIGO_INCLUDE_DIR})
		execute_process(COMMAND ${CMAKE_COMMAND} -E tar xf ${THIRD_PARTY_DIR}/${INDIGO_HEADERS_FILENAME}
						WORKING_DIRECTORY ${INDIGO_INCLUDE_DIR})
	endif()

	if(NOT EXISTS ${INDIGO_LIBS_DIR})
		if(NOT EXISTS ${THIRD_PARTY_DIR}/${INDIGO_LIBS_FILENAME})
			message("Downloading Indigo libs")
			file(DOWNLOAD ${LIBS_LOCATION}/${INDIGO_LIBS_FILENAME}
					      ${THIRD_PARTY_DIR}/${INDIGO_LIBS_FILENAME} 
					      STATUS indigo_libs_status SHOW_PROGRESS)
		endif()
		execute_process(COMMAND ${CMAKE_COMMAND} -E make_directory ${INDIGO_LIBS_DIR})
		execute_process(COMMAND ${CMAKE_COMMAND} -E tar xf ${THIRD_PARTY_DIR}/${INDIGO_LIBS_FILENAME}
						WORKING_DIRECTORY ${INDIGO_LIBS_DIR})
	endif()

	include_directories(${INDIGO_INCLUDE_DIR})
	if(NOT UNIX OR APPLE)
		file(GLOB INDIGO_LIBS "${INDIGO_LIBS_SUBDIR}/*${CMAKE_STATIC_LIBRARY_SUFFIX}")
		file(GLOB INDIGO_LIBS_DBG "${INDIGO_LIBS_SUBDIR_DBG}/*${CMAKE_STATIC_LIBRARY_SUFFIX}")
	else()
		set(INDIGO_LIBS 
			${INDIGO_LIBS_SUBDIR}/librender2d.a
			${INDIGO_LIBS_SUBDIR}/libindigo-static.a
			${INDIGO_LIBS_SUBDIR}/libindigo-renderer-static.a
			${INDIGO_LIBS_SUBDIR}/libcommon.a 
			${INDIGO_LIBS_SUBDIR}/libmolecule.a  
			${INDIGO_LIBS_SUBDIR}/libreaction.a
			${INDIGO_LIBS_SUBDIR}/liblayout.a 
			${INDIGO_LIBS_SUBDIR}/libz.a 
			${INDIGO_LIBS_SUBDIR}/libtinyxml.a 
			${INDIGO_LIBS_SUBDIR}/libgraph.a
			${INDIGO_LIBS_SUBDIR}/libcairo.a
			${INDIGO_LIBS_SUBDIR}/libpixman.a
			${INDIGO_LIBS_SUBDIR}/libpng.a)
	endif()
else()
	message(FATAL_ERROR "Not implemented yet!")
endif()
