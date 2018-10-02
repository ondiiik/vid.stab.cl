#pragma once
/*
 * vs_transformation.h
 *
 *  Created on: 2. 10. 2018
 *      Author: ondiiik
 */
#include "common_vect.h"


namespace VidStab
{
    /**
     * @brief   Generic transformation interface
     */
    class Transformation
    {
    public:
        /**
         * @brief   Nothing is initialized - default constructor
         */
        Transformation()
        {
        
        }
        
        
        /**
         * @brief   Virtual methods - virtual constructor
         */
        virtual ~Transformation()
        {
        
        }
        
        
        /**
         * @brief   Transformation vector type
         */
        typedef Common::Vect<float> Vect;
        
        
        /**
         * @brief   Transform into transformation
         *
         * Process transformation from linear space to nonlinear space
         * of image.
         *
         * @param   aDst    Destination transformation
         * @param   aSrc    Source transformation
         */
        virtual void to(Vect& aDst, const Vect& aSrc) const noexcept = 0;
        
        
        /**
         * @brief   Transform from transformation
         *
         * Process transformation from nonlinear space of of image to
         * linear space.
         *
         * @param   aDst    Source transformation
         * @param   aSrc    Destination transformation
         */
        virtual void from(Vect& aDst, const Vect& aSrc) const noexcept = 0;
    };
}
