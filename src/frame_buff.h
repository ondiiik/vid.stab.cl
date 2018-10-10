/*
 * vs_frame_buff.h
 *
 *  Created on: 8. 10. 2018
 *      Author: ondiiik
 */
#pragma once


#include "frame_canvas.h"
#include "common_exception.h"
#include <iostream> // DEBUG


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
    template <typename _PixT, typename _NumT = unsigned> class PixIfc;
    
    
    template <typename _NumT> class PixIfc<PixYUV, _NumT>
    {
    public:
        typedef _NumT num_t;
        
        
        PixIfc() noexcept
            :
            _px
        {
            num_t(0U)
        }
        {
        
        }
        
        
        PixIfc(num_t aBwColor) noexcept
            :
            _px
        {
            num_t(aBwColor)
        }
        {
        
        }
        
        
        PixIfc(const PixYUV& aSrc) noexcept
            :
            _px
        {
            num_t(aSrc[0])
        }
        {
        
        }
        
        
        inline const num_t& operator[](unsigned aIdx) const noexcept
        {
            return this->_px;
        }
        
        
        inline num_t& operator[](unsigned aIdx) noexcept
        {
            return this->_px;
        }
        
        
        template<typename _SrcT> inline PixIfc& operator+=(const _SrcT& aSrc)
        {
            _px += num_t(aSrc[0]);
            return *this;
        }
        
        
    private:
        num_t _px;
    };
    
    
    template <typename _NumT> class PixIfc<PixRGB, _NumT>
    {
    public:
        typedef _NumT num_t;
        
        
        PixIfc() noexcept
            :
            _px
        {
            num_t(0), num_t(0), num_t(0)
        }
        {
        
        }
        
        
        PixIfc(unsigned aBwColor) noexcept
            :
            _px
        {
            num_t(aBwColor), num_t(aBwColor), num_t(aBwColor)
        }
        {
        
        }
        
        
        PixIfc(const PixRGB& aSrc) noexcept
            :
            _px
        {
            num_t(aSrc[0]), num_t(aSrc[1]), num_t(aSrc[2])
        }
        {
        
        }
        
        
        inline const num_t& operator[](unsigned aIdx) const noexcept
        {
            if (aIdx >= __Pix_RGB_CNT)
            {
                aIdx %= __Pix_RGB_CNT;
            }
            
            return _px[aIdx];
        }
        
        
        inline num_t& operator[](unsigned aIdx) noexcept
        {
            if (aIdx >= __Pix_RGB_CNT)
            {
                aIdx %= __Pix_RGB_CNT;
            }
            
            return _px[aIdx];
        }
        
        
        template<typename _SrcT> inline PixIfc& operator+=(const _SrcT& aSrc)
        {
            _px[0] += num_t(aSrc[0]);
            _px[1] += num_t(aSrc[1]);
            _px[2] += num_t(aSrc[2]);
            return *this;
        }
        
        
    private:
        num_t _px[__Pix_RGB_CNT];
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
        
        
        inline Layer& operator=(const Canvas<pix_t>& aSrc)
        {
            std::cout << "[VIDSTAB DBG] copy Layer[<?>:" << this->width() << "x" << this->height() << "] <-- Canvas[<?>:" << aSrc.width() << "x" << aSrc.height() << "]\n";
            Canvas<_PixT>::operator=(aSrc);
            return *this;
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
