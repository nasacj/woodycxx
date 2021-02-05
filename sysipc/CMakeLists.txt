include_directories(..)
add_definitions(-std=c++11)

ADD_LIBRARY(woodycxx_sysipc STATIC sysipc.cpp sysipc_threads.cpp sysipc_event_log.cpp)
target_link_libraries(woodycxx_sysipc pthread)

add_executable(Test_Thread Test_Thread.cpp)
target_link_libraries(Test_Thread woodycxx_sysipc)
add_executable(Test_log Test_log.cpp)
target_link_libraries(Test_log woodycxx_sysipc boost_serialization)
