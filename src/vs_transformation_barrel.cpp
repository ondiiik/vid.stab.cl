/*
 * vs_barrel.cpp
 *
 *  Created on: 2. 10. 2018
 *      Author: ondiiik
 */
#include "vs_transformation_barrel.h"


namespace VidStab
{
    TransformationBarrel::TransformationBarrel(float aK0,
                                               float aK1,
                                               float aK2,
                                               int   aWidth,
                                               int   aHeight) noexcept
        :
        Transformation
    {
    
    },
    _center
    {
        float(aWidth)  / 2,
        float(aHeight) / 2
    }
    {
        _k[0] = aK0;
        _k[1] = aK1;
        _k[2] = aK2;
    }
    
    
    TransformationBarrel::~TransformationBarrel() noexcept
    {
    
    }
    
    
    void TransformationBarrel::to(Vect& aDst, const Vect& aSrc) const noexcept
    {
        Vect   src = aSrc - _center;
        double rq  = src.qsize();
        double acc = 1 + rq * (_k[0] + rq * (_k[1] + rq * _k[2]));
        aDst       = (aSrc / acc) + _center;
    }
    
    
    void TransformationBarrel::from(Vect& aDst, const Vect& aSrc) const noexcept
    {
        Vect   src   = aSrc - _center;
        double rq    = src.qsize();
        double acc   = 1 + rq * (_k[0] + rq * (_k[1] + rq * _k[2]));
        
        Vect   guess = src * acc;
        
        for (unsigned i = 0; i < 64; ++i)
        {
            Vect reality;
            to(  reality, guess);
            guess += (src - reality);
            
            if ((fabs(src.x - reality.x) < 0.1) && (fabs(src.y - reality.y) < 0.1))
            {
                break;
            }
        }
        
        aDst = guess + _center;
    }
}
