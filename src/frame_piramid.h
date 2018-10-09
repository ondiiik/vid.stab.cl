/*
 * vs_sift_piramid.h
 *
 *  Created on: 8. 10. 2018
 *      Author: ondiiik
 */
#pragma once


#include <vector>

#include "frame_buff.h"


namespace Frame
{
    template <typename _Pix> class Piramid
    {
    public:
        Piramid(unsigned aWidth,
                unsigned aHeight,
                unsigned aMin)
            :
            Piramid(Common::Vect<unsigned>(aWidth, aHeight), aMin)
        {
        
        }
        
        
        Piramid(const Common::Vect<unsigned>& aDim,
                unsigned                      aMin)
        {
            for (auto* b = new Layer<_Pix>(aDim); (b->width() > aMin) && (b->height() > aMin); b = new Layer<_Pix>(b->dim() / 2))
            {
                _piramid.push_back(b);
            }
        }
        
        
        virtual ~Piramid()
        {
            for (auto* b : _piramid)
            {
                delete b;
            }
        }
        
        
    private:
        std::vector<Layer<_Pix>*> _piramid;
    };
}
