/*
 * com_vect.h
 *
 *  Created on: 28. 9. 2018
 *      Author: ondiiik
 */
#pragma once


#include <cmath>


namespace Common
{
    /**
     * @brief   Represents x y coordinates vector
     */
    template <typename _Tp> struct Vect
    {
        Vect() noexcept
            :
            x { _Tp(0) },
        y { _Tp(0) }
        {
        
        }
        
        
        Vect(_Tp aX,
             _Tp aY) noexcept
            :
            x { aX },
        y { aY }
        {
        
        }
        
        
        inline Vect& operator-=(const Vect& aSrc) noexcept
        {
            this->x -= aSrc.x;
            this->y -= aSrc.y;
            return *this;
        }
        
        
        inline Vect operator-(const Vect& aSrc) const noexcept
        {
            ;
            Vect r { *this };
            r -= aSrc;
            return r;
        }
        
        
        inline Vect& operator-=(_Tp aVal) noexcept
        {
            this->x -= aVal;
            this->y -= aVal;
            return *this;
        }
        
        
        inline Vect operator-(_Tp aVal) const noexcept
        {
            Vect r { *this };
            r -= aVal;
            return r;
        }
        
        
        inline Vect& operator+=(const Vect& aSrc) noexcept
        {
            this->x += aSrc.x;
            this->y += aSrc.y;
            return *this;
        }
        
        
        inline Vect operator+(const Vect& aSrc) const noexcept
        {
            Vect r { *this };
            r += aSrc;
            return r;
        }
        
        
        inline Vect& operator+=(_Tp aVal) noexcept
        {
            this->x += aVal;
            this->y += aVal;
            return *this;
        }
        
        
        inline Vect operator+(_Tp aVal) const noexcept
        {
            Vect r { *this };
            r += aVal;
            return r;
        }
        
        
        inline Vect& operator/=(_Tp aVal) noexcept
        {
            this->x /= aVal;
            this->y /= aVal;
            return *this;
        }
        
        
        inline Vect operator/(_Tp aVal) const noexcept
        {
            Vect r { *this };
            r /= aVal;
            return r;
        }
        
        
        inline Vect& operator*=(_Tp aVal) noexcept
        {
            this->x *= aVal;
            this->y *= aVal;
            return *this;
        }
        
        
        inline Vect operator*(_Tp aVal) const noexcept
        {
            Vect r { *this };
            r *= aVal;
            return r;
        }
        
        
        /**
         * @brief   Say if another vector is close to this
         *
         * Square is used as range. Vector must be in square range
         * limited by @c +- @c ARange in @c x and @c y direction.
         *
         * @param aVec      Another vector
         * @param aRange    Range
         *
         * @return
         */
        inline bool isCloseSq(const Vect& aVec, _Tp aRange) const noexcept
        {
            return (_Tp(fabs(aVec.x - x)) < aRange) &&
                   (_Tp(fabs(aVec.y - y)) < aRange);
        }


        inline _Tp qsize() const noexcept
        {
            return x * x + y * y;
        }
        
        
        inline _Tp size() const noexcept
        {
            return _Tp(sqrt(qsize()));
        }
        
        
        inline double angle() const noexcept
        {
            return atan2(double(y), double(x));
        }
        
        
        _Tp x;
        _Tp y;
    };
}
