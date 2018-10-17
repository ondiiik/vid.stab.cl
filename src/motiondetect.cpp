/*
 * motiondetect.cpp
 *
 *  Copyright (C) Georg Martius - February 1007-2011
 *   georg dot martius at web dot de
 *  Copyright (C) Alexey Osipov - Jule 2011
 *   simba at lerlan dot ru
 *   speed optimizations (threshold, spiral, SSE, asm)
 *
 *  This file is part of vid.stab video stabilization library
 *
 *  vid.stab is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License,
 *  as published by the Free Software Foundation; either version 2, or
 *  (at your option) any later version.
 *
 *  vid.stab is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with GNU Make; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */
#include "motiondetect_internal.h"
#include "motiondetect_opt.h"
#include <math.h>
#include <limits.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>

#ifdef USE_SSE2
#include <emmintrin.h>

#define USE_SSE2_CMP_HOR
#define SSE2_CMP_SUM_ROWS 8
#endif

#include "vidstabdefines.h"
#include "localmotion2transform.h"
#include "transformtype_operations.h"
#include "transformtype_operations.h"


/*
 * C++ includes
 */
#include "motiondetect.h"
#include "frame_canvas.h"
#include "common_exception.h"
#include "common_dbg.h"

#include "dbg_profiler.h"

#include "cl/opencl.h"
#include "cl/opencl___blur_h.h"
#include "cl/opencl___blur_v.h"


#include "sys_omp.h"

#include <vector>
#include <algorithm>
#include <cstdio>
#include <limits>




using namespace VidStab;


namespace
{
    /**
     * @brief   Module name used by exceptions
     */
    const char moduleName[] { "Detect" };
    
    
    /**
     * @brief   Minimal count of detection boxes in shortest direction
     */
    const unsigned _cellsCnt { 16 };
    
    
    /**
     * @brief   Detection box size (for smallest pyramid layer)
     */
    const unsigned _cellSize { 8 };
    
    
    /**
     * @brief   Minimal pyramid size
     */
    const unsigned _piramidMinSize { _cellsCnt * _cellSize };
    
    
    /**
     * @brief   Quality threshold for selection
     */
    const unsigned _contrastThreshold { 64 };
    
    
    /**
     * @brief   Slow filter A count of frames
     *
     * Filter used for filtering slow movements
     */
    const unsigned _slowACnt { 15 };
    
    
    /**
     * @brief   Slow filter B count of frames
     *
     * Filter used for filtering slow movements (shifted)
     */
    const unsigned _slowBCnt { _slowACnt / 2 };
    
    
    /**
     * @brief   Slow filter A count of frames
     *
     * Filter used for filtering slow movements
     */
    const unsigned _staticACnt { 60 };
    
    
    /**
     * @brief   Slow filter B count of frames
     *
     * Filter used for filtering slow movements (shifted)
     */
    const unsigned _staticBCnt { _staticACnt / 2 };
    
    
    /**
     * @brief   Border detection divider
     */
    const unsigned _borderDiv { 6 };
    
    
    /**
     * @brief   Deviation measure factor
     */
    const unsigned _devFactor { 4 };
    
    
    /**
     * @brief   Minimal quadratic size for analysis
     */
    const unsigned _minQSize { 1 };
    
    
    /**
     * @brief   Convert motion detect instance to C++ representation
     * @param   aMd     Motion detect instance
     * @return  C++ representation of motion detect instance
     */
    inline const VSMD& VSMD2Inst(const VSMotionDetect* aMd)
    {
        if (nullptr != aMd)
        {
            const VSMD* const md = (VSMD*)aMd->_inst;
            return *md;
        }
        else
        {
            throw Common::VS_EXCEPTION("Detect data C structure is NULL!");
        }
    }
}


namespace VidStab
{
    VSMD::VSMD(const char*                 aModName,
               VSMotionDetect*             aMd,
               const VSMotionDetectConfig* aConf,
               const VSFrameInfo*          aFi)
        :
        fiInfoC        { aMd->fi             },
        firstFrame     { true                },
        fi             { fiInfoC             },
        curr           { _currPrepFrameC, fi },
        currPrep       { _currPrepFrameC, fi },
        currTmp        { _currTmpFrameC,  fi },
        prev           { _prevFrameC,     fi },
        _mn            { aModName            },
        
        
        
        
        _piramidRGB    { nullptr                  },
        _piramidYUV    { nullptr                  },
        _idx           { 0U                       },
        _idxCurrent    { 0U                       },
        _idxPrev       { 0U                       },
        
        _threadsCnt    { OMP_MAX_THREADS          },
        
        _cells         {                          },
        _detectRange   { unsigned(aFi->width / _borderDiv) }
        
        
        
        
#if defined(USE_OPENCL)
        ,
        _clDevice      {          },
        _clContext     { nullptr  },
        _clProgram     {          },
        _clProgramName {          }
#endif
    {
        /*
         * Check input arguments
         */
        if (nullptr == aConf)
        {
            throw Common::VS_EXCEPTION("Configuration structure is NULL!");
        }
        
        if (nullptr == aFi)
        {
            throw Common::VS_EXCEPTION("Frame info structure is NULL!");
        }
        
        
        /*
         * Allocates pyramids for faster correlation
         */
        conf    = *aConf;
        fiInfoC = *aFi;
        
        if (fi.pixFormat() > PF_PACKED)
        {
            _piramidRGB = new Pyramids<Frame::PixRGB>(fi.dim(), _piramidMinSize);
            _init(*_piramidRGB);
        }
        else
        {
            _piramidYUV = new Pyramids<Frame::PixYUV>(fi.dim(), _piramidMinSize);
            _init(*_piramidYUV);
        }
        
        
        
        
        
        
        
        
        
        
        
        
        _initMsg();
        _initVsDetect(aConf, aFi);
        _initOpenCl();
    }
    
    
    VSMD::~VSMD()
    {
        delete   _piramidRGB;
        delete   _piramidYUV;
        
        
        
        
        if (fieldscoarse.fields)
        {
            vs_free(fieldscoarse.fields);
            fieldscoarse.fields = 0;
        }
        
        if (fieldsfine.fields)
        {
            vs_free(fieldsfine.fields);
            fieldsfine.fields = 0;
        }
        
        prev.free();
        currPrep.free();
        currTmp.free();
        
        
#if defined(USE_OPENCL)
        for (auto& pgm : _clProgram)
        {
            delete pgm;
        }
        
        delete _clContext;
#endif
    }
    
    
    void VSMD::operator ()(LocalMotions* aMotions,
                           VSFrame&      aFrame)
    {
        if      (nullptr != _piramidYUV)
        {
            _process(      *_piramidYUV, aFrame);
        }
        else if (nullptr != _piramidRGB)
        {
            _process(      *_piramidRGB, aFrame);
        }
        else
        {
            throw Common::VS_EXCEPTION("No valid pixel model selected!");
        }
        
        
        /*
         * Code will be removed in future. Now is required
         * to prevent ffmpeg to access NULL pointer
         */
        LmList lm { *aMotions };
        lm.init(1);
    }
    
    
    template <typename _PixT> inline void VSMD::_init(Pyramids<_PixT>& aPt)
    {
        const unsigned               idx    { aPt.fm[_idxCurrent].size() - 1 };
        const unsigned               mul    { 1U << idx };
        const Frame::Canvas<_PixT>&  canvas { aPt.fm[_idxCurrent][idx] };
        VectU                        begin  { 1 };
        VectU                        end    { (canvas.dim() - _cellSize / 2) / _cellSize };
        VectU                        rect   { _cellSize };
        
        
        _cells.list.resize(0);
        _cells.dim = end - begin;
        
        
        Common::VectIt<unsigned> i { begin, end };
        
        do
        {
            VectU pos { i()* _cellSize };
            
            Cell cell
            {
                (pos + rect / 2)* mul,
                rect * mul,
                i() - begin,
                { },
                0
            };
            
            _cells.list.push_back(cell);
        }
        while (i.next());
    }
    
    
    template <typename _PixT> void VSMD::_process(Pyramids<_PixT>& aPt,
                                                  VSFrame&         aFrame)
    {
        _next(      aPt, aFrame );
        _select(    aPt, aFrame );
        _estimate(  aPt, aFrame );
        _analyze(   aPt, aFrame );
        _finalize(  aPt, aFrame );
        _visualize( aPt, aFrame );
    }
    
    
    template <typename _PixT> void VSMD::_nextPiramid(Pyramids<_PixT>& aPt,
                                                      VSFrame&         aFrame)
    {
        unsigned idx { _idx };
        _idxPrev     = _idx & 1;
        ++_idx;
        _idxCurrent  = _idx & 1;
        
        Frame::Canvas<_PixT> c { (_PixT*)aFrame.data[0], fi.dim() };
        aPt.fm[_idxCurrent]( c);
        
        
        if (0 == (idx % _slowACnt))
        {
            aPt.fm[aPt.PTYPE_SLOW_A] = aPt.fm[_idxCurrent];
        }
        
        if ((0 == idx) || (0 == ((idx + _slowBCnt) % _slowACnt)))
        {
            aPt.fm[aPt.PTYPE_SLOW_B] = aPt.fm[_idxCurrent];
        }
        
        if (0 == (idx % _staticACnt))
        {
            aPt.fm[aPt.PTYPE_STATIC_A] = aPt.fm[_idxCurrent];
        }
        
        if ((0 == idx) || (0 == ((idx + _staticBCnt) % _staticACnt)))
        {
            aPt.fm[aPt.PTYPE_STATIC_B] = aPt.fm[_idxCurrent];
        }
    }
    
    
    template <typename _PixT> void VSMD::_select(Pyramids<_PixT>& aPt,
                                                 VSFrame&         aFrame)
    {
        const unsigned               idx    { aPt.fm[_idxCurrent].size() - 1             };
        const Frame::Canvas<_PixT>&  canvas { aPt.fm[_idxCurrent][idx]                   };
        VectU                        begin  { 1                                          };
        VectU                        end    { (canvas.dim() - _cellSize / 2) / _cellSize };
        VectU                        rect   { _cellSize                                  };
        const unsigned               t      { Direction::frame2vidx(_idx)                };
        
        
        Common::VectIt<unsigned> i { begin, end };
        auto                     c { _cells.list.begin() };
        
        do
        {
            VectU    pos { i()* _cellSize };
            unsigned q   { _selectContrast(canvas, pos, rect) };
            
            if (_contrastThreshold <= q)
            {
                c->qfContrast = q - _contrastThreshold;
            }
            else
            {
                c->qfContrast = 0;
            }
            
            c->direction[t].valid = true;
            ++c;
        }
        while (i.next());
    }
    
    
    
