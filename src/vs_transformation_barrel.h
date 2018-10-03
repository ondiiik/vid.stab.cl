/*
 * vs_barrel.h
 *
 *  Created on: 2. 10. 2018
 *      Author: ondiiik
 */
#pragma once


#include "vs_transformation.h"
#include "common_vect.h"


namespace VidStab
{
    class TransformationBarrel
    {
    public:
        /**
         * @brief   Transformation vector type
         */
        typedef Common::Vect<float> Vect;


        /**
         * @brief   Construct transformation to barrel distortion
         */
        TransformationBarrel(float aK0     = 8e-08,
                             float aK1     = -3.8e-15,
                             float aK2     = 9e-23,
                             int   aWidth  = 3840,
                             int   aHeight = 2160) noexcept;
                             
                             
        /**
         * @brief   Construct transformation to barrel distortion
         */
        virtual ~TransformationBarrel() noexcept;
        
        
        /**
         * @brief   Transform into barrel distortion
         *
         * Use barrel distortion equation to get corresponding point in
         * distortion space from linear space.
         *
         * @param   aDst    Destination transformation
         * @param   aSrc    Source transformation
         */
        void to(Vect&       aDst,
                const Vect& aSrc,
                float       aRatio) noexcept;
        
        
        /**
         * @brief   Transform from transformation
         *
         * Call equation resolver to get corresponding point in linear
         * space from barrel distortion space.
         *
         * @param   aDst    Source transformation
         * @param   aSrc    Destination transformation
         */
        void from(Vect&       aDst,
                  const Vect& aSrc,
                  float       aRatio) noexcept;
        
        
    private:
        /**
         * @brief   Barrel distortion equation coefficients
         */
        float _k[3];
        
        
        /**
         * @brief   Distortion size
         *
         * Defines size where coefficients was calculated
         */
        Vect _center;
        
        
        /**
         * @brief   Last destination transformation in @c from barrel
         *          direction
         *
         * Value is used for faster resolver guess when there is repeated
         * calculation for close point.
         */
        Vect _lastSrc;
        
        
        /**
         * @brief   Last source transformation in @c from barrel
         *          direction
         *
         * Value is used for faster resolver guess when there is repeated
         * calculation for close point.
         */
        Vect _lastDst;
    };
}
