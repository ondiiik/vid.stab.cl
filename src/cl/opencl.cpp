/*
 * opencl_inst.cpp
 *
 *  Created on: 18. 9. 2018
 *      Author: ondiiik
 */


#if defined(USE_OPENCL)
#include "opencl.h"


namespace
{
    const char moduleName[] { "OpenCL" };
}


namespace OpenCl
{
    Platform::Platform(const cl::Platform& aPlatform)
        :
        platform { aPlatform }
    {
        cl_int result = platform.getDevices(CL_DEVICE_TYPE_ALL, &_devices);
        
        if ((CL_SUCCESS != result) && (CL_DEVICE_NOT_FOUND != result))
        {
            throw Common::exception("[OpenCL] Get devices reported %i!", result);
        }
    }
    
    
    cl::Device& Platform::operator [](std::size_t aIdx)
    {
        if (aIdx >= _devices.size())
        {
            throw Common::exception("[OpenCL] Device index %d out of range <%d - %d>!",
                                    aIdx, 0, _devices.size());
        }
        
        return _devices[aIdx];
    }
    
    
    Devices::Devices()
        :
        _platforms {}
    {
        std::vector<cl::Platform>          platforms;
        cl_int result = cl::Platform::get(&platforms);
        
        if (CL_SUCCESS != result)
        {
            throw Common::exception("[OpenCL] Get platforms reported %i!", result);
        }
        
        if (0 == platforms.size())
        {
            throw Common::exception("[OpenCL] No platforms found!");
        }
        
        for (auto& i : platforms)
        {
            Platform                            p { i };
            _platforms.insert(_platforms.end(), p);
        }
    }
    
    
    Platform& Devices::operator [](std::size_t aIdx)
    {
        if (aIdx >= _platforms.size())
        {
            throw Common::exception("[OpenCL] Platform index %d out of range <%d - %d>!",
                                    aIdx, 0, _platforms.size());
        }
        
        return _platforms[aIdx];
    }
    
    
    const char* err2string(cl_int aErr)
    {
        switch (aErr)
        {
#if defined(CL_SUCCESS)
            case CL_SUCCESS:
                return "SUCCESS";
#endif
#if defined(CL_DEVICE_NOT_FOUND)
            case CL_DEVICE_NOT_FOUND:
                return "DEVICE_NOT_FOUND";
#endif
#if defined(CL_DEVICE_NOT_AVAILABLE)
            case CL_DEVICE_NOT_AVAILABLE:
                return "DEVICE_NOT_AVAILABLE";
#endif
#if defined(CL_COMPILER_NOT_AVAILABLE)
            case CL_COMPILER_NOT_AVAILABLE:
                return "COMPILER_NOT_AVAILABLE";
#endif
#if defined(CL_MEM_OBJECT_ALLOCATION_FAILURE)
            case CL_MEM_OBJECT_ALLOCATION_FAILURE:
                return "MEM_OBJECT_ALLOCATION_FAILURE";
#endif
#if defined(CL_OUT_OF_RESOURCES)
            case CL_OUT_OF_RESOURCES:
                return "OUT_OF_RESOURCES";
#endif
#if defined(CL_OUT_OF_HOST_MEMORY)
            case CL_OUT_OF_HOST_MEMORY:
                return "OUT_OF_HOST_MEMORY";
#endif
#if defined(CL_PROFILING_INFO_NOT_AVAILABLE)
            case CL_PROFILING_INFO_NOT_AVAILABLE:
                return "PROFILING_INFO_NOT_AVAILABLE";
#endif
#if defined(CL_MEM_COPY_OVERLAP)
            case CL_MEM_COPY_OVERLAP:
                return "MEM_COPY_OVERLAP";
#endif
#if defined(CL_IMAGE_FORMAT_MISMATCH)
            case CL_IMAGE_FORMAT_MISMATCH:
                return "IMAGE_FORMAT_MISMATCH";
#endif
#if defined(CL_IMAGE_FORMAT_NOT_SUPPORTED)
            case CL_IMAGE_FORMAT_NOT_SUPPORTED:
                return "IMAGE_FORMAT_NOT_SUPPORTED";
#endif
#if defined(CL_BUILD_PROGRAM_FAILURE)
            case CL_BUILD_PROGRAM_FAILURE:
                return "BUILD_PROGRAM_FAILURE";
#endif
#if defined(CL_MAP_FAILURE)
            case CL_MAP_FAILURE:
                return "MAP_FAILURE";
#endif
#if defined(CL_MISALIGNED_SUB_BUFFER_OFFSET)
            case CL_MISALIGNED_SUB_BUFFER_OFFSET:
                return "MISALIGNED_SUB_BUFFER_OFFSET";
#endif
#if defined(CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST)
            case CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST:
                return "EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST";
#endif
#if defined(CL_COMPILE_PROGRAM_FAILURE)
            case CL_COMPILE_PROGRAM_FAILURE:
                return "COMPILE_PROGRAM_FAILURE";
#endif
#if defined(CL_LINKER_NOT_AVAILABLE)
            case CL_LINKER_NOT_AVAILABLE:
                return "LINKER_NOT_AVAILABLE";
#endif
#if defined(CL_LINK_PROGRAM_FAILURE)
            case CL_LINK_PROGRAM_FAILURE:
                return "LINK_PROGRAM_FAILURE";
#endif
#if defined(CL_DEVICE_PARTITION_FAILED)
            case CL_DEVICE_PARTITION_FAILED:
                return "DEVICE_PARTITION_FAILED";
#endif
#if defined(CL_KERNEL_ARG_INFO_NOT_AVAILABLE)
            case CL_KERNEL_ARG_INFO_NOT_AVAILABLE:
                return "KERNEL_ARG_INFO_NOT_AVAILABLE";
#endif
#if defined(CL_INVALID_VALUE)
            case CL_INVALID_VALUE:
                return "INVALID_VALUE";
#endif
#if defined(CL_INVALID_DEVICE_TYPE)
            case CL_INVALID_DEVICE_TYPE:
                return "INVALID_DEVICE_TYPE";
#endif
#if defined(CL_INVALID_PLATFORM)
            case CL_INVALID_PLATFORM:
                return "INVALID_PLATFORM";
#endif
#if defined(CL_INVALID_DEVICE)
            case CL_INVALID_DEVICE:
                return "INVALID_DEVICE";
#endif
#if defined(CL_INVALID_CONTEXT)
            case CL_INVALID_CONTEXT:
                return "INVALID_CONTEXT";
#endif
#if defined(CL_INVALID_QUEUE_PROPERTIES)
            case CL_INVALID_QUEUE_PROPERTIES:
                return "INVALID_QUEUE_PROPERTIES";
#endif
#if defined(CL_INVALID_COMMAND_QUEUE)
            case CL_INVALID_COMMAND_QUEUE:
                return "INVALID_COMMAND_QUEUE";
#endif
#if defined(CL_INVALID_HOST_PTR)
            case CL_INVALID_HOST_PTR:
                return "INVALID_HOST_PTR";
#endif
#if defined(CL_INVALID_MEM_OBJECT)
            case CL_INVALID_MEM_OBJECT:
                return "INVALID_MEM_OBJECT";
#endif
#if defined(CL_INVALID_IMAGE_FORMAT_DESCRIPTOR)
            case CL_INVALID_IMAGE_FORMAT_DESCRIPTOR:
                return "INVALID_IMAGE_FORMAT_DESCRIPTOR";
#endif
#if defined(CL_INVALID_IMAGE_SIZE)
            case CL_INVALID_IMAGE_SIZE:
                return "INVALID_IMAGE_SIZE";
#endif
#if defined(CL_INVALID_SAMPLER)
            case CL_INVALID_SAMPLER:
                return "INVALID_SAMPLER";
#endif
#if defined(CL_INVALID_BINARY)
            case CL_INVALID_BINARY:
                return "INVALID_BINARY";
#endif
#if defined(CL_INVALID_BUILD_OPTIONS)
            case CL_INVALID_BUILD_OPTIONS:
                return "INVALID_BUILD_OPTIONS";
#endif
#if defined(CL_INVALID_PROGRAM)
            case CL_INVALID_PROGRAM:
                return "INVALID_PROGRAM";
#endif
#if defined(CL_INVALID_PROGRAM_EXECUTABLE)
            case CL_INVALID_PROGRAM_EXECUTABLE:
                return "INVALID_PROGRAM_EXECUTABLE";
#endif
#if defined(CL_INVALID_KERNEL_NAME)
            case CL_INVALID_KERNEL_NAME:
                return "INVALID_KERNEL_NAME";
#endif
#if defined(CL_INVALID_KERNEL_DEFINITION)
            case CL_INVALID_KERNEL_DEFINITION:
                return "INVALID_KERNEL_DEFINITION";
#endif
#if defined(CL_INVALID_KERNEL)
            case CL_INVALID_KERNEL:
                return "INVALID_KERNEL";
#endif
#if defined(CL_INVALID_ARG_INDEX)
            case CL_INVALID_ARG_INDEX:
                return "INVALID_ARG_INDEX";
#endif
#if defined(CL_INVALID_ARG_VALUE)
            case CL_INVALID_ARG_VALUE:
                return "INVALID_ARG_VALUE";
#endif
#if defined(CL_INVALID_ARG_SIZE)
            case CL_INVALID_ARG_SIZE:
                return "INVALID_ARG_SIZE";
#endif
#if defined(CL_INVALID_KERNEL_ARGS)
            case CL_INVALID_KERNEL_ARGS:
                return "INVALID_KERNEL_ARGS";
#endif
#if defined(CL_INVALID_WORK_DIMENSION)
            case CL_INVALID_WORK_DIMENSION:
                return "INVALID_WORK_DIMENSION";
#endif
#if defined(CL_INVALID_WORK_GROUP_SIZE)
            case CL_INVALID_WORK_GROUP_SIZE:
                return "INVALID_WORK_GROUP_SIZE";
#endif
#if defined(CL_INVALID_WORK_ITEM_SIZE)
            case CL_INVALID_WORK_ITEM_SIZE:
                return "INVALID_WORK_ITEM_SIZE";
#endif
#if defined(CL_INVALID_GLOBAL_OFFSET)
            case CL_INVALID_GLOBAL_OFFSET:
                return "INVALID_GLOBAL_OFFSET";
#endif
#if defined(CL_INVALID_EVENT_WAIT_LIST)
            case CL_INVALID_EVENT_WAIT_LIST:
                return "INVALID_EVENT_WAIT_LIST";
#endif
#if defined(CL_INVALID_EVENT)
            case CL_INVALID_EVENT:
                return "INVALID_EVENT";
#endif
#if defined(CL_INVALID_OPERATION)
            case CL_INVALID_OPERATION:
                return "INVALID_OPERATION";
#endif
#if defined(CL_INVALID_GL_OBJECT)
            case CL_INVALID_GL_OBJECT:
                return "INVALID_GL_OBJECT";
#endif
#if defined(CL_INVALID_BUFFER_SIZE)
            case CL_INVALID_BUFFER_SIZE:
                return "INVALID_BUFFER_SIZE";
#endif
#if defined(CL_INVALID_MIP_LEVEL)
            case CL_INVALID_MIP_LEVEL:
                return "INVALID_MIP_LEVEL";
#endif
#if defined(CL_INVALID_GLOBAL_WORK_SIZE)
            case CL_INVALID_GLOBAL_WORK_SIZE:
                return "INVALID_GLOBAL_WORK_SIZE";
#endif
#if defined(CL_INVALID_PROPERTY)
            case CL_INVALID_PROPERTY:
                return "INVALID_PROPERTY";
#endif
#if defined(CL_INVALID_IMAGE_DESCRIPTOR)
            case CL_INVALID_IMAGE_DESCRIPTOR:
                return "INVALID_IMAGE_DESCRIPTOR";
#endif
#if defined(CL_INVALID_COMPILER_OPTIONS)
            case CL_INVALID_COMPILER_OPTIONS:
                return "INVALID_COMPILER_OPTIONS";
#endif
#if defined(CL_INVALID_LINKER_OPTIONS)
            case CL_INVALID_LINKER_OPTIONS:
                return "INVALID_LINKER_OPTIONS";
#endif
#if defined(CL_INVALID_DEVICE_PARTITION_COUNT)
            case CL_INVALID_DEVICE_PARTITION_COUNT:
                return "INVALID_DEVICE_PARTITION_COUNT";
#endif
#if defined(CL_INVALID_PIPE_SIZE)
            case CL_INVALID_PIPE_SIZE:
                return "INVALID_PIPE_SIZE";
#endif
#if defined(CL_INVALID_DEVICE_QUEUE)
            case CL_INVALID_DEVICE_QUEUE:
                return "INVALID_DEVICE_QUEUE";
#endif
            default:
                return "<?>";
        }
    }
    
    
    Devices devices {};
}


#endif /* defined(USE_OPENCL) */