    template <typename _PixT> unsigned VSMD::_selectContrast(const Frame::Canvas<_PixT>& aCanvas,
                                                             const VectU&                aPosition,
                                                             const VectU&                aRect) const
    {
        const unsigned dist { _cellSize / 2 };
        VectS          h    { int(dist), 0       };
        VectS          v    { 0, int(dist)       };
        VectU          rect { aRect - dist       };
        VectIterU      i    { rect               };
        
        int            minV { std::numeric_limits<int>::max() };
        int            maxV { std::numeric_limits<int>::min() };
        int            minH { std::numeric_limits<int>::max() };
        int            maxH { std::numeric_limits<int>::min() };
        
        do
        {
            int p  {     int(aCanvas[aPosition + i()    ].abs()) };
            int dh { p - int(aCanvas[aPosition + i() + h].abs()) };
            int dv { p - int(aCanvas[aPosition + i() + v].abs()) };
            
            if (minV > dv)
            {
                minV = dv;
            }
            
            if (maxV < dv)
            {
                maxV = dv;
            }
            
            if (minH > dh)
            {
                minH = dh;
            }
            
            if (maxH < dh)
            {
                maxH = dh;
            }
        }
        while (i.next());
        
        return abs(minV * maxV * minH * maxH);
    }
    
    
    template <typename _PixT> void VSMD::_estimate(Pyramids<_PixT>& aPt,
                                                   VSFrame&          aFrame)
    {
        OMP_ALIAS(md, this)
        OMP_PARALLEL_FOR(_threadsCnt,
                         omp parallel for shared(md),
                         (unsigned idx = 0; idx < _cells.list.size(); ++idx))
        {
            /*
             * Calculates fast filter
             */
            auto&                 cell = _cells.list[idx];
            unsigned              p    { aPt.fm[_idxCurrent].size() - 1 };
            const VectS           rb   { - int(_detectRange >> (p + 1)) };
            const VectS           re   {   int(_detectRange >> (p + 1)) };
            const VectU           size { cell.size          >>  p       };
            const VectU           pos  { cell.position      >>  p       };
            Frame::Canvas<_PixT>& curr { aPt.fm[_idxCurrent][   p]      };
            const unsigned        t    { Direction::frame2vidx(_idx)    };
            
            {
                Frame::Canvas<_PixT>& prev { aPt.fm[_idxPrev][p]                  };
                VectIterSSpiral       i    { rb, re                               };
                unsigned              min  { std::numeric_limits<unsigned>::max() };
                const unsigned        did  { Cell::ptype2dir(aPt.PTYPE_SW)        };
                
                do
                {
                    unsigned crl { _corelate(curr, prev, pos, pos + i(), size, min) };
                    
                    if (min > crl)
                    {
                        min                         = crl;
                        cell.direction[did].vect[t] = i();
                    }
                }
                while (i.next());
                
                cell.direction[did].vect[t] *= (1U << p);
            }
            
            
            /*
             * Calculates slow filter
             */
            for (unsigned idx = aPt.PTYPE_SLOW_A; idx < aPt.PTYPE_COUNT; ++idx)
            {
                Frame::Canvas<_PixT>& prev { aPt.fm[idx][p] };
                VectIterSSpiral       i    { rb, re         };
                unsigned              min  { std::numeric_limits<unsigned>::max() };
                const unsigned        did  { Cell::ptype2dir(idx)                 };
                
                do
                {
                    unsigned crl { _corelate(curr, prev, pos, pos + i(), size, min) };
                    
                    if (min > crl)
                    {
                        min                         = crl;
                        cell.direction[did].vect[t] = i();
                    }
                }
                while (i.next());
                
                cell.direction[did].vect[t] *= (1U << p);
            }
        }
    }
    
    
    template <typename _PixT> void VSMD::_analyze(Pyramids<_PixT>& aPt,
                                                  VSFrame&         aFrame)
    {
        /*
         * Analyze all time layers
         */
        for (unsigned idx = aPt.PTYPE_SW; idx < aPt.PTYPE_COUNT; ++idx)
        {
            /*
             * Get time index of current and previous frame
             */
            const unsigned t0  { Direction::frame2vidx(_idx)     };
            const unsigned t1  { Direction::frame2vidx(_idx - 1) };
            const unsigned did { Cell::ptype2dir(aPt.PTYPE_SW)   };
            
            for (auto& cell : _cells.list)
            {
                /*
                 * Value is calculated by Calman filter. Analyzes
                 * deviations according to contrast, history and
                 * surroundings. Deviated vectors are estimated
                 * from history and surroundings. Cell is marked
                 * invalid if deviation is too high.
                 */
                auto& dir  = cell.direction[did];
                auto& v0   { dir.vect[t0] };
                auto  v0qs { v0.qsize()   };
                
                if (int(_minQSize) < v0qs)
                {
                    auto& v1          { dir.vect[t1]                              };
                    auto  va          = _analyze_avg(cell.idx, did, t0);
                    auto  dt          { v0 - v1                                   };
                    auto  ds          { v0 - va                                   };
                    auto  estimated   { (va + v1) / 2                             };
                    auto& measured    = dir.vect[t0];
                    auto  qfMeasured  { unsigned(4 * v0qs)                        };
                    auto  qfEstimated { (dt.qsize() + 4 * ds.qsize())* _devFactor };
                    
                    dir.vect[t0] =
                        (measured * qfMeasured + estimated * qfEstimated)
                        /
                        (qfMeasured + qfEstimated);
                        
                    /*
                     * We uses Calman filter but we would like to remove cells
                     * with big deviation.
                     */
                    if (qfEstimated > qfMeasured)
                    {
                        dir.valid    = false;
//                        dir.vect[t0] = (va + v1) / 2;
                    }
                }
                
                /*
                 * Also low contrast area would be invalidated
                 */
                if (0 == cell.qfContrast)
                {
                    dir.valid = false;
                }
            }
        }
    }
    
    
    VSMD::VectS VSMD::_analyze_avg(VectU    aPos,
                                   unsigned aDid,
                                   unsigned aTi)
    {
        VectS acc {   };
        int   div { 0 };
        
        ++aPos.x;
        
        if (aPos.x < _cells.dim.x)
        {
            acc = _cells[aPos].direction[aDid].vect[aTi];
            ++div;
        }
        
        ++aPos.y;
        
        if ((aPos.x < _cells.dim.x) && (aPos.y < _cells.dim.y))
        {
            acc += _cells[aPos].direction[aDid].vect[aTi];
            ++div;
        }
        
        --aPos.x;
        
        if (aPos.y < _cells.dim.y)
        {
            acc += _cells[aPos].direction[aDid].vect[aTi];
            ++div;
        }
        
        --aPos.x;
        
        if ((aPos.x < 65536U) && (aPos.y < _cells.dim.y))
        {
            acc += _cells[aPos].direction[aDid].vect[aTi];
            ++div;
        }
        
        --aPos.y;
        
        if (aPos.x < 65536U)
        {
            acc += _cells[aPos].direction[aDid].vect[aTi];
            ++div;
        }
        
        --aPos.y;
        
        if ((aPos.x < 65536U) && (aPos.y < 65536U))
        {
            acc += _cells[aPos].direction[aDid].vect[aTi];
            ++div;
        }
        
        ++aPos.x;
        
        if (aPos.y < 65536U)
        {
            acc += _cells[aPos].direction[aDid].vect[aTi];
            ++div;
        }
        
        ++aPos.x;
        
        if ((aPos.x < _cells.dim.x) && (aPos.y < 65536U))
        {
            acc += _cells[aPos].direction[aDid].vect[aTi];
            ++div;
        }
        
        acc /= div;
        return acc;
    }
    
    
    template <typename _PixT> void VSMD::_finalize(Pyramids<_PixT>& aPt,
                                                   VSFrame&          aFrame)
    {
        OMP_ALIAS(md, this)
        OMP_PARALLEL_FOR(_threadsCnt,
                         omp parallel for shared(md),
                         (unsigned idx = 0; idx < _cells.list.size(); ++idx))
        {
            /*
             * Calculates fast filter
             */
            auto&          cell = _cells.list[idx];
            const VectS    rb   { -1 };
            const VectS    re   {  1 };
            const unsigned t    { Direction::frame2vidx(_idx) };
            
            for (unsigned p = aPt.fm[_idxCurrent].size() - 2; p < 0x7FFFU; --p)
            {
                const unsigned did  { Cell::ptype2dir(aPt.PTYPE_SW) };
                auto&          dir = cell.direction[did];
                
                if (!dir.valid)
                {
                    continue;
                }
                
                const VectU           size { cell.size           >> p  };
                const VectU           pos  { cell.position       >> p  };
                const VectS           dv   { dir.vect[t] / (1U   << p) };
                Frame::Canvas<_PixT>& curr { aPt.fm[_idxCurrent][   p] };
                Frame::Canvas<_PixT>& prev { aPt.fm[_idxPrev][      p] };
                VectIterS             i    { rb + dv, re + dv          };
                unsigned              min  { std::numeric_limits<unsigned>::max() };
                
                do
                {
                    unsigned crl { _corelate(curr, prev, pos, pos + i(), size, min) };
                    
                    if (min > crl)
                    {
                        min         = crl;
                        dir.vect[t] = i();
                    }
                }
                while (i.next());
                
                dir.vect[t] *= (1U << p);
            }
            
            
            /*
             * Calculates slow filter
             */
            for (unsigned idx = aPt.PTYPE_SLOW_A; idx < aPt.PTYPE_COUNT; ++idx)
            {
                for (unsigned p = aPt.fm[_idxCurrent].size() - 2; p < 0x7FFFU; --p)
                {
                    const unsigned did  { Cell::ptype2dir(idx) };
                    auto&          dir = cell.direction[did];
                    
                    if (!dir.valid)
                    {
                        continue;
                    }
                    
                    const VectU           size { cell.size           >> p  };
                    const VectU           pos  { cell.position       >> p  };
                    const VectS           dv   { dir.vect[t] / (1U   << p) };
                    Frame::Canvas<_PixT>& curr { aPt.fm[_idxCurrent][   p] };
                    Frame::Canvas<_PixT>& prev { aPt.fm[idx][           p] };
                    VectIterSSpiral       i    { rb + dv, re + dv          };
                    unsigned              min  { std::numeric_limits<unsigned>::max() };
                    
                    do
                    {
                        unsigned crl { _corelate(curr, prev, pos, pos + i(), size, min) };
                        
                        if (min > crl)
                        {
                            min         = crl;
                            dir.vect[t] = i();
                        }
                    }
                    while (i.next());
                    
                    dir.vect[t] *= (1U << p);
                }
            }
        }
    }
    
    
    template <typename _PixT> void VSMD::_visualize(Pyramids<_PixT>& aPt,
                                                    VSFrame&          aFrame)
    {
        Frame::Canvas<_PixT> disp { (_PixT*)aFrame.data[0], fi.dim() };
        const unsigned       e    { unsigned(_cells.list.size())     };
        const unsigned       t0   { Direction::frame2vidx(_idx)      };
        const unsigned       t1   { Direction::frame2vidx(_idx - 1)  };
        const unsigned       t2   { Direction::frame2vidx(_idx - 2)  };
        const unsigned       t3   { Direction::frame2vidx(_idx - 2)  };
        
        
        OMP_ALIAS(md, this)
        OMP_PARALLEL_FOR(_threadsCnt,
                         omp parallel for shared(md),
                         (unsigned idx = 0; idx < e; ++idx))
        {
            auto& i = _cells.list[idx];
            
            
            /*
             * Show slow filters
             */
//            for (unsigned idx = aPt.PTYPE_SLOW_A; idx < aPt.PTYPE_COUNT; ++idx)
//            {
//                const unsigned did { Cell::ptype2dir(idx)           };
//                VectU          pos { i.position                     };
//                VectU          dst { pos + i.direction[did].vect[t0] };
//                VectU          rs  { 16                             };
//                disp.drawRectangle(pos, rs, _PixT(0));
//
//                if (i.direction[did].valid)
//                {
//                    _PixT    x { idx < aPt.PTYPE_STATIC_A ? _PixT(64) : _PixT(128)};
//                    unsigned w { idx < aPt.PTYPE_STATIC_A ? 2U        : 1U};
//
//                    disp.drawBox(      dst, rs - 2, x);
//                    disp.drawRectangle(dst, rs,     x);
//                    disp.drawLine(pos, dst, w,      x);
//                }
//            }


            /*
             * Show fast filters
             */
            {
                const unsigned did { Cell::ptype2dir(aPt.PTYPE_SW)  };
                VectU          pos { i.position                     };
                VectU          dst { pos + i.direction[did].vect[t0] };
                
                if (i.direction[did].valid)
                {
                    if (i.qfContrast > _contrastThreshold)
                    {
                        VectU rs1 { i.size - 4 };
                        VectU rs2 { i.size - 8 };
                        
                        disp.drawBox(pos + 1, rs1, _PixT(0));
                        disp.drawBox(pos + 1, rs2, _PixT(255));
                    }
                    else
                    {
                        VectU rs1 { i.size - 20 };
                        VectU rs2 { i.size - 24 };
                        
                        disp.drawBox(pos + 1, rs1, _PixT(0));
                        disp.drawBox(pos + 1, rs2, _PixT(255));
                    }
                    
                    VectU dst1 { dst  + i.direction[did].vect[t1] };
                    VectU dst2 { dst1 + i.direction[did].vect[t2] };
                    VectU dst3 { dst2 + i.direction[did].vect[t3] };
                    disp.drawLine(dst2, dst3, 1, _PixT(0));
                    disp.drawLine(dst1, dst2, 2, _PixT(0));
                    disp.drawLine(dst,  dst1, 3, _PixT(0));
                    disp.drawLine(pos,  dst,  4, _PixT(255));
                }
                else
                {
                    disp.drawLine(pos,  dst,  2, _PixT(0));
                }
            }
        }
    }
    
    
    template <typename _PixT> unsigned VSMD::_corelate(const Frame::Canvas<_PixT>& aCurrCanvas,
                                                       const Frame::Canvas<_PixT>& aPrevCanvas,
                                                       const VectS&                 aCurrShift,
                                                       const VectS&                 aPrevShift,
                                                       const VectU&                 aRect,
                                                       unsigned                    aTrh) const
    {
        VectIterU i   { aRect };
        unsigned  acc { 0     };
        
        do
        {
            int v1 { aCurrCanvas[aCurrShift + i()].abs() };
            int v2 { aPrevCanvas[aPrevShift + i()].abs() };
            
            acc += abs(v1 - v2);
            
            if (aTrh < acc)
            {
                break;
            }
        }
        while (i.next());
        
        return acc;
    }
}



























