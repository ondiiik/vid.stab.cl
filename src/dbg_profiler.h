/*
 * dbg_profiler.h
 *
 *  Created on: 28. 9. 2018
 *      Author: ondiiik
 */
#pragma once


#include <cstring>


namespace Dbg
{
    namespace Profiler
    {
        class Data
        {
        public:
            Data()
                :
                _enter { 0 },
                _leave { 0 }
            {
            
            }
            
            
            inline operator unsigned long long()
            {
                return get();
            }
            
            
            inline unsigned long long operator()()
            {
                return get();
            }
            
            
            unsigned long long get()
            {
                return _leave - _enter;
            }
            
            
            unsigned long long _enter;
            unsigned long long _leave;
        };
        
        
        class Measure
        {
        public:
            inline Measure(Data& aData)
                :
                _data { aData }
            {
                enter();
            }
            
            
            
            inline ~Measure()
            {
                leave();
            }
            
            
            inline void enter()
            {
                _data._enter = _rdtsc();
            }
            
            
            inline void leave()
            {
                _data._leave = _rdtsc();
            }
            
            
        private:
#if defined(__i386__)
        
        
            static __inline__ unsigned long long _rdtsc()
            {
                unsigned long long int x;
                __asm__ volatile (".byte 0x0f, 0x31" : "=A" (x));
                return x;
            }
            
            
#elif defined(__x86_64__)
            
            
            static __inline__ unsigned long long _rdtsc()
            {
                unsigned hi, lo;
                __asm__ __volatile__ ("rdtsc" : "=a"(lo), "=d"(hi));
                return ( (unsigned long long)lo) | ( ((unsigned long long)hi) << 32 );
            }
            
            
#else
#   error   "Profiling unavailable for this platform!"
#endif
            
            
            Data& _data;
        };
    }
}
