noinst_LIBRARIES = libwoodycxx_io.a

bin_PROGRAMS = Test_ByteBuffer Test_InputStreamImpl

libwoodycxx_io_a_SOURCES = InputStream.cpp OutputStream.cpp InputStreamImpl.cpp

Test_ByteBuffer_SOURCES = Test_ByteBuffer.cpp
Test_ByteBuffer_LDADD = libwoodycxx_io.a 

Test_InputStreamImpl_SOURCES = Test_InputStreamImpl.cpp
Test_InputStreamImpl_LDADD = libwoodycxx_io.a

