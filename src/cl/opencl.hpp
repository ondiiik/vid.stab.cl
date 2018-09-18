/*
 * opencl_inst.h
 *
 *  Created on: 18. 9. 2018
 *      Author: ondiiik
 */
#pragma once


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
     * @brief   Represents OpenCL kernel
     */
    class OpenCl
    {
    public:
        /**
         * @brief   Construct OpenCL
         */
        OpenCl();
        
        
        /**
         * @brief   Destroy OpenCL
         */
        virtual ~OpenCl();
        
        
    private:
        /**
         * @brief   Current device
         */
        cl::Device* _device;
    };
    
    
    /**
     * @brief   List of OpenCL devices
     */
    extern Devices devices;
}

