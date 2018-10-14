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
        
        
        template <typename _SrcTp> Vect(_SrcTp aN) noexcept
            :
            x
        {
            _Tp(aN)
        },
        y
        {
            _Tp(aN)
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
    
    
    template <typename _Tp> class VectIt
    {
    public:
        VectIt(const Vect<_Tp>& aEnd) noexcept
            :
            _vect  {  },
        _begin {      },
        _end   { aEnd }
        {
        
        }
        
        
        VectIt(_Tp aEnd) noexcept
            :
            _vect  {  },
        _begin {      },
        _end   { aEnd }
        {
        
        }
        
        
        VectIt(const Vect<_Tp>& aBegin,
               const Vect<_Tp>& aEnd) noexcept
            :
            _vect  { aBegin },
        _begin { aBegin     },
        _end   { aEnd       }
        {
        
        }
        
        
        VectIt(_Tp aBegin,
               _Tp aEnd) noexcept
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
        
        
        inline operator Vect<_Tp>& () noexcept
        {
            return vect();
        }
        
        
        inline Vect<_Tp>& operator()() noexcept
        {
            return vect();
        }
        
        
        inline Vect<_Tp>& vect() noexcept
        {
            return _vect;
        }
        
        
    private:
        Vect<_Tp>       _vect;
        const Vect<_Tp> _begin;
        const Vect<_Tp> _end;
    };
    
    
    template <typename _Tp> class VectItSpiral
    {
    public:
        VectItSpiral(const Vect<_Tp>& aEnd) noexcept
            :
            _vect  { aEnd / 2 },
        _begin {              },
        _end   { aEnd         },
        _dir   { _LEFT        },
        _limit { _Tp(1)       },
        _step  { _Tp(0)       }
        {
        
        }
        
        
        VectItSpiral(_Tp aEnd) noexcept
            :
            _vect  { aEnd / 2 },
        _begin {              },
        _end   { aEnd         },
        _dir   { _LEFT        },
        _limit { _Tp(1)       },
        _step  { _Tp(0)       }
        {
        
        }
        
        
        VectItSpiral(const Vect<_Tp>& aBegin,
                     const Vect<_Tp>& aEnd) noexcept
            :
            _vect  { (aBegin + aEnd) / 2 },
        _begin { aBegin                  },
        _end   { aEnd                    },
        _dir   { _LEFT                   },
        _limit { _Tp(1)                  },
        _step  { _Tp(0)                  }
        {
        
        }
        
        
        VectItSpiral(_Tp aBegin,
                     _Tp aEnd) noexcept
            :
            _vect  { (aBegin + aEnd) / 2 },
        _begin { aBegin                  },
        _end   { aEnd                    },
        _dir   { _LEFT                   },
        _limit { _Tp(1)                  },
        _step  { _Tp(0)                  }
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
        
        
        inline operator Vect<_Tp>& () noexcept
        {
            return vect();
        }
        
        
        inline Vect<_Tp>& operator()() noexcept
        {
            return vect();
        }
        
        
        inline Vect<_Tp>& vect() noexcept
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
        
        Vect<_Tp>       _vect;
        const Vect<_Tp> _begin;
        const Vect<_Tp> _end;
        _Direction      _dir;
        _Tp             _limit;
        _Tp             _step;
    };
}
