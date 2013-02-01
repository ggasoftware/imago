option(USE_SYSTEM_BOOST "Use system Boost libs or download prebuilt from the server" OFF)

#Common settings
#set(Boost_DEBUG ON)
set(Boost_ADDITIONAL_VERSIONS "1.49" "1.49.0")
set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)
set(Boost_FIND_REQUIRED ON)

if(NOT USE_SYSTEM_BOOST)
	set(BOOST_INCLUDEDIR "${THIRD_PARTY_DIR}/boost")
	set(BOOST_LIBRARYDIR "${THIRD_PARTY_DIR}/boost/${SYSTEM_NAME}/${SUBSYSTEM_NAME}")
	set(Boost_NO_SYSTEM_PATHS TRUE)

	set(BOOST_HEADERS_FILENAME "boost_headers.zip")
	set(BOOST_LIBS_FILENAME "boost_libs_${SYSTEM_NAME}_${SUBSYSTEM_NAME}.zip")

	if(NOT EXISTS ${BOOST_INCLUDEDIR})
		if(NOT EXISTS ${THIRD_PARTY_DIR}/${BOOST_HEADERS_FILENAME})
			message("Downloading Boost headers")
			file(DOWNLOAD ${LIBS_LOCATION}/${BOOST_HEADERS_FILENAME}
					      ${THIRD_PARTY_DIR}/${BOOST_HEADERS_FILENAME} 
					      STATUS boost_headers_status SHOW_PROGRESS)
			list(GET boost_headers_status 0 boost_header_status)
			if(NOT boost_headers_status EQUAL 0)
				message(FATAL_ERROR "Couldn't download boost headers!")
			endif()
		endif()
		execute_process(COMMAND ${CMAKE_COMMAND} -E make_directory ${BOOST_INCLUDEDIR})
		execute_process(COMMAND ${CMAKE_COMMAND} -E tar xf ${THIRD_PARTY_DIR}/${BOOST_HEADERS_FILENAME}
						WORKING_DIRECTORY ${BOOST_INCLUDEDIR})
	endif()
	if(NOT EXISTS ${BOOST_LIBRARYDIR})
		if(NOT EXISTS ${THIRD_PARTY_DIR}/${BOOST_LIBS_FILENAME})
			message("Downloading boost libs")
			file(DOWNLOAD ${LIBS_LOCATION}/${BOOST_LIBS_FILENAME}
					      ${THIRD_PARTY_DIR}/${BOOST_LIBS_FILENAME} 
					      STATUS boost_libs_status SHOW_PROGRESS)

		endif()
		execute_process(COMMAND ${CMAKE_COMMAND} -E make_directory ${BOOST_LIBRARYDIR})
		execute_process(COMMAND ${CMAKE_COMMAND} -E tar xf ${THIRD_PARTY_DIR}/${BOOST_LIBS_FILENAME}
						WORKING_DIRECTORY ${BOOST_LIBRARYDIR})
	endif()
endif()

find_package(Boost 1.49.0 COMPONENTS thread program_options system filesystem)

include_directories(${Boost_INCLUDE_DIR})