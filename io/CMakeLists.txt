include_directories(..)
add_definitions(-std=c++11)

ADD_LIBRARY(woodycxx_io STATIC InputStream.cpp OutputStream.cpp InputStreamImpl.cpp)

add_executable(Test_ByteBuffer Test_ByteBuffer.cpp)
target_link_libraries(Test_ByteBuffer woodycxx_io)
add_executable(Test_InputStreamImpl Test_InputStreamImpl.cpp)
target_link_libraries(Test_InputStreamImpl woodycxx_io)