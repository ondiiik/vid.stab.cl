/*
 * opencl_inst.cpp
 *
 *  Created on: 18. 9. 2018
 *      Author: ondiiik
 */

#include "opencl.hpp"


namespace OpenCl
{
    OpenCl::OpenCl()
        :
        _device   { nullptr }
    {
//        cl_int result = cl::Platform::get(&_platforms);
//
//        if (CL_SUCCESS != result)
//        {
//            throw mdException("OpenCL reported %i!", result);
//        }
//
//        if (0 == _platforms.size())
//        {
//            throw mdException("No platforms found!");
//        }
    }
    
    
    OpenCl::~OpenCl()
    {
    
    }
    
    
    Platform::Platform(const cl::Platform& aPlatform)
        :
        platform { aPlatform }
    {
        cl_int result = platform.getDevices(CL_DEVICE_TYPE_ALL, &_devices);
        
        if ((CL_SUCCESS != result) && (CL_DEVICE_NOT_FOUND != result))
        {
            throw exception("OpenCL get devices reported %i!", result);
        }
    }
    
    
    cl::Device& Platform::operator [](std::size_t aIdx)
    {
        if (aIdx >= _devices.size())
        {
            throw exception("Device index %d out of range <%d - %d>!",
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
            throw exception("OpenCL get platforms reported %i!", result);
        }
        
        if (0 == platforms.size())
        {
            throw exception("No platforms found!");
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
            throw exception("Platform index %d out of range <%d - %d>!",
                            aIdx, 0, _platforms.size());
        }
        
        return _platforms[aIdx];
    }
    
    
    exception::exception(const char* aFmt, ...)
    {
        va_list                            args;
        va_start(                          args, aFmt);
        vsnprintf(_errTxt, _bufSize, aFmt, args);
        va_end(                            args);
    }
    
    
    const char* exception::what() const throw ()
    {
        return _errTxt;
    }
    
    
    Devices devices {};
}
