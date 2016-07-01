cmake_minimum_required(VERSION 2.8)
project(BlockMon)  

#############################################################
# File scan
#############################################################
# core
file(GLOB_RECURSE CORE_OBJ core/*.cpp)

# lib original version

file(GLOB_RECURSE LIB_OBJ_CPP lib/*.cpp)
file(GLOB_RECURSE LIB_OBJ_HPP lib/*.hpp)
file(GLOB_RECURSE LIB_OBJ_C lib/*.c)
list(APPEND LIB_OBJ ${LIB_OBJ_CPP} ${LIB_OBJ_HPP} ${LIB_OBJ_C})

# blocks
file(GLOB BLOCKS_OBJ blocks/*.cpp)

# messages
file(GLOB MSG_OBJ messages/*.cpp)

# user files
file(GLOB USR_OBJ usr/*/blocks/*.cpp)
file(GLOB USR_INCL usr/*/blocks/)
file(GLOB USR_MGS_INCL usr/*/messages/)

# daemon: c++ to python bindings
file(GLOB DAEMON_BINDINGS daemon/*.cpp)

#############################################################
# Optional packages
#############################################################
if(WITH_PFRING)
  message(STATUS "pfring: ON")
  add_definitions(-DPFRING_BLOCK)
  list(APPEND TARGET_LINK_LIBS pfring)
else(WITH_PFRING)
  message(STATUS "pfring: OFF")

   # exclude pf_ring block
   set(TMP_LIST "")
   foreach(arg ${BLOCKS_OBJ})
     string(REGEX REPLACE "PFRingSource.cpp" "" arg ${arg})
     list(APPEND TMP_LIST ${arg})
   endforeach(arg ${BLOCKS_OBJ})
   set(BLOCKS_OBJ ${TMP_LIST})

endif()

if(WITH_PFQ)
  message(STATUS "pfq: ON")
  add_definitions(-DPFQ_BLOCK)
  list(APPEND INCLUDE_DIRS lib/external/pfq)
else(WITH_PFQ)
  message(STATUS "pfq: OFF")

   # exclude pfq block
   set(TMP_LIST "")
   foreach(arg ${BLOCKS_OBJ})
     string(REGEX REPLACE "PFQSource.cpp" "" arg ${arg})
     list(APPEND TMP_LIST ${arg})
   endforeach(arg ${BLOCKS_OBJ})
   set(BLOCKS_OBJ ${TMP_LIST})

endif()

list(APPEND INCLUDE_DIRS lib/fc)

# INVEA-TECH HW acceleration system libraries
if(WITH_COMBO)
   message(STATUS "COMBO(sze2): ON")
  list(APPEND TARGET_LINK_LIBS sze2)
  list(APPEND TARGET_LINK_LIBS commlbr)
else(WITH_COMBO)
   message(STATUS "COMBO(sze2): OFF")

   # exclude any paths to sze2 dependent files
   set(TMP_LIST "")
   foreach(arg ${USR_INCL})
     string(REGEX REPLACE ".*/usr/combo/.*" "" arg ${arg})
     list(APPEND TMP_LIST ${arg})
   endforeach(arg ${USR_INCL})
   set(USR_INCL ${TMP_LIST})

   set(TMP_LIST "")
   foreach(arg ${USR_OBJ})
     string(REGEX REPLACE ".*/usr/combo/.*" "" arg ${arg})
     list(APPEND TMP_LIST ${arg})
   endforeach(arg ${USR_OBJ})
   set(USR_OBJ ${TMP_LIST})

endif()


# For debug purposes
#message (STATUS "INCLUDE_DIRS: " ${INCLUDE_DIRS})
#message (STATUS "BLOCKS_OBJ: " ${BLOCKS_OBJ})

#############################################################
# Build
#############################################################
list(APPEND TARGET_LINK_LIBS  pcap pthread dl rt zmq)
list(APPEND INCLUDE_DIRS lib/external/pugixml lib lib/external/factory lib/external/more lib/external/ lib/hash lib/sketch lib/cusum core messages blocks core/block core/message core/composition core/scheduler ${USR_MSG_INCL} ${USR_INCL} )
 
include_directories( ${INCLUDE_DIRS} )
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -O3 -march=native -Wall -flto")
add_definitions(-D_GLIBCXX_USE_SCHED_YIELD -D_GLIBCXX_USE_NANOSLEEP)
#add_definitions(-D_GLIBCXX_DEBUG_PEDANTIC )

add_executable(blockmon ${CORE_OBJ} ${MSG_OBJ} ${BLOCKS_OBJ} ${LIB_OBJ} ${USR_OBJ} bin/blockmon.cpp)
target_link_libraries(blockmon ${TARGET_LINK_LIBS})

if(WITH_DAEMON)
  message(STATUS "daemon: ON")
  LINK_LIBRARIES(boost_python pcap)
  find_package(PythonLibs REQUIRED)
  include_directories(${PYTHON_INCLUDE_DIRS})
  link_directories(${PYTHON_LIBRARIES})
  ADD_LIBRARY(blockmonlib SHARED ${CORE_OBJ} ${MSG_OBJ} ${LIB_OBJ} ${BLOCKS_OBJ} ${USR_OBJ} ${DAEMON_BINDINGS})
  target_link_libraries(blockmon ${TARGET_LINK_LIBS})
else(WITH_DAEMON)
  message(STATUS "daemon: OFF")
endif()