namespace
{
#if defined(USE_SSE2)
    unsigned char _mask[16] = {0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00};
#endif
    
    
    enum _KernelId
    {
        KRN___BLUR_H,
        KRN___BLUR_V
    };
    
    
    class _Spiral
    {
    public:
        _Spiral()
            :
            i      {       },
            _limit { 1     },
            _step  { 0     },
            _dir   { _LEFT }
        {
        
        }
        
        
        /**
         * @brief   Check if spiral index is in range
         * @param   aMaxShift    Range
         * @return  Result
         */
        inline bool operator<(int aMaxShift)
        {
            return (i.y >= -aMaxShift) &&
                   (i.y <=  aMaxShift) &&
                   (i.x >= -aMaxShift) &&
                   (i.x <=  aMaxShift);
        }
        
        
        /**
         * @brief   Spiral indexing
         * @param   aStepSize   Step size
         */
        inline _Spiral& operator+=(int aStepSize)
        {
            ++_step;
            
            switch (_dir)
            {
                case _LEFT:
                    i.x += aStepSize;
                    
                    if (_step == _limit)
                    {
                        _dir  = _DOWN;
                        _step = 0;
                    }
                    
                    break;
                    
                    
                case _DOWN:
                    i.y += aStepSize;
                    
                    if (_step == _limit)
                    {
                        _dir  = _RIGHT;
                        _step = 0;
                        ++_limit;
                    }
                    
                    break;
                    
                    
                case _RIGHT:
                    i.x -= aStepSize;
                    
                    if (_step == _limit)
                    {
                        _dir  = _UP;
                        _step = 0;
                    }
                    
                    break;
                    
                    
                case _UP:
                    i.y -= aStepSize;
                    
                    if (_step == _limit)
                    {
                        _dir  = _LEFT;
                        _step = 0;
                        ++_limit;
                    }
                    
                    break;
            }
            
            return *this;
        }
        
        
        Vec i;
        
        
    private:
        enum _Direction
        {
            _LEFT,
            _DOWN,
            _RIGHT,
            _UP
        };
        
