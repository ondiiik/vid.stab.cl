/*
 * com_vect.h
 *
 *  Created on: 28. 9. 2018
 *      Author: ondiiik
 */
#pragma once


namespace Common
{
    /**
     * @brief   Represents x y coordinates vector
     */
    template <typename _Tp> struct Vect
    {
        Vect()
            :
            x { _Tp(0) },
            y { _Tp(0) }
        {
        
        }
        
        
        Vect(_Tp aX,
             _Tp aY)
            :
            x { aX },
            y { aY }
        {
        
        }
        
        
        inline Vect& operator-=(const Vect& aSrc)
        {
            this->x -= aSrc.x;
            this->y -= aSrc.y;
            return *this;
        }
        
        
        inline Vect operator-(const Vect& aSrc) const
        {
            ;
            Vect r { *this };
            r -= aSrc;
            return r;
        }
        
        
        inline Vect& operator+=(const Vect& aSrc)
        {
            this->x += aSrc.x;
            this->y += aSrc.y;
            return *this;
        }
        
        
        inline Vect operator+(const Vect& aSrc) const
        {
            Vect r { *this };
            r += aSrc;
            return r;
        }
        
        
        inline Vect& operator/=(_Tp aVal)
        {
            this->x /= aVal;
            this->y /= aVal;
            return *this;
        }
        
        
        inline Vect operator/(_Tp aVal) const
        {
            Vect r { *this };
            r /= aVal;
            return r;
        }
        
        
        _Tp x;
        _Tp y;
    };
}
