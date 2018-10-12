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
    template <typename _PixT> class Pyramid
    {
    public:
        typedef _PixT pix_t;


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
            for (auto* b = new Canvas<pix_t>(aDim); (b->width() > aMin) && (b->height() > aMin); b = new Canvas<pix_t>(b->dim() / 2))
            {
                std::cout << "[VIDSTAB DBG] construct " << (void*)this << " PYRAMID " << b->width() << " x " << b->height() << std::endl;
                _pyramid.push_back(b);
            }
            
            if (1 > _pyramid.size())
            {
                throw Common::VS_EXCEPTION_M("FrmPiramid", "Frame pyramid needs at least one layer!");
            }
        }
        
        
        virtual ~Pyramid()
        {
            for (auto& b : _pyramid)
            {
                std::cout << "[VIDSTAB DBG] destroy " << (void*)this << " PYRAMID " << b->width() << " x " << b->height() << std::endl;
                delete b;
            }
            
            if (1 > _pyramid.size())
            {
                throw Common::VS_EXCEPTION_M("FrmPiramid", "Frame pyramid needs at least one layer!");
            }
        }
        
        
        void operator()(const Canvas<pix_t>& aSrc)
        {
            std::cout << "[VIDSTAB DBG] PYRAMID " << (void*)_pyramid[0] << ":\n";
            *(_pyramid[0]) = aSrc;
            const Canvas<pix_t>* src { _pyramid[0] };

            for (auto bi {_pyramid.begin() + 1}; bi != _pyramid.end(); ++bi)
            {
                auto& b = *bi;
                _packLayer(*b, *src);
                src = b;
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
        std::vector<Canvas<pix_t>*> _pyramid;
        
        
        static void _packLayer(Canvas<pix_t>&       aDst,
                               const Canvas<pix_t>& aSrc)
        {
            const auto dim { aDst.dim() };
            
            for (unsigned y { 0 }; y < dim.y; ++y)
            {
                for (unsigned x { 0 }; x < dim.x; ++x)
                {
                    Common::Vect<unsigned> dstIdx { x , y      };
                    Common::Vect<unsigned> srcIdx { dstIdx * 2 };
                    
                    PixIfc<pix_t> acc { aSrc[srcIdx] };
                    
                    ++srcIdx.x;
                    acc += aSrc[srcIdx];
                    
                    ++srcIdx.y;
                    acc += aSrc[srcIdx];
                    
                    --srcIdx.x;
                    acc += aSrc[srcIdx];

                    acc         /= 4U;
                    aDst[dstIdx] = acc;
                }
            }
        }
    };
}
