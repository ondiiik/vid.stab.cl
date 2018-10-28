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
                _pyramid.push_back(b);
            }
            
            if (1 > _pyramid.size())
            {
                throw Common::EXCEPTION_M(FrmPiramid, "Frame pyramid needs at least one layer!");
            }
        }
        
        
        virtual ~Pyramid()
        {
            for (auto& b : _pyramid)
            {
                delete b;
            }
            
            if (1 > _pyramid.size())
            {
                throw Common::EXCEPTION_M(FrmPiramid, "Frame pyramid needs at least one layer!");
            }
        }
        
        
        Pyramid& operator=(const Pyramid& aSrc)
        {
            if (_pyramid.size() != aSrc._pyramid.size())
            {
                throw Common::EXCEPTION_M(FrmPiramid,
                                             "Frame pyramid layers count mismatch (%u != %u)!",
                                             unsigned(_pyramid.size()),
                                             unsigned(aSrc._pyramid.size()));
            }
            
            if (_pyramid[0]->dim() != aSrc._pyramid[0]->dim())
            {
                throw Common::EXCEPTION_M(FrmPiramid,
                                             "Frame pyramid dimensions mismatch ([%ux%u] != [%ux%u])!",
                                             unsigned(_pyramid[0]->width()),
                                             unsigned(_pyramid[0]->height()),
                                             unsigned(aSrc._pyramid[0]->width()),
                                             unsigned(aSrc._pyramid[0]->height()));
            }
            
            for (unsigned i = 0; i < _pyramid.size(); ++i)
            {
                *(_pyramid[i]) = *(aSrc._pyramid[i]);
            }
            
            return *this;
        }
        
        
        void operator()(const Canvas<pix_t>& aSrc)
        {
            *(_pyramid[0]) = aSrc;
            const Canvas<pix_t>* src { _pyramid[0] };
            
            for (auto bi {_pyramid.begin() + 1}; bi != _pyramid.end(); ++bi)
            {
                auto& b = *bi;
                _packLayer(*b, *src);
                src = b;
            }
        }
        
        
        const Canvas<pix_t>& operator[](unsigned aIdx) const
        {
            if (aIdx >= _pyramid.size())
            {
                throw Common::EXCEPTION_M(FrmPiramid,
                                             "Layer %u out of range (max %u)!",
                                             unsigned(aIdx),
                                             unsigned(_pyramid.size()));
            }
            
            return *(_pyramid[aIdx]);
        }
        
        
        Canvas<pix_t>& operator[](unsigned aIdx)
        {
            if (aIdx >= _pyramid.size())
            {
                throw Common::EXCEPTION_M(FrmPiramid,
                                             "Layer %u out of range (max %u)!",
                                             unsigned(aIdx),
                                             unsigned(_pyramid.size()));
            }
            
            return *(_pyramid[aIdx]);
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
                    Common::Vect<unsigned> dstIdx { x, y      };
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
