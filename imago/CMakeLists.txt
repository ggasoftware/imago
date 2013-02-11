project(ImagoLib)

file(GLOB SRC src/*.cpp)

FILE(GLOB HEADERS "src/*.h")
list(APPEND SRC ${HEADERS})

add_library(imago STATIC ${SRC})

target_link_libraries(imago ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(imago ${OpenCV_LIBS})
target_link_libraries(imago ${Boost_LIBRARIES}) 


foreach(lib ${INDIGO_LIBS_DEBUG})
	target_link_libraries(imago debug ${lib})
endforeach()

foreach(lib ${INDIGO_LIBS_RELEASE})
	target_link_libraries(imago optimized ${lib})
endforeach()
