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
        
        
        inline const uint8_t abs() const noexcept
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
        
        
        PixRGB(unsigned aR,
               unsigned aG,
               unsigned aB) noexcept
            :
            _px
        {
            uint8_t(aR), uint8_t(aG), uint8_t(aB)
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
        
        
        inline const uint8_t abs() const noexcept
        {
            return (unsigned(_px[0]) + unsigned(_px[1]) + unsigned(_px[2])) / 3U;
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
        
        
        inline operator PixYUV() const noexcept
        {
            return PixYUV(this->_px[0]);
        }
        
        
        inline const num_t& operator[](unsigned aIdx) const noexcept
        {
            return this->_px[0];
        }
        
        
        inline num_t& operator[](unsigned aIdx) noexcept
        {
            return this->_px[0];
        }
        
        
        template<typename _SrcT> inline PixIfc& operator+=(const _SrcT& aSrc)
        {
            _px[0] += num_t(aSrc[0]);
            return *this;
        }
        
        
        template<typename _SrcT> inline PixIfc& operator-=(const _SrcT& aSrc)
        {
            _px[0] += num_t(aSrc[0]);
            return *this;
        }
        
        
        template<typename _SrcT> inline PixIfc& operator/=(const _SrcT& aSrc)
        {
            _px[0] /= num_t(aSrc);
            return *this;
        }
        
        
        template<typename _SrcT> inline PixIfc& operator*=(const _SrcT& aSrc)
        {
            _px[0] *= num_t(aSrc);
            return *this;
        }
        
        
    private:
        num_t _px[1];
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
        
        
        inline operator PixRGB() const noexcept
        {
            return PixRGB(this->_px[0],
                          this->_px[1],
                          this->_px[2]);
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
        
        
        template<typename _SrcT> inline PixIfc& operator-=(const _SrcT& aSrc)
        {
            _px[0] += num_t(aSrc[0]);
            _px[1] += num_t(aSrc[1]);
            _px[2] += num_t(aSrc[2]);
            return *this;
        }
        
        
        template<typename _SrcT> inline PixIfc& operator/=(const _SrcT& aSrc)
        {
            _px[0] /= num_t(aSrc);
            _px[1] /= num_t(aSrc);
            _px[2] /= num_t(aSrc);
            return *this;
        }
        
        
        template<typename _SrcT> inline PixIfc& operator*=(const _SrcT& aSrc)
        {
            _px[0] *= num_t(aSrc);
            _px[1] *= num_t(aSrc);
            _px[2] *= num_t(aSrc);
            return *this;
        }
        
        
    private:
        num_t _px[__Pix_RGB_CNT];
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
        
        Canvas<PixYUV> m[__Pix_YUV_CNT];
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
        
        Canvas<PixRGB> m[1];
    };
}
