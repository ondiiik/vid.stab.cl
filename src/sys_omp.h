/*
 * sys_omp.h
 *
 *  Created on: 30. 9. 2018
 *      Author: ondiiik
 */
#pragma once


#if defined(USE_OMP)
#   include <omp.h>


#   define  OMP_PARALLEL_FOR(_threads_, _shared_, _for_) \
    omp_set_num_threads(int(_threads_)); \
    _Pragma(#_shared_) \
    for _for_


#   define  OMP_ALIAS(_alias_, _src_) \
    auto _alias_ __attribute__((unused)) = _src_;


#   define  OMP_CRITICAL(_mutex_name_) \
    /* _Pragma("omp critical("#_mutex_name_")") */ \
    _Pragma("omp critical(localmotions_append)")


#   define  OMP_MAX_THREADS unsigned(omp_get_max_threads())

#   define  OMP_THREAD_NUM unsigned(omp_get_thread_num())


#else /* defined(USE_OMP) */


#   define  OMP_PARALLEL_FOR(_threads_, _shared_, _for_) \
    for _for_

#   define  OMP_ALIAS(_alias_, _src_)

#   define  OMP_CRITICAL(_mutex_name_)

#   define  OMP_MAX_THREADS 1U

#   define  OMP_THREAD_NUM 0U


#endif /* defined(USE_OMP) */
