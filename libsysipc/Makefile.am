#-------------------------------------------------------------------------
# 
# Licensed Materials - Property of IBM.
# 
# (C) Copyright IBM Corporation 2008
# 
# All Rights Reserved.
# 
# US Government Users Restricted Rights -
# Use, duplication or disclosure restricted by
# GSA ADP Schedule Contract with IBM Corporation.
#---------------------------------------------------------------------------



# create a static library from the source files
noinst_LIBRARIES = libsysipc.a

if BLD_X86
   X86=1
else 
   X86=0
endif

# treat c file as c++
AM_CFLAGS = -x "c++"

#libsysipc_a_CXXFLAGS = -O2 -fpic -Wall -Werror 
libsysipc_a_CXXFLAGS = -O2 -fpic -Wall -D__X86=$(X86) -I$(LCW_INCDIR)
libsysipc_a_CPPFLAGS = -O2 -fpic -Wall -D__X86=$(X86) -I$(LCW_INCDIR)
libsysipc_a_SOURCES = sysipc.cpp sysipc_message_queue.cpp \
   sysipc_threads.cpp sysipc_tlv.cpp sysipc_messages.cpp sysipc_files.cpp \
   sysipc_command_line.cpp sysipc_dynalib.cpp sysipc_event_log.cpp sysipc_application_control_lib.cpp \
   sysipc_inotify.cpp sysipc_inotify2.cpp sysipc_exceptions.cpp sysipc_efi.cpp EfiCompressDecompress.c \
   sysipc_security_wrap.cpp

