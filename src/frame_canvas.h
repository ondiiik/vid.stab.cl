/*
 * canvas.h
 *
 *  Created on: 28. 9. 2018
 *      Author: ondiiik
 */
#pragma once


#include "common_vect.h"
#include "common_exception.h"
#include <cstdint>
#include <cmath>
#include <cstring>
#include <iostream> // DEBUG


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
        
        
        Canvas(const Common::Vect<unsigned>& aDim)
            :
            _dim     { aDim                  },
            _buf     { new pix_t[aDim.dim()] },
        _autobuf { true                  }
        {
            if (nullptr == _buf)
            {
                throw Common::VS_EXCEPTION_M("FrmCanvas", "Memory allocation failed!");
            }
        }
        
        
        Canvas(unsigned aWidth,
               unsigned aHeight)
            :
            _dim     { aWidth, aHeight       },
            _buf     { new pix_t[_dim.dim()] },
        _autobuf { true                  }
        {
            if (nullptr == _buf)
            {
                throw Common::VS_EXCEPTION_M("FrmCanvas", "Memory allocation failed!");
            }
        }
        
        
        Canvas(pix_t*                        aBuf,
               const Common::Vect<unsigned>& aDim)
            :
            _dim     { aDim  },
            _buf     { aBuf  },
            _autobuf { false }
        {
            if (nullptr == _buf)
            {
                throw Common::VS_EXCEPTION_M("FrmCanvas", "Null canvas buffer!");
            }
        }
        
        
        Canvas(pix_t*   aBuf,
               unsigned aWidth,
               unsigned aHeight)
            :
            _dim     { aWidth, aHeight },
            _buf     { aBuf            },
            _autobuf { false           }
        {
            if (nullptr == _buf)
            {
                throw Common::VS_EXCEPTION_M("FrmCanvas", "Null canvas buffer!");
            }
        }
        
        
        virtual ~Canvas()
        {
            if (_autobuf)
            {
                delete _buf;
            }
        }
        
        
        inline const pix_t& operator[](const Common::Vect<unsigned>& aPixel) const
        {
            return (*this)(aPixel.x, aPixel.y);
        }
        
        
        inline pix_t& operator[](const Common::Vect<unsigned>& aPixel)
        {
            return (*this)(aPixel.x, aPixel.y);
        }
        
        
        inline const pix_t& operator()(const Common::Vect<unsigned>& aPixel) const
        {
            return (*this)(aPixel.x, aPixel.y);
        }
        
        
        inline pix_t& operator()(const Common::Vect<unsigned>& aPixel)
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
        void drawBox(const Common::Vect<unsigned>&   aPos,
                     const Common::Vect<unsigned>&   aSize,
                     pix_t                           aColor)
        {
            const Common::Vect<unsigned> sz2 = aSize / 2;
            Common::Vect<unsigned>       i;

            
            for (i.y = 0; i.y < aSize.y; ++i.y)
            {
                for (i.x = 0; i.x < aSize.x; ++i.x)
                {
                    Common::Vect<unsigned> pos { i + aPos - sz2};
                    
                    unsigned  c = (*this)[pos].abs() * 3;
                    c          += aColor.abs();
                    c          /= 4;
                    
                    (*this)[pos] = c;
                }
            }
        }
        
        
        /**
         * @brief   draws a line from a to b
         *
         * with given thickness(not filled) at the given position x,y (center)
         * in the given color at the first channel
         */
        void drawLine(const Common::Vect<unsigned>& aPix1,
                      const Common::Vect<unsigned>& aPix2,
                      int                           aThickness,
                      pix_t                         aColor)
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
                    for (int k { 0 }; k <= div.x; k++)
                    {
                        (*this)(pix1.x + k, pix1.y + r) = aColor;
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
                    for (int k { 0 }; k <= div.y; ++k)
                    {
                        (*this)(pix1.x + r, pix1.y + k) = aColor;
                    }
                }
                
                return;
            }
            
            
            double   m      = (double)div.x / (double)div.y;
            unsigned horlen = aThickness + fabs(m);
            
            for (int c = 0; c <= abs(div.y); c++)
            {
                int      dy { (div.y < 0) ? -c : c                     };
                unsigned x  { aPix1.x + unsigned(m * dy) - horlen / 2U };
                
                for (unsigned k = 0; k <= horlen; k++)
                {
                    (*this)(x + k, aPix1.y + dy) = aColor;
                }
            }
        }
        
        
        /**
         * draws a rectangle (not filled) at the given position x,y (center) in the given color
         at the first channel
        */
        void drawRectangle(const Common::Vect<unsigned>&   aPos,
                           const Common::Vect<unsigned>&   aSize,
                           pix_t                           color)
        {
            const Common::Vect<unsigned> sz2 = aSize / 2;
            
            for (unsigned k = 0; k < aSize.x; ++k)
            {
                (*this)(aPos.x - sz2.x + k, aPos.y - sz2.y) = color;
            }
            
            for (unsigned k = 0; k <= aSize.x; ++k)
            {
                (*this)(aPos.x - sz2.x + k, aPos.y + sz2.y) = color;
            }
            
            for (unsigned k = 0; k < aSize.y; ++k)
            {
                (*this)(aPos.x - sz2.x, aPos.y - sz2.y + k) = color;
            }
            
            for (unsigned k = 0; k < aSize.y; ++k)
            {
                (*this)(aPos.x + sz2.x, aPos.y - sz2.y + k) = color;
            }
        }
        
        
        
    private:
        inline bool _inside(unsigned aX,
                            unsigned aY) const noexcept
        {
            return (aX < _dim.x) && (aY < _dim.y);
        }
        
        
        pix_t                        _wnull;    /**< @brief Reference on writable null pixel */
        const pix_t                  _rnull;    /**< @brief Reference on read only null pixel */
        
        
        const Common::Vect<unsigned> _dim;      /**< @brief Canvas dimensions */
        pix_t*                 const _buf;      /**< @brief Canvas buffer */
        bool                   const _autobuf;  /**< @brief Automatically allocated buffer flag */
    };
}