        int        _limit;
        int        _step;
        _Direction _dir;
    };
    
    
    
    
    const char* _fmt2str(VSPixelFormat aFmt)
    {
        switch (aFmt)
        {
            case PF_NONE:
                return "NONE";
            case PF_GRAY8:
                return "GRAY8";
            case PF_YUV420P:
                return "YUV420P";
            case PF_YUV422P:
                return "YUV422P";
            case PF_YUV444P:
                return "YUV444P";
            case PF_YUV410P:
                return "YUV410P";
            case PF_YUV411P:
                return "YUV411P";
            case PF_YUV440P:
                return "YUV440P";
            case PF_YUVA420P:
                return "YUVA420P";
            case PF_PACKED:
                return "PACKED";
            case PF_RGB24:
                return "RGB24";
            case PF_BGR24:
                return "BGR24";
            case PF_RGBA:
                return "RGBA";
            default:
                return "unknown";
        }
    }
    
    
    /*
     * Compares contrast_idx structures respect to the contrast
     * (for sort function)
     */
    bool _cmp_Contrast(const VidStab::ContrastIdx& ci1,
                       const VidStab::ContrastIdx& ci2)
    {
        return ci1.contrast < ci2.contrast;
    }
    
    
    short lm_match_better(void* thresh, void* lm)
    {
        if (((LocalMotion*)lm)->match <= *((double*)thresh))
        {
            return 1;
        }
        else
        {
            return 0;
        }
    }
}


namespace VidStab
{
    void VSMD::_blur(const Frame::Frame& aFrame)
    {
        int stepSize;
        
        if (fiInfoC.pFormat > PF_PACKED)
        {
            /*
             * We could calculate a gray-scale version and use
             * the PLANAR stuff afterwards so far smoothing is
             * not needed.
             */
            stepSize = 1;
        }
        else
        {
            /*
             * Box-kernel smoothing (plain average of pixels),
             * which is fine for us.
             */
            stepSize = conf.stepSize;
        }
        
        
        const Frame::Frame& f = _blurBox(aFrame,
                                         stepSize,
                                         _BoxBlurNoColor);
                                         
                                         
        /*
         * Physical copy of instance is required instead of copy operator
         */
        memcpy(&curr, &f, sizeof(curr));
    }
    
    
    const Frame::Frame& VSMD::_blurBox(const Frame::Frame& aSrc,
                                       unsigned int        aStepSize,
                                       _BoxBlurColorMode   aColormode)
    {
        if (aStepSize < 2)
        {
            return aSrc;
        }
        
        
        const Frame::Info& fi = aSrc.info();
        
        
        // odd and larger than 2 and maximally half of smaller image dimension
        aStepSize  = VS_CLAMP(aStepSize | 1U,
                              3,
                              unsigned(VS_MIN(fi.height() / 2, fi.width() / 2)));
                              
        Frame::Plane planeDst { currPrep[ 0 ] };
        Frame::Plane planeSrc { aSrc[     0 ] };
        Frame::Plane planeBuf { currTmp[  0 ] };
        
        _blurBoxHV(planeDst, planeBuf, planeSrc, aStepSize);
        
        
        int size2 = aStepSize / 2 + 1; // odd and larger than 0
        
        switch (aColormode)
        {
            case _BoxBlurColor:
                // color
                if (size2 > 1)
                {
                    for (int plane = 1; plane < fi.planes(); ++plane)
                    {
                        planeDst = currPrep[ plane ];
                        planeSrc = aSrc[     plane ];
                        planeBuf = currTmp[  plane ];
                        
                        _blurBoxHV(planeDst, planeBuf, planeSrc, size2);
                    }
                }
                break;
                
                
            case _BoxBlurKeepColor:
                // copy both color channels
                for (int plane = 1; plane < fi.planes(); plane++)
                {
                    planeDst = currPrep[ plane ];
                    planeSrc = aSrc[     plane ];
                    planeDst = planeSrc;
                }
                break;
                
                
            default:
                break;
        }
        
        return currPrep;
    }
    
    
    void VSMD::_blurBoxHV(Frame::Plane&        aDst,
                          Frame::Plane&        aTmp,
                          const Frame::Plane&  aSrc,
                          int                  aSize)
    {
#if defined(USE_OPENCL)
        /*
         * Prepare CL buffers
         */
        const std::size_t size = std::size_t(fi.width() * fi.height());
        
        int args[]
        {
            fi.width(),
            fi.height(),
            aDst.lineSize(),
            aSrc.lineSize(),
            aSize
        };
        
        
        /*
         * Create horizontal blur buffers
         */
        OpenCl::Buffer buffer_args(*_clContext, CL_MEM_READ_ONLY,  sizeof(args));
        OpenCl::Buffer buffer_src( *_clContext, CL_MEM_READ_ONLY,  size);
        OpenCl::Buffer buffer_tmp( *_clContext, CL_MEM_READ_WRITE, size);
        OpenCl::Buffer buffer_dst( *_clContext, CL_MEM_READ_WRITE, size);
        
        
        /*
         * Process horizontal blur: src -> tmp
         */
        OpenCl::CommandQueue clQ(*_clContext, _clDevice);
        
        clQ.enqueueWriteBuffer(buffer_args, CL_TRUE, 0, sizeof(args), args);
        clQ.enqueueWriteBuffer(buffer_src,  CL_TRUE, 0, size,         aSrc.buff());
        
        OpenCl::Kernel blurH(*(_clProgram[KRN___BLUR_H]), _clProgramName[KRN___BLUR_H]);
        auto arg = blurH.setArg(0,   buffer_tmp);
        arg      = blurH.setArg(arg, buffer_src);
        arg      = blurH.setArg(arg, buffer_args);
        
        clQ.enqueueNDRangeKernel(blurH, cl::NullRange, cl::NDRange(fi.height()));
        
        
        /*
         * Process vertical blur: tmp -> dst
         */
        OpenCl::Kernel blurV(*(_clProgram[KRN___BLUR_V]), _clProgramName[KRN___BLUR_V]);
        arg = blurV.setArg(0,   buffer_dst);
        arg = blurV.setArg(arg, buffer_tmp);
        arg = blurV.setArg(arg, buffer_args);
        
        clQ.enqueueNDRangeKernel(blurV, cl::NullRange, cl::NDRange(fi.width()));
        
        clQ.enqueueReadBuffer(buffer_dst, CL_TRUE, 0, size, aDst.buff());
        
#else  /* defined(USE_OPENCL) */
        
        _blurBoxH(aTmp, aSrc, aSize);
        _blurBoxV(aDst, aTmp, aSize);
        
#endif /* defined(USE_OPENCL) */
    }
    
    
    void VSMD::_blurBoxH(Frame::Plane&        dst,
                         const Frame::Plane&  src,
                         const int            size)
    {
        const int size2 = size / 2; /* Size of one side of the kernel without center */
        
        OMP_ALIAS(md, this)
        OMP_PARALLEL_FOR(conf.numThreads,
                         omp parallel for shared(md, dst, src),
                         (int y = 0; y < fi.height(); ++y))
        {
            const unsigned char* inBegin = src.buff() + (y * src.lineSize());
            const unsigned char* inEnd   = inBegin + size2;
            unsigned int         acc     = (*inBegin) * (size2 + 1);
            
            /* Right half of kernel */
            for (const unsigned char* in = inBegin; in != inEnd; ++in)
            {
                acc += in[0];
            }
            
            /* Go through the image */
            unsigned char* out = dst.buff() + (y * dst.lineSize());
            
            for (int x = 0; x < fi.width(); ++x)
            {
                acc = acc + inEnd[0] - inBegin[0];
                
                if (x > size2)
                {
                    ++inBegin;
                }
                
                if (x < (fi.width() - size2 - 1))
                {
                    ++inEnd;
                }
                
                *out = acc / size;
                ++out;
            }
        }
    }
    
    
    void VSMD::_blurBoxV(Frame::Plane&        dst,
                         const Frame::Plane&  src,
                         const int            size)
    {
        const int size2 = size / 2; // size of one side of the kernel without center
        
        OMP_ALIAS(md, this)
        OMP_PARALLEL_FOR(conf.numThreads,
                         omp parallel for shared(md, dst, src),
                         (int x = 0; x < fi.width(); x++))
        {
            const unsigned char* start   = src.buff() + x;         // start and end of kernel
            const unsigned char* end     = start;
            unsigned char*       current = dst.buff() + x;         // current destination pixel
            int                  acc     = (*start) * (size2 + 1); // left half of kernel with first pixel
            
            // right half of kernel
            for (int k = 0; k < size2; k++)
            {
                acc += (*end);
                end += src.lineSize();
            }
            
            // go through the image
            for (int y = 0; y < fi.height(); y++)
            {
                acc = acc - (*start) + (*end);
                
                if (y > size2)
                {
                    start += src.lineSize();
                }
                
                if (y < fi.height() - size2 - 1)
                {
                    end += src.lineSize();
                }
                
                *current = acc / size;
                current += dst.lineSize();
            }
        }
    }
    
