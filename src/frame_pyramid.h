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
            for (auto* b = new Layer<_PixT>(aDim); (b->width() > aMin) && (b->height() > aMin); b = new Layer<_PixT>(b->dim() / 2))
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
        
        
        void operator()(const Canvas<_PixT>& aSrc)
        {
            *(_pyramid[0]) = aSrc;
            const Canvas<_PixT>* src { _pyramid[0] };
            
            for (auto b {_pyramid.begin() + 1}; b != _pyramid.end(); ++b)
            {
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
        std::vector<Layer<_PixT>*> _pyramid;
        
        
        static void _packLayer(Canvas<_PixT>&       aDst,
                               const Canvas<_PixT>& aSrc)
        {
            const auto dim { aDst.dim() };
            
            for (unsigned y { 0 }; y < dim.y; ++y)
            {
                for (unsigned x { 0 }; x < dim.x; ++x)
                {
                    const Common::Vect<unsigned> srcIdx { x * 2, y * 2 };


                }
            }
        }
    };
}
