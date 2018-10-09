/*
 * vs_sift_piramid.h
 *
 *  Created on: 8. 10. 2018
 *      Author: ondiiik
 */
#pragma once


#include "frame_buff.h"
#include <vector>
#include <iostream> // DEBUG


namespace Frame
{
    template <typename _Pix> class Pyramid
    {
    public:
        Pyramid(unsigned aWidth,
                unsigned aHeight,
                unsigned aMin)
            :
            Pyramid(Common::Vect<unsigned>(aWidth, aHeight), aMin)
        {
        
        }
        
        
        Pyramid(const Common::Vect<unsigned>& aDim,
                unsigned                      aMin)
        {
            for (auto* b = new Layer<_Pix>(aDim); (b->width() > aMin) && (b->height() > aMin); b = new Layer<_Pix>(b->dim() / 2))
            {
                std::cout << "[VIDSTAB DBG] " << (void*)this << " PYRAMID " << b->width() << " x " << b->height() << std::endl;
                _pyramid.push_back(b);
            }
        }
        
        
        virtual ~Pyramid()
        {
            for (auto* b : _pyramid)
            {
                delete b;
            }
        }
        
        
        /**
         * @brief   Return pyramid elements count
         */
        inline unsigned size() const noexcept
        {
            return unsigned(_pyramid.size());
        }
        
        
    private:
        std::vector<Layer<_Pix>*> _pyramid;
    };
}