    void VSMD::_vs_detect(LocalMotions*       aMotions,
                          const Frame::Frame& aFrame)
    {
        LmList       motions       { *aMotions };
        LocalMotions                 motionsfineC;
        LmList       motionsfine   { motionsfineC };
        LocalMotions                 motionscoarseC;
        LmList       motionscoarse { motionscoarseC };
        int          num_motions   { _detectContrast(motionscoarse) };
        
        if (num_motions < 1)
        {
            motionsfine.init(0);
            vs_log_warn(conf.modName,
                        "Too low contrast. (no translations are detected in frame %i)\n",
                        frameNum);
        }
        else
        {
            /*
             * Calculates transformation and perform another scan with small fields
             */
            VSTransform        t = vsSimpleMotionsToTransform(fiInfoC, conf.modName, &motionscoarseC);
            fieldsfine.offset    = t;
            fieldsfine.useOffset = 1;
            fieldsfine.pt        = prepare_transform(&fieldsfine.offset, &fiInfoC);
            
            LocalMotions motions2;
            
            if (fiInfoC.pFormat > PF_PACKED)
            {
                motions2 = _calcTransFields(fieldsfine,
                                            visitor_calcFieldTransPacked,
                                            visitor_contrastSubImgPacked);
            }
            else
            {
                motions2 = _calcTransFields(fieldsfine,
                                            visitor_calcFieldTransPlanar,
                                            visitor_contrastSubImgPlanar);
            }
            
            /*
             * Through out those with bad match (worse than mean of coarse scan)
             */
            VSArray matchQualities1 = localmotionsGetMatch(&motionscoarseC);
            double  meanMatch       = cleanmean(matchQualities1.dat, matchQualities1.len, NULL, NULL);
            
            motionsfineC = vs_vector_filter(&motions2, lm_match_better, &meanMatch);
        }
        
        
//        if (conf.show)
//        {
//            Frame::Plane plane
//            {
//                aFrame[0]
//            };
//
//            Frame::Canvas<Frame::PixYUV> canvas
//            {
//                (Frame::PixYUV*)plane.buff(),
//                unsigned(plane.lineSize()),
//                unsigned(fi.height())
//            };
//
//            _draw(canvas, num_motions, motionscoarseC, motionsfineC);
//        }

        motions.concat(motionscoarse, motionsfine);
    }
    
    
    int VSMD::_detectContrast(LmList& aLmCoarse)
    {
        aLmCoarse.init(0);
        LocalMotions& motionscoarse = aLmCoarse.LocalMotionsC();
        
        
        fieldscoarse.pt = prepare_transform(&fieldsfine.offset, &fiInfoC);
        
        if (fiInfoC.pFormat > PF_PACKED)
        {
            motionscoarse = _calcTransFields(fieldscoarse,
                                             visitor_calcFieldTransPacked,
                                             visitor_contrastSubImgPacked);
        }
        else
        {
            motionscoarse = _calcTransFields(fieldscoarse,
                                             visitor_calcFieldTransPlanar,
                                             visitor_contrastSubImgPlanar);
        }
        
        return aLmCoarse.size();
    }
    
    
    /*
     * Select only the best 'maxfields' fields
     *
     * First calculate contrasts then select from each part of the frame
     * some fields. We may simplify here by using random. People want high
     * quality, so typically we use all.
     */
    void VSMD::_selectfields(std::vector<ContrastIdx>& goodflds,
                             VSMotionDetectFields&     fs,
                             contrastSubImgFunc        contrastfunc)
    {
        /*
         * Calculate contrast for each field
         */
        std::vector<ContrastIdx> ci { size_t(fs.fieldNum) };
        
        
        Field* f   = fs.fields;
        int    idx = 0;
        
        for (auto& i : ci)
        {
            i.contrast = contrastfunc(*this, *f);
            i.index    = idx;
            
            if (i.contrast < fs.contrastThreshold)
            {
                i.contrast = 0;
            }
            
            ++f;
            ++idx;
        }
        
        
        /*
         * Get best fields from each segment
         *
         * We split all fields into row + 1 segments and take
         * from each segment the best fields.
         */
        std::vector<ContrastIdx> ci_segms { ci.begin(), ci.end()         };
        int                             numsegms { fs.fieldRows + 1            };
        int                             segmlen  { fs.fieldNum  / numsegms + 1 };
        
        for (int i = 0; i < numsegms; i++)
        {
            int startindex = segmlen * i;
            int endindex   = segmlen * (i + 1);
            endindex       = (endindex > fs.fieldNum) ? fs.fieldNum : endindex;
            
            /*
             * Sort within segment
             */
            auto b = ci_segms.begin() + startindex;
            auto e = b +    (endindex - startindex);
            std::sort(b,  e, _cmp_Contrast);
            
            /*
             * Take maxfields / numsegms
             */
            for (int j = 0; j < (fs.maxFields / numsegms); j++)
            {
                if ((startindex + j) >= endindex)
                {
                    continue;
                }
                
                if (ci_segms[startindex + j].contrast > 0)
                {
                    /*
                     * Don't consider them in the later selection process
                     */
                    goodflds.push_back(ci[ci_segms[startindex + j].index]);
                    ci_segms[startindex + j].contrast = 0;
                }
            }
        }
        
        
        /*
         * Split the frame list into rows+1 segments.
         * Check whether enough fields are selected.
         */
        int remaining = fs.maxFields - goodflds.size();
        
        if (remaining > 0)
        {
            /*
             * Take the remaining from the leftovers
             */
            auto b = ci_segms.begin();
            std::sort(b, b + fs.fieldNum, _cmp_Contrast);
            
            for (int j = 0; j < remaining; j++)
            {
                if (ci_segms[j].contrast > 0)
                {
                    goodflds.push_back(ci_segms[j]);
                }
            }
        }
    }
    
    
    /* calculates the optimal transformation for one field in Packed
     * slower than the Planar version because it uses all three color channels
     */
    LocalMotion visitor_calcFieldTransPacked(VSMD&                       md,
                                             const VSMotionDetectFields& fs,
                                             const Field&                field,
                                             int                         fieldnum)
    {
        int                tx       = 0;
        int                ty       = 0;
        
        const Frame::Plane curr     = md.curr[0];
        const Frame::Plane prev     = md.prev[0];
        int                width1   = curr.lineSize() / 3;
        int                width2   = prev.lineSize() / 3;
        int                stepSize = fs.stepSize;
        int                maxShift = fs.maxShift;
        
        Vec offset              = { 0, 0};
        LocalMotion lm          = null_localmotion();
        
        if (fs.useOffset)
        {
            offset = transform_vec(&fs.pt, (Vec*)&field);
            
            /*
             * Is the field still in the frame
             */
            if (((offset.x - maxShift - stepSize) < 0) || ((offset.x + maxShift + stepSize) >= md.fiInfoC.width) ||
                ((offset.y - maxShift - stepSize) < 0) || ((offset.y + maxShift + stepSize) >= md.fiInfoC.height))
            {
                lm.match = -1;
                return lm;
            }
        }
        
        /*
         * Here we improve speed by checking first the most probable position
         * then the search paths are most effectively cut. (0,0) is a simple start
         */
        Correlate correlate
        {
            curr.buff(),
            prev.buff(),
            field,
            width1,
            width2,
            md.fiInfoC.height,
            3
        };
        
        unsigned int minerror = correlate(offset.x, offset.y, UINT_MAX);
        
        
        /*
         * Check all positions...
         */
        for (int i = -maxShift; i <= maxShift; i += stepSize)
        {
            for (int j = -maxShift; j <= maxShift; j += stepSize)
            {
                if ( i == 0 && j == 0 )
                {
                    /*
                     * No need to check this since already done
                     */
                    continue;
                }
                
                unsigned int error = correlate(i + offset.x,
                                               j + offset.y,
                                               minerror);
                                               
                if (error < minerror)
                {
                    minerror = error;
                    tx = i;
                    ty = j;
                }
            }
        }
        
        
        /*
         * Make fine grain check around the best match
         */
        if (stepSize > 1)
        {
            int txc = tx; // save the shifts
            int tyc = ty;
            int r   = stepSize - 1;
            
            for (int i = txc - r; i <= txc + r; i += 1)
            {
                for (int j = tyc - r; j <= tyc + r; j += 1)
                {
                    if (i == txc && j == tyc)
                    {
                        /*
                         * No need to check this since already done
                         */
                        continue;
                    }
                    
                    unsigned int error = correlate(i + offset.x,
                                                   j + offset.y,
                                                   minerror);
                                                   
                    if (error < minerror)
                    {
                        minerror = error;
                        tx = i;
                        ty = j;
                    }
                }
            }
        }
        
        if ((fabs(tx) >= (maxShift + stepSize - 1)) ||
            (fabs(ty) >= (maxShift + stepSize - 1)))
        {
            lm.match = -1;
            return lm;
        }
        
        lm.f     = field;
        lm.v.x   = tx + offset.x;
        lm.v.y   = ty + offset.y;
        lm.match = ((double)minerror) / (field.size * field.size);
        
        return lm;
    }
    
    
    /*
     * Calculates the optimal transformation for one field in Planar frames
     * (only luminance)
     */
    LocalMotion visitor_calcFieldTransPlanar(VSMD&                       md,
                                             const VSMotionDetectFields& fs,
                                             const Field&                field,
                                             int                         fieldnum)
    {
        Vec                t      {             };
        const Frame::Plane curr = { md.curr[0] };
        const Frame::Plane prev = { md.prev[0] };
        
        /*
         * We only use the luminance part of the image
         */
        int         stepSize = fs.stepSize;
        int         maxShift = fs.maxShift;
        LocalMotion lm       = null_localmotion();
        Vec         offset { };
        
        if (fs.useOffset)
        {
            Vec fieldpos = {field.x, field.y};
            
            offset = transform_vec(&fs.pt, &fieldpos) - fieldpos;
            
            
            /*
             * Is the field still in the frame
             */
            int s2 = field.size / 2;
            
            if (unlikely((fieldpos.x + offset.x - s2 - maxShift - stepSize) < 0             ||
                         (fieldpos.x + offset.x + s2 + maxShift + stepSize) >= md.fiInfoC.width ||
                         (fieldpos.y + offset.y - s2 - maxShift - stepSize) < 0             ||
                         (fieldpos.y + offset.y + s2 + maxShift + stepSize) >= md.fiInfoC.height))
            {
                lm.match = -1;
                return lm;
            }
        }
        
        
        Correlate correlate
        {
            curr.buff(),
            prev.buff(),
            field,
            curr.lineSize(),
            prev.lineSize(),
            md.fiInfoC.height,
            1
        };
        
#if !defined(DISABLE_SPIRAL_FIELD_CALC)
        
        
        unsigned int minerror = UINT_MAX;
        
        for (_Spiral sp; sp < maxShift; sp += stepSize)
        {
            unsigned int error = correlate(sp.i.x + offset.x,
                                           sp.i.y + offset.y,
                                           minerror);
                                           
            if (error < minerror)
            {
                minerror = error;
                t        = sp.i;
            }
        }
        
        
#else
        
        
        /*
         * Here we improve speed by checking first the most probable position
         * then the search paths are most effectively cut. (0,0) is a simple
         * start
         */
        unsigned int minerror = correlate(0, 0, UINT_MAX);
        
        
        /*
         * Check all positions...
         */
        for (int i = -maxShift; i <= maxShift; i += stepSize)
        {
            for (int j = -maxShift; j <= maxShift; j += stepSize)
            {
                if ( i == 0 && j == 0 )
                {
                    /*
                     * No need to check this since already done
                     */
                    continue;
                }
        
                unsigned int error = correlate(i + offset.x,
                                               j + offset.y,
                                               minerror);
        
                if (error < minerror)
                {
                    minerror = error;
                    t.x = i;
                    t.y = j;
                }
            }
        }
#endif
        
        
        /*
         * Make fine grain check around the best match
         */
        while (stepSize > 1)
        {
            /*
             * Save the shifts
             */
            Vec tc { t };
            int newStepSize = stepSize / 2;
            int r = stepSize - newStepSize;
            for (int i = tc.x - r; i <= tc.x + r; i += newStepSize)
            {
                for (int j = tc.y - r; j <= tc.y + r; j += newStepSize)
                {
                    if ((i == tc.x) && (j == tc.y))
                    {
                        /*
                         * No need to check this since already done
                         */
                        continue;
                    }
                    
                    unsigned int error = correlate(i + offset.x,
                                                   j + offset.y,
                                                   minerror);
                                                   
                    if (error < minerror)
                    {
                        minerror = error;
                        t.x = i;
                        t.y = j;
                    }
                }
            }
            
            stepSize /= 2;
        }
        
        if (unlikely(fabs(t.x) >= maxShift + stepSize - 1  ||
                     fabs(t.y) >= maxShift + stepSize))
        {
            /*
             * To be kicked out
             */
            lm.match = -1.0;
            return lm;
        }
        
        lm.f     = field;
        lm.v.x   = t.x + offset.x;
        lm.v.y   = t.y + offset.y;
        lm.match = ((double) minerror) / (field.size * field.size);
        
        return lm;
    }
    
    
    double visitor_contrastSubImgPacked(VSMD&        md,
                                        const Field& field)
    {
        Frame::Plane pl { md.curr[0] };
        int linesize2 = pl.lineSize() / 3; // linesize in pixels
        
        return (contrastSubImg(pl.buff() + 0, &field, linesize2, md.fi.height(), 3) +
                contrastSubImg(pl.buff() + 1, &field, linesize2, md.fi.height(), 3) +
                contrastSubImg(pl.buff() + 2, &field, linesize2, md.fi.height(), 3)) / 3;
    }
    
    
    double visitor_contrastSubImgPlanar(VSMD&        md,
                                        const Field& field)
    {
        Frame::Plane pl { md.curr[0] };
        
#ifdef USE_SSE2
        return contrastSubImg1_SSE(pl.buff(),
                                   &field,
                                   pl.lineSize(),
                                   md.fi.height());
#else
        return contrastSubImg(pl.buff(),
                              &field,
                              pl.lineSize(),
                              md.fi.height(),
                              1);
#endif
    }
    
    
    void VSMD::_initOpenCl()
    {
#if defined(USE_OPENCL)
        _initOpenCl_selectDevice();
        _initOpenCl_prepareKernels();
#endif
    }
    
    
#if defined(USE_OPENCL)
    void VSMD::_initOpenCl_selectDevice()
    {
        vs_log_info(_mn.c_str(), "[OpenCL] Devices:\n");
        
        
        bool first { true };
        
        for (auto& platform : OpenCl::devices)
        {
            for (auto& device : platform)
            {
                if (first)
                {
                    vs_log_info(_mn.c_str(),
                                "[OpenCL] --> %s\n",
                                device.getInfo<CL_DEVICE_NAME>().c_str());
                    _clDevice = device;
                    first     = false;
                }
                else
                {
                    vs_log_info(_mn.c_str(),
                                "[OpenCL]     %s\n",
                                device.getInfo<CL_DEVICE_NAME>().c_str());
                }
            }
        }
        
        if (first)
        {
            throw Common::VS_EXCEPTION("There is no device available!");
        }
        
        _clContext = new cl::Context({_clDevice});
    }
    
    
    void VSMD::_initOpenCl_prepareKernels()
    {
        cl::Program::Sources           opencl___blur_h_src;
        opencl___blur_h_src.push_back({opencl___blur_h,  opencl___blur_h_len});
        _clSources.push_back(          opencl___blur_h_src);
        _clProgramName.push_back(      "blurH");
        
        cl::Program::Sources           opencl___blur_v_src;
        opencl___blur_v_src.push_back({opencl___blur_v,  opencl___blur_v_len});
        _clSources.push_back(          opencl___blur_v_src);
        _clProgramName.push_back(      "blurV");
        
        
        for (auto& src : _clSources)
        {
            cl::Program* pgm = new cl::Program(*_clContext, src);
            
            if (CL_SUCCESS != pgm->build({_clDevice}))
            {
                throw Common::VS_EXCEPTION("OpenCL build error:\n%s\n", pgm->getBuildInfo<CL_PROGRAM_BUILD_LOG>(_clDevice).c_str());
            }
            
            _clProgram.push_back(pgm);
        }
        
        vs_log_info(_mn.c_str(), "[OpenCL] Kernels built successfully\n");
    }
#endif /* !defined(USE_OPENCL) */
    
    
    void VSMD::_initMsg()
    {
        vs_log_info(_mn.c_str(), "[filter] Info:\n");
        vs_log_info(_mn.c_str(), "[filter] \tbuilt - " __DATE__ "\n");
        vs_log_info(_mn.c_str(), "[filter] \tflags - "
#if defined(USE_OPENCL)
                    "OpenCL "
#endif
#if defined(USE_OMP)
                    "OMP "
#endif
#if defined(USE_SSE2)
                    "SSE2 "
#endif
                    
                    "\n");
    }
    
    
    void VSMD::_initVsDetect(const VSMotionDetectConfig* aConf,
                             const VSFrameInfo*          aFi)
    {
        /*
         * First of all check inputs before we use them
         */
        if (nullptr == aConf)
        {
            throw Common::VS_EXCEPTION("Configuration structure is NULL!");
        }
        
        if (nullptr == aFi)
        {
            throw Common::VS_EXCEPTION("Frame info is NULL!");
        }
        
        conf    = *aConf;
        fiInfoC = *aFi;
        
        
        /*
         * Is requested pixel format supported by us?
         */
        if ((fi.pixFormat() <= PF_NONE)   ||
            (fi.pixFormat() == PF_PACKED) ||
            (fi.pixFormat() >= PF_NUMBER))
        {
            throw Common::VS_EXCEPTION("Unsupported Pixel Format (%i)", fi.pixFormat());
        }
        
        
#ifdef USE_OMP
        if (conf.numThreads == 0)
        {
            conf.numThreads = VS_MAX(omp_get_max_threads(), 1);
        }
        vs_log_info(conf.modName, "[filter] Multithreading: use %i threads\n", conf.numThreads);
#endif
        
        
        prev.alloc();
        currTmp.alloc();
        currPrep.alloc();
        
        frameNum        = 0;
        
        conf.shakiness  = VS_MIN(10, VS_MAX(1, conf.shakiness));
        conf.accuracy   = VS_MIN(15, VS_MAX(1, conf.accuracy));
        
        if ((conf.accuracy) < (conf.shakiness / 2))
        {
            conf.accuracy = conf.shakiness / 2;
            vs_log_info(conf.modName,
                        "[filter] Accuracy should not be lower than shakiness / 2 -- fixed to %i\n",
                        conf.accuracy);
        }
        
        if ((conf.accuracy > 9) && (conf.stepSize > 6))
        {
            conf.stepSize = 6;
            vs_log_info(conf.modName,
                        "[filter] For high accuracy use lower stepsize  -- fixed to %i now\n",
                        conf.stepSize);
        }
        
        int minDimension  = VS_MIN(fi.width(), fi.height());
        //  shift: shakiness 1: height/40; 10: height/4
        //  maxShift = VS_MAX(4,(minDimension*conf.shakiness)/40);
        //  size: shakiness 1: height/40; 10: height/6 (clipped)
        //  fieldSize = VS_MAX(4,VS_MIN(minDimension/6, (minDimension*conf.shakiness)/40));
        
        // fixed size and shift now
        int maxShift      = VS_MAX(16, minDimension / 7);
        int fieldSize     = VS_MAX(16, minDimension / 10);
        int fieldSizeFine = VS_MAX(6,  minDimension / 60);
        
#if defined(USE_SSE2)
        fieldSize     = (fieldSize     / 16 + 1) * 16;
        fieldSizeFine = (fieldSizeFine / 16 + 1) * 16;
#endif
        
        _initFields(fieldscoarse, fieldSize,     maxShift,      conf.stepSize, 1, 0,             conf.contrastThreshold);
        _initFields(fieldsfine,   fieldSizeFine, fieldSizeFine, 2,             1, fieldSizeFine, conf.contrastThreshold / 2);
    }
    
    
    void VSMD::_initFields(VSMotionDetectFields& fs,
                           int                   size,
                           int                   maxShift,
                           int                   stepSize,
                           short                 keepBorder,
                           int                   spacing,
                           double                contrastThreshold)
    {
        fs.fieldSize         = size;
        fs.maxShift          = maxShift;
        fs.stepSize          = stepSize;
        fs.useOffset         = 0;
        fs.contrastThreshold = contrastThreshold;
        
        int rows = VS_MAX(3, (fiInfoC.height - fs.maxShift * 2) / (size + spacing) -1);
        int cols = VS_MAX(3, (fiInfoC.width  - fs.maxShift * 2) / (size + spacing) -1);
        
        // make sure that the remaining rows have the same length
        fs.fieldNum  = rows * cols;
        fs.fieldRows = rows;
        
        if (!(fs.fields = (Field*)vs_malloc(sizeof(Field) * fs.fieldNum)))
        {
            throw Common::VS_EXCEPTION("Allocation failed!");
        }
        else
        {
            int border = fs.stepSize;
            // the border is the amount by which the field centers
            // have to be away from the image boundary
            // (stepsize is added in case shift is increased through stepsize)
            if (keepBorder)
            {
                border = size / 2 + fs.maxShift + fs.stepSize;
            }
            int step_x = (fiInfoC.width  - 2 * border) / VS_MAX(cols - 1, 1);
            int step_y = (fiInfoC.height - 2 * border) / VS_MAX(rows - 1, 1);
            
            for (int j = 0; j < rows; j++)
            {
                for (int i = 0; i < cols; i++)
                {
                    int idx = j * cols + i;
                    
                    fs.fields[idx].x    = border + i * step_x;
                    fs.fields[idx].y    = border + j * step_y;
                    fs.fields[idx].size = size;
                }
            }
        }
        
        
        fs.maxFields = (conf.accuracy) * fs.fieldNum / 15;
        
        vs_log_info(conf.modName,
                    "[filter] Fieldsize: %i, Maximal translation: %i pixel\n",
                    fs.fieldSize,
                    fs.maxShift);
                    
        vs_log_info(conf.modName,
                    "[filter] Number of used measurement fields: %i out of %i\n",
                    fs.maxFields,
                    fs.fieldNum);
    }
    
    
    LocalMotions VSMD::_calcTransFields(VSMotionDetectFields& fields,
                                        calcFieldTransFunc    fieldfunc,
                                        contrastSubImgFunc    contrastfunc)
    {
        LocalMotions                   localmotionsC;
        LmList          localmotions { localmotionsC };
        localmotions.init(fields.maxFields);
        
        
        std::vector<ContrastIdx> goodflds {};
        _selectfields(goodflds, fields, contrastfunc);
        
        
        OMP_ALIAS(md, this)
        OMP_PARALLEL_FOR(conf.numThreads,
                         omp parallel for shared(goodflds, md, localmotions),
                         (int index = 0; index < int(goodflds.size()); ++index))
        {
            int i = goodflds[index].index;
            
            LocalMotion m = fieldfunc(*this, fields, fields.fields[i], i); // e.g. calcFieldTransPlanar
            
            if (m.match >= 0)
            {
                m.contrast = goodflds[index].contrast;
                
                OMP_CRITICAL(localmotions_append)
                {
                    vs_vector_append_dup(&localmotionsC, &m, sizeof(LocalMotion));
                }
            }
        }
        
        return localmotionsC;
    }
    
    
}


