include_directories(..)
add_definitions(-std=c++11)

ADD_LIBRARY(woodycxx_net STATIC SocketsOpt.cpp InetAddress.cpp InetSocketAddress.cpp SocketInputStream.cpp SocketOutputStream.cpp AbstractSocketImpl.cpp)
target_link_libraries(woodycxx_net woodycxx_io)

enable_testing()
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIR})

################################
# Unit Tests
################################
# Add test cpp file
add_executable( runUnitTests testgtest.cpp )
# Link test executable against gtest & gtest_main
target_link_libraries(runUnitTests gtest gtest_main pthread)
add_test( runUnitTests runUnitTests )


add_executable(Test_InetAddress Test_InetAddress.cpp)
target_link_libraries(Test_InetAddress woodycxx_net)

add_executable(Test_InetSocketAddress Test_InetSocketAddress.cpp)
target_link_libraries(Test_InetSocketAddress woodycxx_net)

add_executable(Test_AbstractSocketImpl Test_AbstractSocketImpl.cpp)
#target_link_libraries(Test_AbstractSocketImpl woodycxx_sysipc)
#target_link_libraries(Test_AbstractSocketImpl woodycxx_io)
target_link_libraries(Test_AbstractSocketImpl woodycxx_net)
