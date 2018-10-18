/*
 * common_c.h
 *
 *  Created on: 29. 9. 2018
 *      Author: ondiiik
 */
#pragma once


#include <cstddef>


#define COMMON_ARRAY_ITEMS_CNT(_array_) (sizeof(_array_) / sizeof((_array_)[0]))


namespace Common
{
    template <typename _Tp> inline void swap(_Tp& aV1, _Tp& aV2)
    {
        _Tp tmp { aV1 };
        aV1 = aV2;
        aV2 = tmp;
    }
}