#if defined(USE_SSE2)


unsigned int Correlate::operator ()(int          aOffsetX,
                                    int          aOffsetY,
                                    unsigned int aThreshold)
{
#ifndef USE_SSE2_CMP_HOR
    unsigned char summes[16];
#endif
    
    
    __m128i        xmmsum  = _mm_setzero_si128();
    __m128i        xmmmask = _mm_loadu_si128((__m128i const*)_mask);
    
    int            s2      = _field.size / 2;
    const uint8_t* curr    = _current  + ((_field.x - s2           ) + (_field.y - s2           ) * _widthCurrent)  * _bpp;
    const uint8_t* prev    = _previous + ((_field.x - s2 + aOffsetX) + (_field.y - s2 + aOffsetY) * _widthPrevious) * _bpp;
    unsigned int   sum     = 0;
    unsigned char  row     = 0;
    
    for (int j = 0; j < _field.size; j++)
    {
        for (int k = 0; k < _wbpp; k += 16)
        {
            {
                __m128i xmm0 = _mm_loadu_si128((__m128i const*)curr);
                __m128i xmm1 = _mm_loadu_si128((__m128i const*)prev);
                
                __m128i xmm2 = _mm_subs_epu8(xmm0, xmm1);
                xmm0         = _mm_subs_epu8(xmm1, xmm0);
                xmm0         = _mm_adds_epu8(xmm0, xmm2);
                
                xmm1         = _mm_and_si128(xmm0, xmmmask);
                xmm0         = _mm_srli_si128(xmm0, 1);
                xmm0         = _mm_and_si128(xmm0, xmmmask);
                
                xmmsum       = _mm_adds_epu16(xmmsum, xmm0);
                xmmsum       = _mm_adds_epu16(xmmsum, xmm1);
            }
            
            curr += 16;
            prev += 16;
            
            row++;
            if (row == SSE2_CMP_SUM_ROWS)
            {
                row = 0;
                
#ifdef USE_SSE2_CMP_HOR
                {
                    __m128i xmm1 = _mm_srli_si128(xmmsum, 8);
                    xmmsum       = _mm_adds_epu16(xmmsum, xmm1);
                    
                    xmm1         = _mm_srli_si128(xmmsum, 4);
                    xmmsum       = _mm_adds_epu16(xmmsum, xmm1);
                    
                    xmm1         = _mm_srli_si128(xmmsum, 2);
                    xmmsum       = _mm_adds_epu16(xmmsum, xmm1);
                    
                    sum         += _mm_extract_epi16(xmmsum, 0);
                }
#else
                _mm_storeu_si128((__m128i*)summes, xmmsum);
                
                for (int i = 0; i < 16; i += 2)
                {
                    sum += summes[i] + summes[i + 1] * 256;
                }
#endif
                xmmsum = _mm_setzero_si128();
            }
        }
        
        if (sum > aThreshold)
        {
            break;
        }
        
        curr += (_widthCurrent  - _field.size) * _bpp;
        prev += (_widthPrevious - _field.size) * _bpp;
    }
    
#if (SSE2_CMP_SUM_ROWS != 1) && \
    (SSE2_CMP_SUM_ROWS != 2) && \
    (SSE2_CMP_SUM_ROWS != 4) && \
    (SSE2_CMP_SUM_ROWS != 8) && \
    (SSE2_CMP_SUM_ROWS != 16)
    //process all data left unprocessed
    //this part can be safely ignored if
    //SSE_SUM_ROWS = {1, 2, 4, 8, 16}
#ifdef USE_SSE2_CMP_HOR
    {
        __m128i xmm1 = _mm_srli_si128(xmmsum, 8);
        xmmsum       = _mm_adds_epu16(xmmsum, xmm1);
        
        xmm1         = _mm_srli_si128(xmmsum, 4);
        xmmsum       = _mm_adds_epu16(xmmsum, xmm1);
        
        xmm1         = _mm_srli_si128(xmmsum, 2);
        xmmsum       = _mm_adds_epu16(xmmsum, xmm1);
        
        sum         += _mm_extract_epi16(xmmsum, 0);
    }
#else
    _mm_storeu_si128((__m128i*)summes, xmmsum);
    for (i = 0; i < 16; i += 2)
    {
        sum += summes[i] + summes[i + 1] * 256;
    }
#endif
#endif
    
    return sum;
}


