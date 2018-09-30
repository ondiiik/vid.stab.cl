/*
 * opencl_inst.h
 *
 *  Created on: 18. 9. 2018
 *      Author: ondiiik
 */
#pragma once


#if defined(USE_OPENCL)
#include <CL/cl2.hpp>
#include <exception>
#include <cstdarg>


namespace OpenCl
{
    /**
     * @brief   OpenCL exception
     */
    class exception: public std::exception
    {
    public:
        /**
         * @brief   Construct exception
         * @param   aFmt    Formating text
         */
        exception(const char* aFmt, ...);
        
        
        /**
         * @brief   Return text of exception
         * @return  Text of exception
         */
        virtual const char* what() const throw();
        
        
    private:
        static const std::size_t _bufSize { 1024 };
        char             _errTxt[_bufSize];
    };
    
    
    /**
     * @brief   Convert error code to string
     * @param   aErr    Error code
     * @return  String representation of error code
     */
    const char* err2string(cl_int aErr);
    
    
    /**
     * @brief   List of device belonging to single platform
     */
    class Platform
    {
    public:
        /**
         * @brief   Device structure can be iterated over platforms
         */
        typedef std::vector<cl::Device>::iterator iterator;
        
        
        /**
         * @brief   Device structure can be iterated over platforms
         */
        typedef std::vector<cl::Device>::const_iterator const_iterator;
        
        
        /**
         * @brief   Build platform
         * @param   aPlatform   Platform
         */
        Platform(const cl::Platform& aPlatform);
        
        
        /**
         * @brief   Get device on selected index
         * @param   aIdx    Index of platform
         * @return  Device
         */
        cl::Device& operator[](std::size_t aIdx);
        
        
        /**
         * @brief   Defines platform begin iterator
         * @return  Iterator
         */
        inline iterator begin()
        {
            return _devices.begin();
        }
        
        
        /**
         * @brief   Defines platform begin iterator
         * @return  Iterator
         */
        inline const_iterator begin() const
        {
            return _devices.begin();
        }
        
        
        /**
         * @brief   Defines platform begin iterator
         * @return  Iterator
         */
        inline iterator end()
        {
            return _devices.end();
        }
        
        
        /**
         * @brief   Defines platform begin iterator
         * @return  Iterator
         */
        inline const_iterator end() const
        {
            return _devices.end();
        }
        
        
        /**
         * @brief   Platform
         */
        cl::Platform platform;
        
        
    private:
        /**
         * @brief   List of device belonging to platform
         */
        std::vector<cl::Device> _devices;
    };
    
    
    /**
     * @brief   Represents all devices and platforms
     */
    class Devices
    {
    public:
        /**
         * @brief   Device structure can be iterated over platforms
         */
        typedef std::vector<Platform>::iterator iterator;
        
        
        /**
         * @brief   Device structure can be iterated over platforms
         */
        typedef std::vector<Platform>::const_iterator const_iterator;
        
        
        /**
         * @brief   Build platform and device list
         */
        Devices();
        
        
        /**
         * @brief   Get platform on selected index
         * @param   aIdx    Index of platform
         * @return  Platform
         */
        Platform& operator[](std::size_t aIdx);
        
        
        /**
         * @brief   Defines platform begin iterator
         * @return  Iterator
         */
        inline iterator begin()
        {
            return _platforms.begin();
        }
        
        
        /**
         * @brief   Defines platform begin iterator
         * @return  Iterator
         */
        inline const_iterator begin() const
        {
            return _platforms.begin();
        }
        
        
        /**
         * @brief   Defines platform begin iterator
         * @return  Iterator
         */
        inline iterator end()
        {
            return _platforms.end();
        }
        
        
        /**
         * @brief   Defines platform begin iterator
         * @return  Iterator
         */
        inline const_iterator end() const
        {
            return _platforms.end();
        }
        
        
    private:
        /**
         * @brief   List of available platforms
         */
        std::vector<Platform> _platforms;
    };
    
    
    /**
     * @brief   Extends command queue
     */
    class CommandQueue : public cl::CommandQueue
    {
    public:
        using cl::CommandQueue::CommandQueue;
        
        
        inline void enqueueWriteBuffer(const cl::Buffer&             buffer,
                                       cl_bool                       blocking,
                                       cl::size_type                 offset,
                                       cl::size_type                 size,
                                       const void*                   ptr,
                                       const std::vector<cl::Event>* events = NULL,
                                       cl::Event*                    event  = NULL) const
        {
            cl_int ret = cl::CommandQueue::enqueueWriteBuffer(buffer,
                                                              blocking,
                                                              offset,
                                                              size,
                                                              ptr,
                                                              events,
                                                              event);
                                                              
            if (CL_SUCCESS != ret)
            {
                throw exception("[OpenCL] Enqueue write buffer reported %i (%s)!",
                                int(ret),
                                OpenCl::err2string(ret));
            }
        }
        
        
        inline void enqueueNDRangeKernel(const cl::Kernel&             kernel,
                                         const cl::NDRange&            offset,
                                         const cl::NDRange&            global,
                                         const cl::NDRange&            local  = cl::NullRange,
                                         const std::vector<cl::Event>* events = NULL,
                                         cl::Event*                    event  = NULL) const
        {
            cl_int ret = cl::CommandQueue::enqueueNDRangeKernel(kernel,
                                                                offset,
                                                                global,
                                                                local,
                                                                events,
                                                                event);
                                                                
            if (CL_SUCCESS != ret)
            {
                throw exception("[OpenCL] Enqueue kernel reported %i (%s)!",
                                int(ret),
                                OpenCl::err2string(ret));
            }
        }
        
        
        inline void enqueueReadBuffer(const cl::Buffer&             buffer,
                                      cl_bool                       blocking,
                                      cl::size_type                 offset,
                                      cl::size_type                 size,
                                      void*                         ptr,
                                      const std::vector<cl::Event>* events = NULL,
                                      cl::Event*                    event  = NULL) const
        {
            cl_int ret = cl::CommandQueue::enqueueReadBuffer(buffer,
                                                             blocking,
                                                             offset,
                                                             size,
                                                             ptr,
                                                             events,
                                                             event);
                                                             
            if (CL_SUCCESS != ret)
            {
                throw exception("[OpenCL] Enqueue read buffer reported %i (%s)!",
                                int(ret),
                                OpenCl::err2string(ret));
            }
        }
    };
    
    
    /**
     * @brief   Extends kernel object
     */
    class Kernel : public cl::Kernel
    {
    public:
        inline Kernel(const cl::Program& program,
                      const char*        name)
            :
            cl::Kernel { program, name, &_ret },
            _name      { name                 }
        {
            if (CL_SUCCESS != _ret)
            {
                throw exception("[OpenCL-%s] Kernel constructor reported %i (%s)!",
                                _name.c_str(),
                                int(_ret),
                                OpenCl::err2string(_ret));
            }
        }
        
        
        template <typename _Tp> inline cl_uint setArg(cl_uint aIdx,
                                                      _Tp&    aArg)
        {
            cl_int ret = cl::Kernel::setArg(aIdx, aArg);
            
            if (CL_SUCCESS != ret)
            {
                throw exception("[OpenCL-%s] Set arg idx %i reported %i (%s)!",
                                _name.c_str(),
                                int(aIdx),
                                int(ret),
                                OpenCl::err2string(ret));
            }

            return aIdx + 1;
        }
        
        
    private:
        cl_int      _ret;
        std::string _name;
    };
    
    
    /**
     * @brief   Extends CL buffer
     */
    class Buffer : public cl::Buffer
    {
    public:
        inline Buffer(const cl::Context& context,
                      cl_mem_flags       flags,
                      cl::size_type      size,
                      void*              host_ptr = NULL)
            :
            cl::Buffer(context, flags, size, host_ptr, &_ret)
        {
            if (CL_SUCCESS != _ret)
            {
                throw exception("[OpenCL] Buffer constructor reported %i (%s)!",
                                int(_ret),
                                OpenCl::err2string(_ret));
            }
        }
        
        
    private:
        cl_int _ret;
    };
    
    
    /**
     * @brief   List of OpenCL devices
     */
    extern Devices devices;
}
#endif /* defined(USE_OPENCL) */

