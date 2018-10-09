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
    /**
     * @brief   YUV layers indexes
     */
    enum PixYUV_Layers
    {
        Pix_Y,
        Pix_U,
        Pix_V,
        __Pix_YUV_CNT
    };
    
    
    /**
     * @brief   RGB colors indexes
     */
    enum PixRGB_Layers
    {
        Pix_R,
        Pix_G,
        Pix_B,
        __Pix_RGB_CNT
    };
    
    
#   pragma pack(push, 1)
    /**
     * @brief   YUV pixel interface
     */
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
    
    
    /**
     * @brief   RGB pixel interface
     */
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
    
    
    /**
     * @brief   BW pixel abstract factory
     * @tparam  \_PixT      Pixel type
     * @param   aBwColor    BW color in range 0 .. 256
     * @param   aLayerIdx   Pixel layer
     * @return  Value for selected layer
     */
    template <typename _PixT> inline _PixT Pix(unsigned aBwColor,
                                               unsigned aLayerIdx);
                                               
                                               
    /**
     * @brief   BW pixel YUV factory
     * @param   aBwColor    BW color in range 0 .. 256
     * @param   aLayerIdx   Pixel layer
     * @return  Value for selected layer
     */
    template <> inline PixYUV Pix<PixYUV>(unsigned aBwColor,
                                          unsigned aLayerIdx)
    {
        return PixYUV(aBwColor);
    }
    
    
    /**
     * @brief   BW pixel RGB factory
     * @param   aBwColor    BW color in range 0 .. 256
     * @param   aLayerIdx   Pixel layer
     * @return  Value for selected layer
     */
    template <> inline PixRGB Pix<PixRGB>(unsigned aBwColor,
                                          unsigned aLayerIdx)
    {
        return PixRGB(aBwColor);
    }
    
    
    /**
     * @brief   One single layer abstract factory
     * @tparam  \_PixT  Pixel type
     */
    template <typename _PixT> class Layer
        :
        public Canvas<_PixT>
    {
    public:
        Layer(const Common::Vect<unsigned>& aDim)
            :
            Canvas<_PixT>
        {
            new _PixT[aDim.dim()], aDim
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
    
    
    /**
     * @brief   YUV pixels buffer type
     */
    struct BuffYUV
    {
        BuffYUV(const Common::Vect<unsigned>& aDim)
            :
            m { aDim, aDim, aDim }
        {
        
        }
        
        Layer<PixYUV> m[__Pix_YUV_CNT];
    };
    
    
    /**
     * @brief   RGB pixels buffer type
     */
    struct BuffRGB
    {
        BuffRGB(const Common::Vect<unsigned>& aDim)
            :
            m { aDim }
        {

        }

        Layer<PixRGB> m[1];
    };
}
