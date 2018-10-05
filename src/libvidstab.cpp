/*
 * libvidstab.cpp
 *
 *  Created on: Feb 21, 2011
 *  Copyright (C) Georg Martius - February 2011
 *
 *  This file is part of transcode, a video stream processing tool
 *
 *  transcode is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  transcode is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with GNU Make; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */
#include "libvidstab.h"
#include "vidstabdefines.h"


#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <cstdarg>

#include "common_exception.h"


namespace
{
    const char moduleName[] { "VidStab" };
}




/**** default values for memory and logging ****/

/// logging function
int _vs_log(int type, const char* tag, const char* format, ...)
{
    if (vs_log_level >= type)
    {
        fprintf(stderr, "%s (%s):",
                type == VS_ERROR_TYPE ? "Error: " :
                type == VS_WARN_TYPE  ? "Warn:  " :
                type == VS_INFO_TYPE  ? "Info:  " :
                type == VS_MSG_TYPE   ? "Msg:   " : "Unknown",
                tag);
        va_list ap;
        va_start (ap, format);
        vfprintf (stderr, format, ap);
        va_end (ap);
        fprintf(stderr, "\n");
    }
    return 0;
}


/**
 * @brief   Memory allocation with zero initialization
 */
static void* _zalloc(size_t size)
{
    return memset(malloc(size), 0, size);
}


void* operator new (std::size_t size)
{
    void* p = vs_malloc(size);
    
    if (nullptr == p)
    {
        throw Common::VS_EXCEPTION("Memory allocation failed!");
    }
    
    return p;
}


void* operator new[](std::size_t size)
{
    void* p = vs_malloc(size);

    if (nullptr == p)
    {
        throw Common::VS_EXCEPTION("Memory allocation failed!");
    }

    return p;
}


void operator delete (void* ptr) noexcept
{
    if (nullptr != ptr)
    {
        vs_free(ptr);
    }
}


void operator delete[](void* ptr) noexcept
{
    if (nullptr != ptr)
    {
        vs_free(ptr);
    }
}


vs_malloc_t  vs_malloc  { malloc  };
vs_realloc_t vs_realloc { realloc };
vs_free_t    vs_free    { free    };
vs_zalloc_t  vs_zalloc  { _zalloc };

vs_log_t  vs_log        { _vs_log };
int       vs_log_level  { 4       };

int       VS_ERROR_TYPE { 0       };
int       VS_WARN_TYPE  { 1       };
int       VS_INFO_TYPE  { 2       };
int       VS_MSG_TYPE   { 3       };

int       VS_ERROR      { -1      };
int       VS_OK         {  0      };
