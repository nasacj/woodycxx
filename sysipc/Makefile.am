noinst_LIBRARIES = libsysipc.a

bin_PROGRAMS = Test_Thread Test_log

libsysipc_a_SOURCES = sysipc.cpp sysipc_threads.cpp sysipc_event_log.cpp

Test_Thread_SOURCES = Test_Thread.cpp
Test_Thread_LDADD = libsysipc.a

Test_log_SOURCES = Test_log.cpp
Test_log_LDADD = libsysipc.a
