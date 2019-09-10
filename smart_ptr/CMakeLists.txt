include_directories(..)
add_definitions(-std=c++11)

add_executable(Test_scoped_ptr Test_scoped_ptr.cpp)
add_executable(Test_scoped_array Test_scoped_array.cpp)
add_executable(Test_shared_ptr Test_shared_ptr.cpp)
add_executable(Test_shared_array Test_shared_array.cpp)
