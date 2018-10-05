/*
 *  frameinfo.cpp
 *
 *  Copyright (C) Georg Martius - Feb - 2013
 *   georg dot martius at web dot de
 *
 *  This file is part of vid.stab video stabilization library
 *
 *  vid.stab is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License,
 *  as published by the Free Software Foundation; either version 2, or
 *  (at your option) any later version.
 *
 *  vid.stab is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with GNU Make; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */
#include "frameinfo.h"
#include "vidstabdefines.h"
#include "common_cutils.h"
#include <cassert>
#include <cstring>

#include "common_exception.h"


namespace
{
    const char moduleName[] { "frame" };
}


namespace Frame
{
    Frame::Frame(VSFrame&    aFrame,
                 const Info& aInfo)
        :
        _frame { aFrame },
        _info  { aInfo  }
    {
        _init();
    }
    
    
    Frame::Frame(const VSFrame& aFrame,
                 const Info&    aInfo)
        :
        _frame { const_cast<VSFrame&>(aFrame) },
        _info  { aInfo                        }
    {
        _init();
    }
    
    
    void Frame::_init()
    {
        if (_info.planes() > int(COMMON_ARRAY_ITEMS_CNT(_frame.data)))
        {
            throw Common::VS_EXCEPTION("Too many planes (%i)! Maximum is %i!",
                                        _info.planes(),
                                        int(COMMON_ARRAY_ITEMS_CNT(_frame.data)));
        }
    }
    
    
    void Frame::alloc()
    {
        forget();
        
        int planes = _info.planes();
        
        for (int i = 0; i < planes; ++i)
        {
            auto size = _info.calcSize(i);
            
            if (0 == size)
            {
                throw Common::VS_EXCEPTION("Unexpected plane %i!", i);
            }
            
            _frame.data[i]     = new uint8_t[size];
            _frame.linesize[i] = _info.calcLineWidth(i);
            memset(_frame.data[i], 0, size);
        }
    }
    
    
    void Frame::free()
    {
        int planes = _info.planes();
        
        for (int i = 0; i < planes; ++i)
        {
            delete _frame.data[i];
            _frame.data[i]     = nullptr;
            _frame.linesize[i] = 0;
        }
    }
    
    
    void Frame::forget()
    {
        memset(&_frame, 0, sizeof(_frame));
    }
    
    
    Frame& Frame::operator =(const Frame& aSrc)
    {
        if (empty())
        {
            throw Common::VS_EXCEPTION("Can not copy to empty frame!");
        }
        
        if (_info != aSrc._info)
        {
            throw Common::VS_EXCEPTION("Copy of incompatible frames!");
        }
        
        
        for (int i = 0, e = _info.planes(); i < e; ++i)
        {
            auto size = _info.calcSize(i);
            memcpy(_frame.data[i], aSrc._frame.data[i], size);
        }
        
        return *this;
    }
    
    
    bool Frame::operator ==(const Frame& aSrc)
    {
        return (&_frame == &aSrc._frame) || (_frame.data[0] == aSrc._frame.data[0]);
    }
    
    
    Plane& Plane::operator =(const Plane& aSrc)
    {
        if (nullptr == _buff)
        {
            throw Common::VS_EXCEPTION("Can not copy to null plane!");
        }
        
        if (nullptr == aSrc._buff)
        {
            throw Common::VS_EXCEPTION("Can not copy from null plane!");
        }
        
        if (_frame.info() == aSrc._frame.info())
        {
            memcpy(_buff, aSrc._buff, _lineSize * _frame.info().height());
        }
        else
        {
            throw Common::VS_EXCEPTION("Incompatible planes to copy!");
        }
        
        return *this;
    }
    
    
    std::size_t Info::calcSize(unsigned aPlane) const
    {
        if (pixFormat() < PF_PACKED)
        {
            if (aPlane < 4)
            {
                return sublineWidth(aPlane) * sublineHeight(aPlane);
            }
            else
            {
                return 0;
            }
        }
        else
        {
            if (aPlane < 1)
            {
                return (_info.width * _info.height * bpp());
            }
            else
            {
                return 0;
            }
        }
    }
}


int vsFrameInfoInit(VSFrameInfo* fi, int width, int height, VSPixelFormat pFormat)
{
    fi->pFormat       = pFormat;
    fi->width         = width;
    fi->height        = height;
    fi->planes        = 3;
    fi->log2ChromaW   = 0;
    fi->log2ChromaH   = 0;
    fi->bytesPerPixel = 1;
    
    assert(((width % 2) == 0) && ((height % 2) == 0));
    
    
    switch (pFormat)
    {
        case PF_GRAY8:
            fi->planes = 1;
            break;
            
        case PF_YUV420P:
            fi->log2ChromaW = 1;
            fi->log2ChromaH = 1;
            break;
            
        case PF_YUV422P:
            fi->log2ChromaW = 1;
            fi->log2ChromaH = 0;
            break;
            
        case PF_YUV444P:
            break;
            
        case PF_YUV410P:
            fi->log2ChromaW = 2;
            fi->log2ChromaH = 2;
            break;
            
        case PF_YUV411P:
            fi->log2ChromaW = 2;
            fi->log2ChromaH = 0;
            break;
            
        case PF_YUV440P:
            fi->log2ChromaW = 0;
            fi->log2ChromaH = 1;
            break;
            
        case PF_YUVA420P:
            fi->log2ChromaW = 1;
            fi->log2ChromaH = 1;
            fi->planes      = 4;
            break;
            
        case PF_RGB24:
        case PF_BGR24:
            fi->bytesPerPixel = 3;
            fi->planes        = 0;
            break;
            
        case PF_RGBA:
            fi->bytesPerPixel = 4;
            fi->planes        = 0;
            break;
            
        default:
            fi->pFormat = PF_GRAY8;
            return 0;
    }
    
    return 1;
}
