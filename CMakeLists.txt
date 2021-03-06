cmake_minimum_required (VERSION 2.8.5)
project (vid.stab)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/CMakeModules/")

include (FindSSE)
include (GNUInstallDirs)

set(MAJOR_VERSION 42)
set(MINOR_VERSION 0)
set(PATCH_VERSION 0)
set(VIDSTAB_VERSION ${MAJOR_VERSION}.${MINOR_VERSION}${PATCH_VERSION})

# Default to release builds if no explicit build type specified.
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE "Release")
endif()

option(BUILD_SHARED_LIBS "build shared libraries instead of static libraries"
       ON)

option(USE_OMP              "use parallelization over OMP"                         ON)
option(USE_COUT             "use std::cout for printing messages"                  ON)
option(USE_SSE2             "use SSE2 optimization"                                ON)
option(USE_OPENCL_DETECT    "use parallelization over OpenCL for detect plugin"    ON)
option(USE_OPENCL_TRANSFORM "use parallelization over OpenCL for transform plugin" ON)
option(USE_OPENCL_DEBUG     "use non-obfuscated OpenCL code"                       OFF)

add_definitions(-fPIC
                -DCL_HPP_TARGET_OPENCL_VERSION=110
                -DCL_HPP_MINIMUM_OPENCL_VERSION=110)

### ORC is not used in any active code at the moment  ###
# I tried it with 0.4.14
#  0.4.10 did not work (not all opcode implemented)
# find_package(Orc)
if(ORC_FOUND)
add_definitions(-DUSE_ORC ${ORC_DEFINITIONS})
include_directories(${ORC_INCLUDE_DIRS})
else()
add_definitions(-DDISABLE_ORC)
endif()

# here we should check for SSE2
# our  -DUSE_SSE2_ASM code does not work with fpic

if(USE_SSE2)
if(SSE2_FOUND)
add_definitions(-DUSE_SSE2=1
                -msse2
                -ffast-math)
endif()
endif()

if(USE_COUT)
add_definitions(-DUSE_COUT=1)
endif()

if(USE_OMP)
add_definitions(-fopenmp
                -DUSE_OMP=1)
endif()

if(USE_OPENCL_DETECT)
add_definitions(-DUSE_OPENCL=1
                -DUSE_OPENCL_DETECT=1)
endif()

if(USE_OPENCL_DEBUG)
add_definitions(-DOPENCL_DBG_MODE=1)
endif()

set(SOURCES
    src/cl/opencl.cpp
    src/cl/opencl___blur_h.c
    src/cl/opencl___blur_v.c
    src/frameinfo.cpp
    src/gimbal_detector.cpp
    src/gimbal_corrector.cpp
    src/gimbal_motions.cpp
    src/gimbal_serializer.cpp
    src/libvidstab.cpp
    src/localmotion2transform.cpp
    src/motiondetect_opt.cpp
    src/orc/motiondetectorc.c
    src/serialize.cpp
    src/transformfixedpoint.cpp
    src/transformtype.cpp
    src/gimbal_barrel.cpp
    src/vs_vector.cpp)

set(HEADERS
    src/common_cutils.h
    src/common_dbg.h
    src/common_exception.h
    src/common_range.h
    src/common_vect.h
    src/dbg_profiler.h
    src/frame_buff.h
    src/frame_canvas.h
    src/frame_pyramid.h
    src/frameinfo.h
    src/gimbal_detector.h
    src/gimbal_corrector.h
    src/gimbal_motions.h
    src/gimbal_serializer.h
    src/libvidstab.h
    src/localmotion2transform.h
    src/motiondetect_internal.h
    src/motiondetect_opt.h
    src/serialize.h
    src/sys_omp.h
    src/transform_internal.h
    src/transformfixedpoint.h
    src/transformfloat.h
    src/transformtype.h
    src/transformtype_operations.h
    src/vidstabdefines.h
    src/vs_transformation.h
    src/gimbal_barrel.h
    src/vs_vector.h)


# Create the vidstab library
add_library (vidstab ${SOURCES})

#set version of lib
set_target_properties(vidstab PROPERTIES SOVERSION ${MAJOR_VERSION}.${MINOR_VERSION})


target_link_libraries(vidstab m)
set(PKG_EXTRA_LIBS -lm)
 
target_link_libraries(vidstab OpenCL)
set(PKG_EXTRA_LIBS "${PKG_EXTRA_LIBS} -lOpenCL")

if(ORC_FOUND)
target_link_libraries(vidstab ${ORC_LIBRARIES})
set(PKG_EXTRA_LIBS "${PKG_EXTRA_LIBS} ${ORC_LIBRARIES}")
endif()

if(USE_OMP)
target_link_libraries(vidstab gomp)
set(PKG_EXTRA_LIBS "${PKG_EXTRA_LIBS} -lgomp -lpthread")
endif()


#if(!NOHEADERS)
FILE(GLOB HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h")
INSTALL(FILES ${HEADERS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/vid.stab)
#endif()

INSTALL(TARGETS vidstab
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

include(create_pkgconfig_file)
create_pkgconfig_file(vidstab "Vid.Stab, a library for stabilizing video clips")
