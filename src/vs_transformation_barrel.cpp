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
    
    
    void TransformationBarrel::to(Vect& aDst, const Vect& aSrc) noexcept
    {
        /*
         * Conversion from linear space is simple by filling in
         * equation variables
         */
        Vect   src = aSrc - _center;
        double rq  = src.qsize();
        double acc = 1 + rq * (_k[0] + rq * (_k[1] + rq * _k[2]));
        aDst       = (aSrc / acc) + _center;
    }
    
    
    void TransformationBarrel::from(Vect& aDst, const Vect& aSrc) noexcept
    {
        /*
         * To get back transformation, we have to run equation resolver.
         * First resolver step is to guess some point close to expected
         * result. The best estimation is to use last result, if calculated
         * vector was somewhere close. Otherwise we have to take a guess.
         */
        Vect src = aSrc - _center;

        if (!_lastSrc.isCloseSq(src, 4))
        {
            double rq  = src.qsize();
            double acc = 1 + rq * (_k[0] + rq * (_k[1] + rq * _k[2]));
            _lastDst   = src * acc;
        }
        
        _lastSrc = src;
        
        
        /*
         * Resolver is iterative, where with each iteration we should
         * getting closer to solution.
         * 64 iteration should be enough for selected accuracy (0.1)
         */
        for (unsigned i = 0; i < 64; ++i)
        {
            /*
             * Calculates real point related to last guess. Then we
             * process estimation correction to get closer to solution.
             */
            Vect reality;
            to(  reality, _lastDst);
            _lastDst += (src - reality);
            
            /*
             * We consider equation resolved when our difference from
             * reality is 0.1 in both directions (x and y).
             */
            if ((fabs(src.x - reality.x) < 0.1) && (fabs(src.y - reality.y) < 0.1))
            {
                break;
            }
        }
        
        /*
         * Calculation done. Write result of resolver.
         */
        aDst = _lastDst + _center;
    }
}
