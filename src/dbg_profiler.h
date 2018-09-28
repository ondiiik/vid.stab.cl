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
            Data(unsigned long long aCpuFreqKHz = 1885673ULL)
                :
                _enter      { 0           },
                _leave      { 0           },
                _cpuFreqKHz { aCpuFreqKHz }
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
                return ((_leave - _enter) * 1000ULL) / _cpuFreqKHz;
            }
            
            
            unsigned long long _enter;
            unsigned long long _leave;
            unsigned long long _cpuFreqKHz;
        };
        
        
        class Measure
        {
        public:
            inline Measure(Data& aData)
                :
                _data    { aData },
                _entered { false }
            {
                enter();
            }
            
            
            
            inline ~Measure()
            {
                leave();
            }
            
            
            inline void enter()
            {
                if (!_entered)
                {
                    _data._enter = _rdtsc();
                    _entered     = true;
                }
            }
            
            
            inline void leave()
            {
                if (_entered)
                {
                    _data._leave = _rdtsc();
                    _entered     = false;
                }
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
            bool  _entered;
        };
    }
}
