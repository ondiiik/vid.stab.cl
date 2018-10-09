/*
 * vs_frame_buff.h
 *
 *  Created on: 8. 10. 2018
 *      Author: ondiiik
 */
#pragma once


#include "frame_canvas.h"
#include "common_exception.h"


namespace Frame
{
    enum PixYUV_Layers
    {
        Pix_Y,
        Pix_U,
        Pix_V,
        __Pix_YUV_CNT
    };
    
    
    enum PixRGB_Layers
    {
        Pix_R,
        Pix_G,
        Pix_B,
        __Pix_RGB_CNT
    };
    
    
#   pragma pack(push, 1)
    class PixYUV
    {
    public:
        PixYUV() noexcept
            :
            _px
        {
            uint8_t(0)
        }
        {
        
        }
        
        
        PixYUV(unsigned aBwColor) noexcept
            :
            _px
        {
            uint8_t(aBwColor)
        }
        {
        
        }
        
        
    private:
        uint8_t _px;
    };
    
    
    class PixRGB
    {
    public:
        PixRGB() noexcept
            :
            _px
        {
            uint8_t(0), uint8_t(0), uint8_t(0)
        }
        {
        
        }
        
        
        PixRGB(unsigned aBwColor) noexcept
            :
            _px
        {
            uint8_t(aBwColor), uint8_t(aBwColor), uint8_t(aBwColor)
        }
        {
        
        }
        
        
    private:
        uint8_t _px[__Pix_RGB_CNT];
    };
#   pragma pack(pop)
    
    
    
    
    
    template <typename _PixT> inline _PixT Pix(unsigned aBwColor,
                                               unsigned aLayerIdx);
                                               
                                               
    template <> inline PixYUV Pix<PixYUV>(unsigned aBwColor,
                                          unsigned aLayerIdx)
    {
        return PixYUV(aBwColor);
    }
    

    template <> inline PixRGB Pix<PixRGB>(unsigned aBwColor,
                                          unsigned aLayerIdx)
    {
        return PixRGB(aBwColor);
    }
    
    
    
    template <typename _Pix> class Layer
        :
        public Canvas<_Pix>
    {
    public:
        Layer(const Common::Vect<unsigned>& aDim)
            :
            Canvas<_Pix>
        {
            new _Pix[aDim.dim()], aDim
        }
        {
            if (nullptr == this->_buf)
            {
                throw Common::VS_EXCEPTION_M("FramBuf", "Memory allocation failed!");
            }
        }
        
        
        virtual ~Layer() noexcept
        {
            delete this->_buf;
        }
    };
    
    
    typedef Canvas<PixYUV> BuffYUV[__Pix_YUV_CNT];
    
    
    typedef Canvas<PixRGB> BuffRGB;
}
