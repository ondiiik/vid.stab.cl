/*
 * md_exception.h
 *
 *  Created on: 18. 9. 2018
 *      Author: ondiiik
 */
#pragma once


#include <exception>
#include <cstdarg>


namespace VidStab
{
    /**
     * @brief   Vidstab exception
     */
    class mdException: public std::exception
    {
    public:
        mdException(const char* aFmt, ...)
        {
            va_list                            args;
            va_start(                          args, aFmt);
            vsnprintf(_errTxt, _bufSize, aFmt, args);
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

