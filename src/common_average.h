#pragma once
/*
 * common_avg.h
 *
 *  Created on: 28. 10. 2018
 *      Author: OSi (Ondrej Sienczak)
 */
#include "common_exception.h"


namespace Common
{
    /**
     * @brief   Average calculator
     */
    template <typename _Tp, typename _AccTp = _Tp> class Average
    {
    public:
        inline Average() noexcept
            :
            _acc { _AccTp(0) },
        _cnt { 0U        }
        {
        
        }
        
        
        inline Average& operator=(const _Tp& aVal) noexcept
        {
            _acc = _AccTp(aVal);
            _cnt = 1U;
            return *this;
        }
        
        
        inline Average& operator+=(const _Tp& aVal) noexcept
        {
            _acc += _AccTp(aVal);
            ++_cnt;
            return *this;
        }
        
        
        inline Average& operator-=(const _Tp& aVal)
        {
            if (0 != _cnt)
            {
                _acc -= _AccTp(aVal);
                --_cnt;
                return *this;
            }
            else
            {
                throw EXCEPTION_M(AVG, "Can not got to negative count!");
            }
        }
        
        
        inline _Tp operator()() const
        {
            if (0 != _cnt)
            {
                return _Tp(_acc / _cnt);
            }
            else
            {
                throw EXCEPTION_M(AVG, "No samples to calculate average!");
            }
        }
        
        
        inline Average& add(const _Tp& aVal,
                            unsigned   aWeight) noexcept
        {
            _acc += _AccTp(aVal * aWeight);
            _cnt += aWeight;
            return *this;
        }
        
        
        inline Average& sub(const _Tp& aVal,
                            unsigned   aWeight)
        {
            if (aWeight <= _cnt)
            {
                _acc -= _AccTp(aVal * aWeight);
                _cnt -= aWeight;
                return *this;
            }
            else
            {
                throw EXCEPTION_M(AVG, "Can not got to negative count!");
            }
        }
        
        
        inline operator _Tp() const
        {
            return (*this)();
        }
        
        
        inline void reset()
        {
            _acc = _AccTp(0);
            _cnt = 0U;
        }
        
        
        inline unsigned cnt() const
        {
            return _cnt;
        }
        
        
    private:
        _AccTp   _acc;
        unsigned _cnt;
    };
}
