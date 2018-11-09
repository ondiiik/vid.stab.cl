/*
 * vs_barrel.cpp
 *
 *  Created on: 2. 10. 2018
 *      Author: ondiiik
 */
#include "gimbal_barrel.h"
#include <iostream> // DEBUG


namespace Gimbal
{
    Barrel::Barrel(float aK0,
                   float aK1,
                   float aK2,
                   int   aWidth,
                   int   aHeight) noexcept
        :
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
    
    
    Barrel::~Barrel() noexcept
    {
    
    }
    
    
    void Barrel::to(Common::Vect<float>&       aDst,
                    const Common::Vect<float>& aSrc,
                    float                      aRatio) noexcept
    {
        /*
         * Conversion from linear space is simple by filling in
         * equation variables
         */
        Common::Vect<float> center = _center / aRatio;
        Common::Vect<float> src    = aSrc - center;
        src                       *= aRatio;
        float rq                   = src.qsize();
        float acc                  = 1.0F + rq * (_k[0] + rq * (_k[1] + rq * _k[2]));
        aDst                       = (src / acc);
        aDst                      /= aRatio;
        aDst                      += center;
    }
    
    
    void Barrel::from(Common::Vect<float>&       aDst,
                      const Common::Vect<float>& aSrc,
                      float                      aRatio) noexcept
    {
        /*
         * To get back transformation, we have to run equation resolver.
         * First resolver step is to guess some point close to expected
         * result. The best estimation is to use last result, if calculated
         * vector was somewhere close. Otherwise we have to take a guess.
         */
        Common::Vect<float> center    { _center / aRatio                              };
        Common::Vect<float> src       { (aSrc - center)* aRatio                       };
        float rq                      { src.qsize()                                   };
        float acc                     { 1.0F + rq* (_k[0] + rq * (_k[1] + rq* _k[2])) };
        Common::Vect<float> estimated { src * acc                                     };
        
        
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
            rq         = estimated.qsize();
            acc        = 1 + rq * (_k[0] + rq * (_k[1] + rq * _k[2]));
            Common::Vect<float> reality { estimated / acc };
            estimated += (src - reality);
            
            
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
        estimated /= aRatio;
        aDst       = estimated + center;
    }
}
