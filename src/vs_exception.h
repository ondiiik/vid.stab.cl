/*
 * md_exception.h
 *
 *  Created on: 18. 9. 2018
 *      Author: ondiiik
 */
#pragma once


#include <exception>
#include <cstdarg>
#include <cstring>


#define VD_EXCEPTION(...) exception(moduleName, __FUNCTION__, __LINE__, __VA_ARGS__)


namespace VidStab
{
    /**
     * @brief   Vidstab exception
     */
    class exception: public std::exception
    {
    public:
        exception(const char* aModule,
                  const char* aFunc,
                  const int   aLine,
                  const char* aFmt, ...)
        {
            va_list  args;
            va_start(args, aFmt);
            
            
            snprintf(_errTxt, _bufSize, "[%s]\t%s:%i\n", aModule, aFunc, aLine);
            
            std::size_t idx = strnlen(_errTxt, _bufSize);
            
            if (idx < (_bufSize - 2))
            {
                vsnprintf(_errTxt + idx, _bufSize - idx - 1, aFmt, args);
            }
            
            va_end(                            args);
        }
        
        
        virtual const char* what() const throw()
        {
            return _errTxt;
        }
        
        
    private:
        static const std::size_t _bufSize { 1024 };
        char             _errTxt[_bufSize];
    };
}

