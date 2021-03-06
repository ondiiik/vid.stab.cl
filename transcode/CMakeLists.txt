cmake_minimum_required (VERSION 2.6)
project (vid.stab.transcode)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/../CMakeModules/")

# set your transcode path here!
set(TRANSCODE_ROOT ../../transcode)

# Default to release builds if no explicit build type specified.
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE "Release")
endif()

add_definitions(-DTRANSCODE)
# I tried it with 0.4.14
#  0.4.10 did not work (not all opcode implemented)
# find_package(Orc)  // it actually not used by any active code
if(ORC_FOUND)
add_definitions( -DUSE_ORC ${ORC_DEFINITIONS} )
include_directories( ${ORC_INCLUDE_DIRS} )
else()
add_definitions( -DDISABLE_ORC )
endif()

# here we should check for SSE2
# our  -DUSE_SSE2_ASM code does not work with fpic
add_definitions( -DUSE_SSE2 -msse2 -ffast-math )

# Make sure the compiler can find include files from transcode
include_directories (../src ${TRANSCODE_ROOT}/src ${TRANSCODE_ROOT}/ )

# Create the stabilize and transform libraries
add_library (filter_transform SHARED filter_transform.c
  ../src/transformtype.cpp ../src/libvidstab.cpp ../src/transform.cpp
  ../src/transformfixedpoint.cpp  ../src/vsvector.cpp ../src/serialize.cpp ../src/frameinfo.cpp
	../src/localmotion2transform.cpp) #  orc/transformorc.c)
add_library (filter_stabilize SHARED filter_stabilize.c
  ../src/transformtype.cpp ../src/libvidstab.cpp ../src/motiondetect.cpp
	../src/orc/motiondetectorc.c ../src/motiondetect_opt.cpp ../src/localmotion2transform.cpp
  ../src/boxblur.cpp ../src/vsvector.cpp ../src/serialize.cpp  ../src/frameinfo.cpp)
add_library (filter_deshake SHARED filter_deshake.c
  ../src/transformtype.cpp ../src/libvidstab.cpp ../src/motiondetect.cpp
  ../src/orc/motiondetectorc.c ../src/boxblur.cpp ../src/transform.cpp ../src/motiondetect_opt.cpp
  ../src/transformfixedpoint.cpp ../src/vsvector.cpp ../src/serialize.cpp ../src/frameinfo.cpp
	../src/localmotion2transform.cpp)


set_target_properties(filter_stabilize PROPERTIES  PREFIX "" )
set_target_properties(filter_transform PROPERTIES  PREFIX "" )
set_target_properties(filter_deshake   PROPERTIES  PREFIX "" )

target_link_libraries(filter_stabilize m )
target_link_libraries(filter_transform m )
target_link_libraries(filter_deshake   m )

if(ORC_FOUND)
target_link_libraries(filter_stabilize ${ORC_LIBRARIES} )
target_link_libraries(filter_transform ${ORC_LIBRARIES} )
target_link_libraries(filter_deshake   ${ORC_LIBRARIES} )
endif()
if(USE_OMP)
target_link_libraries(filter_stabilize gomp )
target_link_libraries(filter_transform gomp )
target_link_libraries(filter_deshake   gomp )
endif()
