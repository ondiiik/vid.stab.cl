/*
 * common_range.h
 *
 *  Created on: 18. 10. 2018
 *      Author: ondiiik
 */
#pragma once


#include "common_cutils.h"


namespace Common
{
    /**
     * @brief   Represents min-max range
     */
    template <typename _Tp> struct Range
    {
        typedef _Tp type_name;
        
        
        template <typename _SrcTp> Range(_SrcTp aMin,
                                         _SrcTp aMax) noexcept
            :
            min
        {
            type_name(aMin)
        },
        max
        {
            type_name(aMax)
        }
        {
            if (min > max)
            {
                swap(min, max);
            }
        }
        
        
        template <typename _SrcTp> inline bool operator>=(_SrcTp aVal) const
        {
            return isIn(aVal);
        }
        
        
        template <typename _SrcTp> inline bool operator>(_SrcTp aVal) const
        {
            return isExclusvelyIn(aVal);
        }
        
        
        template <typename _SrcTp> inline bool operator<=(_SrcTp aVal) const
        {
            return isOut(aVal);
        }
        
        
        template <typename _SrcTp> inline bool operator<(_SrcTp aVal) const
        {
            return isExclusivelyOut(aVal);
        }
        
        
        template <typename _SrcTp> inline bool isIn(_SrcTp aVal) const
        {
            return (min <= type_name(aVal)) && (max >= type_name(aVal));
        }
        
        
        template <typename _SrcTp> inline bool isExclusivelyIn(_SrcTp aVal) const
        {
            return (min < type_name(aVal)) && (max > type_name(aVal));
        }
        
        
        template <typename _SrcTp> inline bool isOut(_SrcTp aVal) const
        {
            return !isExclusivelyIn(aVal);
        }
        
        
        template <typename _SrcTp> inline bool isExclusivelyOut(_SrcTp aVal) const
        {
            return !isIn(aVal);
        }
        
        
        template <typename _SrcTp> inline Vect& operator-=(_SrcTp aVal) noexcept
        {
            this->min -= type_name(aVal);
            this->max -= type_name(aVal);
            
            if (min > max)
            {
                swap(min, max);
            }
            return *this;
        }
        
        
        template <typename _SrcTp> inline Vect operator-(_SrcTp aVal) const noexcept
        {
            Vect r { *this };
            r -= type_name(aVal);
            return r;
        }
        
        
        template <typename _SrcTp> inline Vect& operator+=(_SrcTp aVal) noexcept
        {
            this->min += type_name(aVal);
            this->max += type_name(aVal);
            return *this;
        }
        
        
        template <typename _SrcTp> inline Vect operator+(_SrcTp aVal) const noexcept
        {
            Vect r { *this };
            r += type_name(aVal);
            return r;
        }
        
        
        template <typename _SrcTp> inline Vect& operator*=(_SrcTp aVal) noexcept
        {
            this->min *= aVal;
            this->max *= aVal;
            return *this;
        }
        
        
        template <typename _SrcTp> inline Vect operator*(_SrcTp aVal) const noexcept
        {
            Vect r { *this };
            r *= aVal;
            return r;
        }
        
        
        template <typename _SrcTp> inline Vect& operator/=(_SrcTp aVal) noexcept
        {
            this->min /= aVal;
            this->max /= aVal;
            return *this;
        }


        template <typename _SrcTp> inline Vect operator/(_SrcTp aVal) const noexcept
        {
            Vect r { *this };
            r /= aVal;
            return r;
        }


        _Tp min;
        _Tp max;
    };
}