#elif defined(USE_ORC)


unsigned int Correlate::operator ()(int          aOffsetX,
                                    int          aOffsetY,
                                    unsigned int aThreshold)
{
    const int      s2 = _field.size / 2;
    unsigned char* p1  = _current  + ((_field.x - s2)            + (_field.y - s2)            * _widthCurrent)  * _bpp;
    unsigned char* p2  = _previous + ((_field.x - s2 + aOffsetX) + (_field.y - s2 + aOffsetY) * _widthPrevious) * _bpp;
    
    unsigned int sum = 0;
    
    for (int j = 0; j < _field.size; j++)
    {
        unsigned int s = 0;
        image_line_difference_optimized(&s, p1, p2, _field.size * _bpp);
        
        sum += s;
        
        if (sum > aThreshold) // no need to calculate any longer: worse than the best match
        {
            break;
        }
        
        p1 += _widthCurrent  * _bpp;
        p2 += _widthPrevious * _bpp;
    }
    
    
    return sum;
}


#else


unsigned int Correlate::operator ()(int          aOffsetX,
                                    int          aOffsetY,
                                    unsigned int aThreshold)
{
    unsigned s2 = _field.size / 2;
    unsigned x  = _field.x - s2;
    unsigned y  = _field.y - s2;
    
    unsigned int   sum  = 0;
    const uint8_t* currFrameC = _current  + (x            +  y             * _widthCurrent)  * _bpp;
    const uint8_t* _prevFrameC = _previous + (x + aOffsetX + (y + aOffsetY) * _widthPrevious) * _bpp;
    
    for (int j = 0; j < _field.size; ++j)
    {
        for (int k = 0; k < _field.size * _bpp; k++)
        {
            sum += abs(int(*currFrameC) - int(*_prevFrameC));
            currFrameC++;
            _prevFrameC++;
        }
        
        if (sum > aThreshold)
        {
            /*
             * No need to calculate any longer: worse than the best match
             */
            break;
        }
        
        currFrameC += (_widthCurrent  - _field.size) * _bpp;
        _prevFrameC += (_widthPrevious - _field.size) * _bpp;
    }
    
    return sum;
}


