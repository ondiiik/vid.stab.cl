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
            x
        {
            _Tp(0)
        },
        y
        {
            _Tp(0)
        }
        {
        
        }
        
        
        template <typename _SrcTp> Vect(_SrcTp aX,
                                        _SrcTp aY) noexcept
            :
            x
        {
            _Tp(aX)
        },
        y
        {
            _Tp(aY)
        }
        {
        
        }
        
        
        template <typename _SrcTp> Vect(const Vect<_SrcTp>& aSrc) noexcept
            :
            x
        {
            _Tp(aSrc.x)
        },
        y
        {
            _Tp(aSrc.y)
        }
        {
        
        }
        
        
        template <typename _SrcTp> inline Vect& operator-=(const Vect<_SrcTp>& aSrc) noexcept
        {
            this->x -= _Tp(aSrc.x);
            this->y -= _Tp(aSrc.y);
            return *this;
        }
        
        
        template <typename _SrcTp> inline Vect operator-(const Vect<_SrcTp>& aSrc) const noexcept
        {
            ;
            Vect r { *this };
            r -= aSrc;
            return r;
        }
        
        
        template <typename _SrcTp> inline Vect& operator-=(_SrcTp aVal) noexcept
        {
            this->x -= _Tp(aVal);
            this->y -= _Tp(aVal);
            return *this;
        }
        
        
        template <typename _SrcTp> inline Vect operator-(_SrcTp aVal) const noexcept
        {
            Vect r { *this };
            r -= _Tp(aVal);
            return r;
        }
        
        
        template <typename _SrcTp> inline Vect& operator+=(const Vect<_SrcTp>& aSrc) noexcept
        {
            this->x += _Tp(aSrc.x);
            this->y += _Tp(aSrc.y);
            return *this;
        }
        
        
        template <typename _SrcTp> inline Vect operator+(const Vect<_SrcTp>& aSrc) const noexcept
        {
            Vect r { *this };
            r += aSrc;
            return r;
        }
        
        
        template <typename _SrcTp> inline Vect& operator+=(_SrcTp aVal) noexcept
        {
            this->x += _Tp(aVal);
            this->y += _Tp(aVal);
            return *this;
        }
        
        
        template <typename _SrcTp> inline Vect operator+(_SrcTp aVal) const noexcept
        {
            Vect r { *this };
            r += _Tp(aVal);
            return r;
        }
        
        
        template <typename _SrcTp> inline Vect& operator/=(_SrcTp aVal) noexcept
        {
            this->x /= _Tp(aVal);
            this->y /= _Tp(aVal);
            return *this;
        }
        
        
        template <typename _SrcTp> inline Vect operator/(_SrcTp aVal) const noexcept
        {
            Vect r { *this };
            r /= _Tp(aVal);
            return r;
        }
        
        
        template <typename _SrcTp> inline Vect& operator*=(_SrcTp aVal) noexcept
        {
            this->x *= _Tp(aVal);
            this->y *= _Tp(aVal);
            return *this;
        }
        
        
        template <typename _SrcTp> inline Vect operator*(_SrcTp aVal) const noexcept
        {
            Vect r { *this };
            r *= _Tp(aVal);
            return r;
        }
        
        
        template <typename _SrcTp> inline Vect div(const Vect<_SrcTp>& aSrc) const noexcept
        {
            Vect r { *this };
            r.x /= _Tp(aSrc.x);
            r.y /= _Tp(aSrc.y);
            return r;
        }
        
        
        template <typename _SrcTp> inline Vect mul(const Vect<_SrcTp>& aSrc) const noexcept
        {
            Vect r { *this };
            r.x *= _Tp(aSrc.x);
            r.y *= _Tp(aSrc.y);
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
        
        
        inline _Tp dim() const noexcept
        {
            return x * y;
        }
        
        
        _Tp x;
        _Tp y;
    };
}
