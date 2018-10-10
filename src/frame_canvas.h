/*
 * canvas.h
 *
 *  Created on: 28. 9. 2018
 *      Author: ondiiik
 */
#pragma once


#include "common_vect.h"
#include <cstdint>
#include <cmath>
#include <cstring>

#include "common_exception.h"


namespace Frame
{
    /**
     * @brief   Represents painting canvas
     */
    template <typename _Pix> class Canvas
    {
    public:
        /**
         * @brief   Pixel type
         *
         * Depends on frame type (RGB, YUV, ...)
         */
        typedef _Pix pix_t;
        
        
        Canvas(pix_t*                        aBuf,
               const Common::Vect<unsigned>& aDim)
            :
            _dim    { aDim },
            _buf    { aBuf }
        {
        
        }
        
        
        Canvas(pix_t*   aBuf,
               unsigned aWidth,
               unsigned aHeight)
            :
            _dim    { aWidth, aHeight },
            _buf    { aBuf            }
        {
        
        }
        
        
        inline const pix_t& operator[](const Vec& aPixel) const
        {
            return (*this)(aPixel.x, aPixel.y);
        }
        
        
        inline pix_t& operator[](const Vec& aPixel)
        {
            return (*this)(aPixel.x, aPixel.y);
        }
        
        
        inline const pix_t& operator()(const Vec& aPixel) const
        {
            return (*this)(aPixel.x, aPixel.y);
        }
        
        
        inline pix_t& operator()(const Vec& aPixel)
        {
            return (*this)(aPixel.x, aPixel.y);
        }
        
        
        inline const pix_t& operator()(unsigned aX,
                                       unsigned aY) const
        {
            if (_inside(aX, aY))
            {
                return *(_buf + (aY * _dim.x + aX));
            }
            else
            {
                return _rnull;
            }
        }
        
        
        inline pix_t& operator()(unsigned aX,
                                 unsigned aY)
        {
            if (_inside(aX, aY))
            {
                return *(_buf + (aY * _dim.x + aX));
            }
            else
            {
                _wnull = _rnull;
                return   _wnull;
            }
        }
        
        
        Canvas& operator=(const Canvas& aSrc)
        {
            if (_dim != aSrc._dim)
            {
                throw Common::VS_EXCEPTION_M("FrmCanvas",
                                             "Incorrect base dimension (expected [%i x %i] but got [%i x %i])!",
                                             width(),
                                             height(),
                                             aSrc.width(),
                                             aSrc.height());
            }
            
            _dim = aSrc._dim;
            memcpy(_buf, aSrc._buf, sizeof(_buf[0]) * _dim.dim());
            return *this;
        }
        
        
        inline const Common::Vect<unsigned> dim() const noexcept
        {
            return _dim;
        }
        
        
        inline unsigned width() const noexcept
        {
            return _dim.x;
        }
        
        
        inline unsigned height() const noexcept
        {
            return _dim.y;
        }
        
        
        /**
         * @brief   Draws a box at the given position x,y (center) in the given color
         *
         *(the same for all channels)
         */
        void drawBox(const Vec&   aPos,
                     const Vec&   aSize,
                     pix_t        aColor)
        {
            pix_t* p { &(*this)[aPos - (aSize / 2)] };
            
            for (int j = 0; j < aSize.y; ++j)
            {
                for (int k = 0; k < aSize.x; ++k)
                {
                    *p = aColor;
                    p++;
                }
                
                p += (_dim.x - aSize.x);
            }
        }
        
        
        /**
         * @brief   draws a line from a to b
         *
         * with given thickness(not filled) at the given position x,y (center)
         * in the given color at the first channel
         */
        void drawLine(const Vec& aPix1,
                      const Vec& aPix2,
                      int        aThickness,
                      pix_t      aColor)
        {
            Vec div { aPix2 - aPix1 };
            
            if (div.y == 0) // horizontal line
            {
                Vec pix1 { aPix1 };
                
                if (div.x < 0)
                {
                    pix1   =  aPix2;
                    div.x *= -1;
                }
                
                int th2 { aThickness / 2 };
                
                for (int r { -th2 }; r <= th2; ++r)
                {
                    pix_t* p { &((*this)(pix1.x, pix1.y + r)) };
                    
                    for (int k { 0 }; k <= div.x; k++)
                    {
                        *p = aColor;
                        ++p;
                    }
                }
                
                return;
            }
            
            if (div.x == 0) // vertical line
            {
                Vec pix1 { aPix1 };
                
                if (div.y < 0)
                {
                    pix1   = aPix2;
                    div.y *= -1;
                }
                
                int th2    { aThickness / 2 };
                
                for (int r { -th2 }; r <= th2; ++r)
                {
                    pix_t* p  { &((*this)(pix1.x + r, pix1.y)) };
                    
                    for (int k { 0 }; k <= div.y; ++k)
                    {
                        *p = aColor;
                        p += _dim.x;
                    }
                }
                
                return;
            }
            
            
            double m      = (double)div.x / (double)div.y;
            int    horlen = aThickness + fabs(m);
            
            for (int c = 0; c <= abs(div.y); c++)
            {
                int    dy { (div.y < 0) ? -c : c               };
                int    x  { aPix1.x + int(m * dy) - horlen / 2 };
                pix_t* p  { &((*this)(x, aPix1.y + dy))        };
                
                for (int k = 0; k <= horlen; k++)
                {
                    *p = aColor;
                    ++p;
                }
            }
        }
        
        
        /**
         * draws a rectangle (not filled) at the given position x,y (center) in the given color
         at the first channel
        */
        void drawRectangle(int   x,
                           int   y,
                           int   sizex,
                           int   sizey,
                           pix_t color)
        {
            unsigned szX2 = sizex / 2;
            unsigned szY2 = sizey / 2;
            pix_t*   p    = &((*this)(x - szX2, y - szY2));
            
            for (int k = 0; k < sizex; k++)
            {
                *p = color;    // upper line
                ++p;
            }
            
            
            p = &((*this)(x - szX2, y + szY2));
            
            for (int k = 0; k < sizex; k++)
            {
                *p = color;    // lower line
                ++p;
            }
            
            
            p = &((*this)(x - szX2, y - szY2));
            
            for (int k = 0; k < sizey; k++)
            {
                *p = color;    // left line
                p += _dim.x;
            }
            
            
            p = &((*this)(x + szX2, y - szY2));
            
            for (int k = 0; k < sizey; k++)
            {
                *p = color;    // right line
                p += _dim.x;
            }
        }
        
        
        
    private:
        inline bool _inside(unsigned aX,
                            unsigned aY) const noexcept
        {
            return (aX < _dim.x) && (aY < _dim.y);
        }
        
        
        const Common::Vect<unsigned> _dim;    /**< @brief Canvas dimensions */
        pix_t                        _wnull;  /**< @brief Reference on writable null pixel */
        const pix_t                  _rnull;  /**< @brief Reference on read only null pixel */
        
        
    protected:
        pix_t*                 const _buf;          /**< @brief Canvas buffer */
    };
}
