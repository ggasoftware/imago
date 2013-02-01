project(ImagoLib)

file(GLOB SRC src/*.cpp)

FILE(GLOB HEADERS "src/*.h")
list(APPEND SRC ${HEADERS})

add_library(imago STATIC ${SRC})

target_link_libraries(imago ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(imago ${OpenCV_LIBS})
target_link_libraries(imago ${Boost_LIBRARIES}) 

target_link_libraries(imago debug ${INDIGO_LIBS_DBG})
target_link_libraries(imago optimized ${INDIGO_LIBS})
