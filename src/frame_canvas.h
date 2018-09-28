/*
 * canvas.h
 *
 *  Created on: 28. 9. 2018
 *      Author: ondiiik
 */
#pragma once


#include "common_vect.h"
#include <cstdint>


namespace Frame
{
    /**
     * @brief   Represents painting canvas
     */
    class Canvas
    {
    public:
        Canvas(uint8_t* aBuf,
                int      aWidth,
                int      aHeight,
                int      aBpp)
            :
            dim    { aWidth, aHeight },
            buf    { aBuf            },
            bpp    { aBpp            }
        {
        
        }
        
        
        inline uint8_t* operator()(const Vec& aPixel) const
        {
            return (*this)(aPixel.x, aPixel.y);
        }
        
        
        uint8_t* operator()(int aX,
                            int aY) const
        {
            return buf + ((aY * dim.x + aX) * bpp);
        }
        
        
        /**
         * @brief   Draws a box at the given position x,y (center) in the given color
         *
         *(the same for all channels)
         */
        void drawBox(const Vec&  aPos,
                     const Vec&  aSize,
                     uint8_t     color)
        {
            uint8_t* p { (*this)(aPos - (aSize / 2)) };
            
            for (int j = 0; j < aSize.y; ++j)
            {
                for (int k = 0; k < aSize.x * bpp; ++k)
                {
                    *p = color;
                    p++;
                }
                
                p += (dim.x - aSize.x) * bpp;
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
                      uint8_t    aColor)
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
                    uint8_t* p  { (*this)(pix1.x, pix1.y + r) };
                    
                    for (int k { 0 }; k <= div.x; k++)
                    {
                        *p = aColor;
                        p += bpp;
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
                int bwidth { dim.x * bpp   };
                
                for (int r { -th2 }; r <= th2; ++r)
                {
                    uint8_t* p  { (*this)(pix1.x + r, pix1.y) };
                    
                    for (int k { 0 }; k <= div.y; ++k)
                    {
                        *p = aColor;
                        p += bwidth;
                    }
                }
                
                return;
            }
            
            
            double m      = (double)div.x / (double)div.y;
            int    horlen = aThickness + fabs(m);
            
            for (int c = 0; c <= abs(div.y); c++)
            {
                int      dy { (div.y < 0) ? -c : c               };
                int      x  { aPix1.x + int(m * dy) - horlen / 2 };
                uint8_t* p  { (*this)(x, aPix1.y + dy)           };
                
                for (int k = 0; k <= horlen; k++)
                {
                    *p = aColor;
                    p += bpp;
                }
            }
        }
        
        
        const Common::Vect<int> dim;    /**< @brief Canvas dimensions */
        uint8_t* const          buf;    /**< @brief Canvas buffer */
        int      const          bpp;    /**< @brief Bytes per pixel */
    };
}
