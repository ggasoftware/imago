option(USE_SYSTEM_OPENCV "Use system OpenCV libs or download prebuilt from the server" OFF)

if(NOT USE_SYSTEM_OPENCV)
	set(OpenCV_DIR "${THIRD_PARTY_DIR}/opencv/${SYSTEM_NAME}/${SUBSYSTEM_NAME}")
	set(OpenCV_FILENAME "opencv_${SYSTEM_NAME}_${SUBSYSTEM_NAME}.zip")
	if(NOT EXISTS ${OpenCV_DIR})
		if(NOT EXISTS ${THIRD_PARTY_DIR}/${OpenCV_FILENAME})
			message("Downloading OpenCV headers")
			file(DOWNLOAD ${LIBS_LOCATION}/${OpenCV_FILENAME}
					      ${THIRD_PARTY_DIR}/${OpenCV_FILENAME} 
					      STATUS opencv_status SHOW_PROGRESS)
		endif()
		execute_process(COMMAND ${CMAKE_COMMAND} -E make_directory ${OpenCV_DIR})
		execute_process(COMMAND ${CMAKE_COMMAND} -E tar xf ${THIRD_PARTY_DIR}/${OpenCV_FILENAME}
						WORKING_DIRECTORY ${OpenCV_DIR})
	endif()

	if(NOT SYSTEM_NAME STREQUAL "Win")
		set(OpenCV_DIR "${OpenCV_DIR}/share/OpenCV")
	endif()

	include("${OpenCV_DIR}/OpenCVConfig.cmake")
	include_directories(${OpenCV_INCLUDE_DIRS})
else()
	#find_package
	message(FATAL_ERROR "Not implemented yet!")
endif()
