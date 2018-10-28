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
                throw Common::exception("[AVG] Can not got to negative count!");
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
                throw Common::exception("[AVG] Can not calculate average from 0 items count!");
            }
        }
        
        
        inline operator _Tp() const
        {
            return (*this)();
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
