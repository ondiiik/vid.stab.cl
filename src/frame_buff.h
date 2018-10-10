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
    
    
#pragma pack(push, 1)
    /**
     * @brief   YUV packed pixel container
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
        
        
        inline const uint8_t& operator[](unsigned aIdx) const noexcept
        {
            return _px;
        }
        
        
        inline uint8_t& operator[](unsigned aIdx) noexcept
        {
            return _px;
        }
        
        
    private:
        uint8_t _px;
    };
    
    
    /**
     * @brief   RGB packed pixel container
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
        
        
        inline const uint8_t& operator[](unsigned aIdx) const noexcept
        {
            if (aIdx >= __Pix_RGB_CNT)
            {
                aIdx %= __Pix_RGB_CNT;
            }
            
            return _px[aIdx];
        }
        
        
        inline uint8_t& operator[](unsigned aIdx) noexcept
        {
            if (aIdx >= __Pix_RGB_CNT)
            {
                aIdx %= __Pix_RGB_CNT;
            }
            
            return _px[aIdx];
        }
        
        
    private:
        uint8_t _px[__Pix_RGB_CNT];
    };
#pragma pack(pop)
    
    
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
     * @brief   Pixel operation interface
     */
    template <typename _PixT, typename _numT = unsigned> class PixIfc;
    
    
    template <> class PixIfc<PixYUV>
    {
    public:
        PixIfc() noexcept
            :
            _px
        {
            0U
        }
        {
        
        }
        
        
        PixIfc(unsigned aBwColor) noexcept
            :
            _px
        {
            aBwColor
        }
        {
        
        }
        
        
        PixIfc(const PixYUV& aSrc) noexcept
            :
            _px
        {
            aSrc[0]
        }
        {
        
        }
        
        
    private:
        unsigned _px;
    };
    
    
    template <> class PixIfc<PixRGB>
    {
    public:
        PixIfc() noexcept
            :
            _px
        {
            0U, 0U, 0U
        }
        {
        
        }
        
        
        PixIfc(unsigned aBwColor) noexcept
            :
            _px
        {
            aBwColor, aBwColor, aBwColor
        }
        {
        
        }
        
        
        PixIfc(const PixRGB& aSrc) noexcept
            :
            _px
        {
            aSrc[0], aSrc[1], aSrc[2]
        }
        {
        
        }
        
        
    private:
        unsigned _px[__Pix_RGB_CNT];
    };
    
    
    /**
     * @brief   One single layer abstract factory
     * @tparam  \_PixT  Pixel type
     */
    template <typename _PixT> class Layer
        :
        public Canvas<_PixT>
    {
    public:
        typedef _PixT pix_t;
        
        
        Layer(const Common::Vect<unsigned>& aDim)
            :
            Canvas<pix_t>
        {
            new pix_t[aDim.dim()], aDim
        }
        {
            if (nullptr == this->_buf)
            {
                throw Common::VS_EXCEPTION_M("FrmBuf", "Memory allocation failed!");
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