#endif


VSMotionDetectConfig vsMotionDetectGetDefaultConfig(const char* modName)
{
    VSMotionDetectConfig conf;
    conf.stepSize          = 6;
    conf.accuracy          = 15;
    conf.shakiness         = 5;
    conf.virtualTripod     = 0;
    conf.contrastThreshold = 0.25;
    conf.show              = 0;
    conf.modName           = modName;
    conf.numThreads        = 0;
    return conf;
}


void vsMotionDetectGetConfig(VSMotionDetectConfig* aConf,
                             const VSMotionDetect* aMd)
{
    try
    {
        const VSMD& md = VSMD2Inst(aMd);
        *aConf    = md.conf;
    }
    catch (std::exception& exc)
    {
        vs_log_error("vidstabdetect", "[filter] Failed!\n");
        vs_log_error("vidstabdetect", "%s\n", exc.what());
        assert(false);
    }
    catch (...)
    {
        vs_log_error("vidstabdetect", "[filter] Failed!\n");
        vs_log_error("vidstabdetect", "Unknown failure type!\n");
        assert(false);
    }
}


const VSFrameInfo* vsMotionDetectGetFrameInfo(const VSMotionDetect* aMd)
{
    try
    {
        const VSMD& md = VSMD2Inst(aMd);
        return     &md.fiInfoC;
    }
    catch (std::exception& exc)
    {
        vs_log_error("vidstabdetect", "[filter] Failed!\n");
        vs_log_error("vidstabdetect", "%s\n", exc.what());
        assert(false);
    }
    catch (...)
    {
        vs_log_error("vidstabdetect", "[filter] Failed!\n");
        vs_log_error("vidstabdetect", "Unknown failure type!\n");
        assert(false);
    }
    
    return nullptr;
}


int vsMotionDetectInit(VSMotionDetect*             aMd,
                       const VSMotionDetectConfig* aConf,
                       const VSFrameInfo*          aFi)
{
    const char* modName = ((nullptr != aConf) ? aConf->modName : "vid.stab");
    
    if (nullptr == aMd)
    {
        vs_log_error(modName, "Can not initialize null instance!\n");
        return VS_ERROR;
    }
    
    try
    {
        VSMD* md   = new VSMD(modName, aMd, aConf, aFi);
        aMd->_inst = md;
    }
    catch (std::exception& exc)
    {
        vs_log_error(modName, "[filter] Failed!\n");
        vs_log_error(modName, "%s\n", exc.what());
        return VS_ERROR;
    }
    catch (...)
    {
        vs_log_error(modName, "[filter] Failed!\n");
        vs_log_error(modName, "Unknown failure type!\n");
        return VS_ERROR;
    }
    
    
    vs_log_info(modName,
                "[filter] Initialized: [%ix%i]:%s\n",
                aFi->width,
                aFi->height,
                _fmt2str(aFi->pFormat));
                
    return VS_OK;
}


void vsMotionDetectionCleanup(VSMotionDetect* aMd)
{
    VSMD*  md = &(VSMD2Inst(aMd));
    delete md;
    aMd->_inst = nullptr;
}


// returns true if match of local motion is better than threshold
bool __lm_match_better(const LocalMotion& lm, const double& tresh)
{
    return (lm.match <= tresh);
}


int vsMotionDetection(VSMotionDetect* aMd,
                      LocalMotions*   aMotions,
                      VSFrame*        aFrame)
{
    VSMD& md = VSMD2Inst(aMd);
    
    try
    {
        md(aMotions, *aFrame);
    }
    catch (std::exception& exc)
    {
        vs_log_error(md.conf.modName, "[filter] Failed!\n");
        vs_log_error(md.conf.modName, "%s\n", exc.what());
        return VS_ERROR;
    }
    
    return VS_OK;
}


int vsPrepareFile(const VSMotionDetect* aMd,
                  FILE*                 f)
{
    if (nullptr == f)
    {
        return VS_ERROR;
    }
    
    try
    {
        const VSMD& md = VSMD2Inst(aMd);
        
        fprintf(f, "VID.STAB 1\n");
        fprintf(f, "#       vidstab = vid.stab.cl built " __DATE__ " " __TIME__ "\n");
        fprintf(f, "#      accuracy = %d\n", md.conf.accuracy);
        fprintf(f, "#     shakiness = %d\n", md.conf.shakiness);
        fprintf(f, "#      stepsize = %d\n", md.conf.stepSize);
        fprintf(f, "#   mincontrast = %f\n", md.conf.contrastThreshold);
    }
    catch (std::exception& exc)
    {
        vs_log_error("vidstab", "[filter] Failed!\n");
        vs_log_error("vidstab", "%s\n", exc.what());
        return VS_ERROR;
    }
    catch (...)
    {
        vs_log_error("vidstab", "[filter] Failed!\n");
        vs_log_error("vidstab", "Unknown failure type!\n");
        return VS_ERROR;
    }
    
    return VS_OK;
}


int vsWriteToFile(const VSMotionDetect* aMd,
                  FILE*                 f,
                  const LocalMotions*   lms)
{
    if ((nullptr == f) || (nullptr == lms))
    {
        return VS_ERROR;
    }
    
    try
    {
        const VSMD& md = VSMD2Inst(aMd);
        
        if (fprintf(f, "Frame %i (", md.frameNum) <= 0)
        {
            return VS_ERROR;
        }
        
        if (vsStoreLocalmotions(f, lms) <= 0)
        {
            return VS_ERROR;
        }
        
        if (fprintf(f, ")\n") <= 0)
        {
            return VS_ERROR;
        }
    }
    catch (std::exception& exc)
    {
        vs_log_error("vidstabdetect", "[filter] Failed!\n");
        vs_log_error("vidstabdetect", "%s\n", exc.what());
        return VS_ERROR;
    }
    catch (...)
    {
        vs_log_error("vidstabdetect", "[filter] Failed!\n");
        vs_log_error("vidstabdetect", "Unknown failure type!\n");
        return VS_ERROR;
    }
    
    return VS_OK;
}


/**
   calculates Michelson-contrast in the given small part of the given image
   to be more compatible with the absolute difference formula this is scaled by 0.1

   \param I pointer to framebuffer
   \param field Field specifies position(center) and size of subimage
   \param width width of frame (linesize in pixels)
   \param height height of frame
   \param bytesPerPixel calc contrast for only for first channel
*/
double contrastSubImg(unsigned char* const I,
                      const Field*         field,
                      int                  width,
                      int                  height,
                      int                  bytesPerPixel)
{
    int            s2   = field->size / 2;
    unsigned char  mini = 255;
    unsigned char  maxi = 0;
    unsigned char* p    = I + ((field->x - s2) + (field->y - s2) * width) * bytesPerPixel;
    
    for (int j = 0; j < field->size; ++j)
    {
        for (int k = 0; k < field->size; ++k)
        {
            mini = (mini < *p) ? mini : *p;
            maxi = (maxi > *p) ? maxi : *p;
            p += bytesPerPixel;
        }
        
        p += (width - field->size) * bytesPerPixel;
    }
    
    return (maxi - mini) / (maxi + mini + 0.1); // +0.1 to prevent from division by 0
}




