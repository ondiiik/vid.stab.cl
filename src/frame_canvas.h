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

#include "common_exception.h"


namespace Frame
{
    /**
     * @brief   Represents painting canvas
     */
    template <typename _Pix> class Canvas
    {
    public:
        typedef _Pix pix_t;
        
        
        Canvas(pix_t*   aBuf,
               unsigned aWidth,
               unsigned aHeight)
            :
            dim    { aWidth, aHeight },
            buf    { aBuf            }
        {
        
        }
        
        
        inline pix_t& operator[](const Vec& aPixel) const
        {
            return (*this)(aPixel.x, aPixel.y);
        }
        
        
        inline pix_t& operator()(const Vec& aPixel) const
        {
            return (*this)(aPixel.x, aPixel.y);
        }
        
        
        inline pix_t& operator()(int aX,
                                 int aY) const
        {
            return *(buf + (aY * dim.x + aX));
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
            if (_invalidRange(aPos))
            {
                throw Common::VS_EXCEPTION_M(canvas, "Really so big coordinates (%i x %i)?", aPos.x, aPos.y);
            }
            
            if (_invalidRange(aPos))
            {
                throw Common::VS_EXCEPTION_M(canvas, "Really that big size (%i x %i)?", aSize.x, aSize.y);
            }
            
            
            pix_t* p { &((*this)(aPos - (aSize / 2))) };
            
            for (int j = 0; j < aSize.y; ++j)
            {
                for (int k = 0; k < aSize.x; ++k)
                {
                    *p = aColor;
                    p++;
                }
                
                p += (dim.x - aSize.x);
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
                        p += dim.x;
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
            if (_invalidRange(x, y))
            {
                throw Common::VS_EXCEPTION_M(canvas, "Really so big coordinates (%i x %i)?", x, y);
            }
            
            
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
                p += dim.x;
            }
            
            
            p = &((*this)(x + szX2, y - szY2));
            
            for (int k = 0; k < sizey; k++)
            {
                *p = color;    // right line
                p += dim.x;
            }
        }
        
        
        
        
        const Common::Vect<unsigned> dim;   /**< @brief Canvas dimensions */
        pix_t*                 const buf;   /**< @brief Canvas buffer */
        
        
    private:
        static inline bool _invalidRange(int aX, int aY)
        {
            return ((16384 < aX) ||
                    (16384 < aY) ||
                    (0     > aX) ||
                    (0     > aY));
                    
        }
        
        
        static inline bool _invalidRange(const Vec& aV)
        {
            return ((16384 < aV.x) ||
                    (16384 < aV.y) ||
                    (0     > aV.x) ||
                    (0     > aV.y));
                    
        }
    };
}
