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
        typedef _Tp type_name;


        Vect() noexcept
            :
            x
        {
            type_name(0)
        },
        y
        {
            type_name(0)
        }
        {
        
        }
        
        
        template <typename _SrcTp> Vect(_SrcTp aX,
                                        _SrcTp aY) noexcept
            :
            x
        {
            type_name(aX)
        },
        y
        {
            type_name(aY)
        }
        {
        
        }
        
        
        template <typename _SrcTp> Vect(_SrcTp aN) noexcept
            :
            x
        {
            type_name(aN)
        },
        y
        {
            type_name(aN)
        }
        {
        
        }
        
        
        template <typename _SrcTp> Vect(const Vect<_SrcTp>& aSrc) noexcept
            :
            x
        {
            type_name(aSrc.x)
        },
        y
        {
            type_name(aSrc.y)
        }
        {
        
        }
        
        
        inline bool operator==(const Vect& aSrc) const noexcept
        {
            return ((this->x == aSrc.x) && (this->y == aSrc.y));
        }
        
        
        bool operator!=(const Vect& aSrc) const noexcept
        {
            return !(*this == aSrc);
        }
        
        
        template <typename _SrcTp> inline Vect& operator-=(const Vect<_SrcTp>& aSrc) noexcept
        {
            this->x -= type_name(aSrc.x);
            this->y -= type_name(aSrc.y);
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
            this->x -= type_name(aVal);
            this->y -= type_name(aVal);
            return *this;
        }
        
        
        template <typename _SrcTp> inline Vect operator-(_SrcTp aVal) const noexcept
        {
            Vect r { *this };
            r -= type_name(aVal);
            return r;
        }
        
        
        template <typename _SrcTp> inline Vect& operator+=(const Vect<_SrcTp>& aSrc) noexcept
        {
            this->x += type_name(aSrc.x);
            this->y += type_name(aSrc.y);
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
            this->x += type_name(aVal);
            this->y += type_name(aVal);
            return *this;
        }
        
        
        template <typename _SrcTp> inline Vect operator+(_SrcTp aVal) const noexcept
        {
            Vect r { *this };
            r += type_name(aVal);
            return r;
        }
        
        
        template <typename _SrcTp> inline Vect& operator/=(_SrcTp aVal) noexcept
        {
            this->x /= aVal;
            this->y /= aVal;
            return *this;
        }
        
        
        template <typename _SrcTp> inline Vect operator/(_SrcTp aVal) const noexcept
        {
            Vect r { *this };
            r /= aVal;
            return r;
        }
        
        
        template <typename _SrcTp> inline Vect& operator*=(_SrcTp aVal) noexcept
        {
            this->x *= aVal;
            this->y *= aVal;
            return  *this;
        }
        
        
        template <typename _SrcTp> inline Vect operator*(_SrcTp aVal) const noexcept
        {
            Vect r { *this };
            r *= aVal;
            return r;
        }
        
        
        template <typename _SrcTp> inline Vect& operator>>=(_SrcTp aVal) noexcept
        {
            this->x >>= unsigned(aVal);
            this->y >>= unsigned(aVal);
            return *this;
        }
        
        
        template <typename _SrcTp> inline Vect operator>>(_SrcTp aVal) const noexcept
        {
            Vect r { *this };
            r >>= unsigned(aVal);
            return r;
        }
        
        
        template <typename _SrcTp> inline Vect& operator<<=(_SrcTp aVal) noexcept
        {
            this->x <<= unsigned(aVal);
            this->y <<= unsigned(aVal);
            return *this;
        }
        
        
        template <typename _SrcTp> inline Vect operator<<(_SrcTp aVal) const noexcept
        {
            Vect r { *this };
            r <<= unsigned(aVal);
            return r;
        }
        
        
        template <typename _SrcTp> inline Vect div(const Vect<_SrcTp>& aSrc) const noexcept
        {
            Vect r { *this };
            r.x /= aSrc.x;
            r.y /= aSrc.y;
            return r;
        }
        
        
        template <typename _SrcTp> inline Vect mul(const Vect<_SrcTp>& aSrc) const noexcept
        {
            Vect r { *this };
            r.x *= aSrc.x;
            r.y *= aSrc.y;
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
            return (type_name(fabs(aVec.x - x)) < aRange) &&
                   (type_name(fabs(aVec.y - y)) < aRange);
        }
        
        
        inline _Tp qsize() const noexcept
        {
            return x * x + y * y;
        }
        
        
        inline _Tp size() const noexcept
        {
            return type_name(sqrt(qsize()));
        }
        
        
        inline double angle() const noexcept
        {
            return atan2(double(y), double(x));
        }
        
        
        inline _Tp dim() const noexcept
        {
            return x * y;
        }
        
        
        type_name x;
        type_name y;
    };
    
    
    template <typename _Tp> class VectIt
    {
    public:
        typedef _Tp type_name;


        VectIt(const Vect<type_name>& aEnd) noexcept
            :
            _vect  {  },
        _begin {      },
        _end   { aEnd }
        {
        
        }
        
        
        VectIt(type_name aEnd) noexcept
            :
            _vect  {  },
        _begin {      },
        _end   { aEnd }
        {
        
        }
        
        
        VectIt(const Vect<type_name>& aBegin,
               const Vect<type_name>& aEnd) noexcept
            :
            _vect  { aBegin },
        _begin { aBegin     },
        _end   { aEnd       }
        {
        
        }
        
        
        VectIt(type_name aBegin,
               type_name aEnd) noexcept
            :
            _vect  { aBegin },
        _begin { aBegin     },
        _end   { aEnd       }
        {
        
        }
        
        
        inline bool next() noexcept
        {
            ++_vect.x;
            
            if (_vect.x >= _end.x)
            {
                _vect.x = _begin.x;
                ++_vect.y;
                
                if (_vect.y >= _end.y)
                {
                    _vect.y = _begin.y;
                    return false;
                }
            }
            
            return true;
        }
        
        
        inline operator Vect<type_name>& () noexcept
        {
            return vect();
        }
        
        
        inline Vect<type_name>& operator()() noexcept
        {
            return vect();
        }
        
        
        inline Vect<type_name>& vect() noexcept
        {
            return _vect;
        }
        
        
    private:
        Vect<type_name>       _vect;
        const Vect<type_name> _begin;
        const Vect<type_name> _end;
    };
    
    
    template <typename _Tp> class VectItSpiral
    {
    public:
        typedef _Tp type_name;


        VectItSpiral(const Vect<type_name>& aEnd) noexcept
            :
            _vect  { aEnd / 2 },
        _begin {              },
        _end   { aEnd         },
        _dir   { _LEFT        },
        _limit { type_name(1)       },
        _step  { type_name(0)       }
        {
        
        }
        
        
        VectItSpiral(type_name aEnd) noexcept
            :
            _vect  { aEnd / 2 },
        _begin {              },
        _end   { aEnd         },
        _dir   { _LEFT        },
        _limit { type_name(1) },
        _step  { type_name(0) }
        {
        
        }
        
        
        VectItSpiral(const Vect<type_name>& aBegin,
                     const Vect<type_name>& aEnd) noexcept
            :
            _vect  { (aBegin + aEnd) / 2 },
        _begin { aBegin                  },
        _end   { aEnd                    },
        _dir   { _LEFT                   },
        _limit { type_name(1)            },
        _step  { type_name(0)            }
        {
        
        }
        
        
        VectItSpiral(type_name aBegin,
                     type_name aEnd) noexcept
            :
            _vect  { (aBegin + aEnd) / 2 },
        _begin { aBegin                  },
        _end   { aEnd                    },
        _dir   { _LEFT                   },
        _limit { type_name(1)            },
        _step  { type_name(0)            }
        {
        
        }
        
        
        inline bool next() noexcept
        {
            ++_step;
            
            switch (_dir)
            {
                case _LEFT:
                    ++_vect.x;
                    
                    if (_step == _limit)
                    {
                        _dir  = _DOWN;
                        _step = 0;
                    }
                    
                    break;
                    
                    
                case _DOWN:
                    ++_vect.y;
                    
                    if (_step == _limit)
                    {
                        _dir  = _RIGHT;
                        _step = 0;
                        ++_limit;
                    }
                    
                    break;
                    
                    
                case _RIGHT:
                    --_vect.x;
                    
                    if (_step == _limit)
                    {
                        _dir  = _UP;
                        _step = 0;
                    }
                    
                    break;
                    
                    
                case _UP:
                    --_vect.y;
                    
                    if (_step == _limit)
                    {
                        _dir  = _LEFT;
                        _step = 0;
                        ++_limit;
                    }
                    
                    break;
            }
            
            if (_vect == _end)
            {
                _vect = _begin;
                return false;
            }
            else
            {
                return true;
            }
        }
        
        
        inline operator Vect<type_name>& () noexcept
        {
            return vect();
        }
        
        
        inline Vect<type_name>& operator()() noexcept
        {
            return vect();
        }
        
        
        inline Vect<type_name>& vect() noexcept
        {
            return _vect;
        }
        
        
    private:
        enum _Direction
        {
            _LEFT,
            _DOWN,
            _RIGHT,
            _UP
        };
        
        Vect<type_name>       _vect;
        const Vect<type_name> _begin;
        const Vect<type_name> _end;
        _Direction            _dir;
        type_name             _limit;
        type_name             _step;
    };
}
