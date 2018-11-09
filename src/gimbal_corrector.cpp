/*
 *  transform.cpp
 *
 *  Copyright (C) Georg Martius - June 2007 - 2011
 *   georg dot martius at web dot de
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
#include <iostream>




#include "gimbal_corrector.h"
#include "transform_internal.h"
#include "transformtype_operations.h"

#include "transformfixedpoint.h"

#include "common_exception.h"
#include "common_average.h"





#include "sys_omp.h"
#include <math.h>
#include <libgen.h>
#include <string.h>
#include "gimbal_barrel.h"




using namespace Gimbal;


namespace
{
    const char moduleName[] { "Transform" };
    
    
    /**
     * @brief   Convert motion transform instance to C++ representation
     * @param   aMd     Motion transform instance
     * @return  C++ representation of motion transform instance
     */
    inline Corrector& VSTR2Inst(VSTransformData* aTd)
    {
        if (nullptr != aTd)
        {
            Corrector* const td = (Corrector*)aTd->_inst;
            return *td;
        }
        else
        {
            throw Common::EXCEPTION("Transform data C structure is NULL!");
        }
    }
}






namespace Gimbal
{
    Corrector::Corrector()
        :
        _serialized { "/tmp/gimbal.gbl" },
        _corrector  {                   },
        _barrel     {                   },
        _debarr     {                   },
        _debarr2    {                   }
    {
        _serialized.load();
        _barrelInit();
        _preprocess();
    }
    
    
    Corrector::~Corrector()
    {
    
    }
    
    
    void Corrector::operator()()
    {
    
    }
    
    
    void Corrector::_debarrel(Common::Vect<float>&          aDst,
                              const Common::Vect<unsigned>& aSrc,
                              unsigned                      aRatio) const
    {
        auto& dim { _serialized.dim() };
        
        if (aRatio == 1)
        {
            aDst = _debarr[dim.x * aSrc.y + aSrc.x];
        }
        else
        {
            aDst = _debarr2[dim.x * aSrc.y / 2 + aSrc.x];
        }
    }
    
    
    void Corrector::_barrelInit()
    {
        auto dim { _serialized.dim() };
        
        std::cout << "Calculates de-barrel for " << dim << "...\n";
        
        for (unsigned y = 0, ey = dim.y; y < ey; ++y)
        {
            for (unsigned x = 0, ex = dim.x; x < ex; ++x)
            {
                Common::Vect<float> src { x, y };
                Common::Vect<float> dst;
                
                _barrel.from(dst, src, 1);
                _debarr.push_back(dst);
            }
        }
        
        dim /= 2;
        
        for (unsigned y = 0, ey = dim.y; y < ey; ++y)
        {
            for (unsigned x = 0, ex = dim.x; x < ex; ++x)
            {
                Common::Vect<float> src { x, y };
                Common::Vect<float> dst;
                
                _barrel.from(dst, src, 2);
                _debarr.push_back(dst);
            }
        }
    }
    
    
    void Corrector::_preprocess()
    {
        std::cout << "Pre-process " << _serialized.cells.size() << ":\n";
        _preprocessCalcMotions();
    }
    
    
    void Corrector::_preprocessCalcMotions()
    {
        _corrector.reserve(_serialized.cells.size());
        
        unsigned n { 1 };
        
        for (const auto& det : _serialized.cells)
        {
            std::cout << n % slowACnt << ";" << (n + slowACnt / 2U) % slowACnt << ";" << n % staticACnt << ";" << (n + staticACnt / 2U) % staticACnt << ";";
            
            CorrectorSet cs;
            
            unsigned i { 0 };
            
            for (auto& ci : cs.items)
            {
                Common::Average<Common::Vect<float> > avgOffset;
                Common::Average<float>                avgAngle;
                
                for (const auto& cell : det.list)
                {
                    const auto& dir = cell.direction[i];
                    
                    if (dir.isValid())
                    {
                    
                        Common::Vect<float>    dst1;
                        _debarrel(             dst1,  cell.position, 1);
                        Common::Vect<float>    dst2;
                        Common::Vect<unsigned> src2 { cell.position + dir.val };
                        _debarrel(             dst2,  src2, 1);
                        avgOffset +=           dst2 - dst1;
                        
                        auto p1 { Common::VectPolar<int>(cell.position) };
                        auto p2 { Common::VectPolar<int>(src2)          };
                        avgAngle += (p1.a - p2.a);
                    }
                }
                
                ci.ofs   = avgOffset();
                ci.angle = avgAngle();
                ci.cnt   = avgOffset.cnt();
                ++i;
                
                std::cout << ci.cnt << ";" << ci.ofs.x << ";" << ci.ofs.y << ";" << Common::rad2deg<float>(ci.angle, 10) << ";";
            }
            
            _corrector.push_back(cs);
            ++n;
            
            std::cout << std::endl;
        }
    }
}







const char* getInterpolationTypeName(VSInterpolType type)
{
    return "unused";
}


// default initialization: attention the ffmpeg filter cannot call it
struct VSTransformConfig vsTransformGetDefaultConfig(const char* modName)
{
    struct VSTransformConfig conf;
    /* Options */
    conf.maxShift           = -1;
    conf.maxAngle           = -1;
    conf.crop               = VSKeepBorder;
    conf.relative           = 1;
    conf.invert             = 0;
    conf.smoothing          = 15;
    conf.zoom               = 0;
    conf.optZoom            = 1;
    conf.zoomSpeed          = 0.25;
    conf.interpolType       = VS_BiLinear;
    conf.verbose            = 0;
    conf.modName            = modName;
    conf.simpleMotionCalculation = 0;
    conf.storeTransforms    = 0;
    conf.smoothZoom         = 0;
    conf.camPathAlgo        = VSOptimalL1;
    return conf;
}

void vsTransformGetConfig(struct     VSTransformConfig* conf,
                          const struct VSTransformData* td)
{
    if (td && conf)
    {
        *conf = td->conf;
    }
}

const VSFrameInfo* vsTransformGetSrcFrameInfo(const struct VSTransformData* td)
{
    return &td->fiSrc;
}

const VSFrameInfo* vsTransformGetDestFrameInfo(const struct VSTransformData* td)
{
    return &td->fiDest;
}


int vsTransformDataInit(VSTransformData*         aTd,
                        const VSTransformConfig* aConf,
                        const VSFrameInfo*       aFiSrc,
                        const VSFrameInfo*       aFiDst)
{
    try
    {
        Corrector*   td   = new Corrector();
        aTd->_inst = td;
    }
    catch (std::exception& exc)
    {
        vs_log_error("gimbal-corrector", "[filter] Failed!\n");
        vs_log_error("gimbal-corrector", "%s\n", exc.what());
        return VS_ERROR;
    }
    catch (...)
    {
        vs_log_error("gimbal-corrector", "[filter] Failed!\n");
        vs_log_error("gimbal-corrector", "Unknown failure type!\n");
        return VS_ERROR;
    }
    
    return VS_OK;
}


void vsTransformDataCleanup(struct VSTransformData* aTd)
{
    Frame::Frame fsrc { aTd->src, Frame::Info(aTd->fiSrc) };
    
    if (aTd->srcMalloced && !fsrc.empty())
    {
        fsrc.free();
    }
    
    Frame::Frame fdst { aTd->destbuf, Frame::Info(aTd->fiDest) };
    
    if ((aTd->conf.crop == VSKeepBorder) && !fdst.empty())
    {
        fdst.free();
    }
    
    
    try
    {
        Corrector*  td = &(VSTR2Inst(aTd));
        delete td;
        aTd->_inst = nullptr;
    }
    catch (std::exception& exc)
    {
        const char*  modName = ((nullptr != aTd) ? aTd->conf.modName : "vid.stab");
        vs_log_error(modName, "[filter] Failed!\n");
        vs_log_error(modName, "%s\n", exc.what());
        assert(false);
    }
    catch (...)
    {
        const char*  modName = ((nullptr != aTd) ? aTd->conf.modName : "vid.stab");
        vs_log_error(modName, "[filter] Failed!\n");
        vs_log_error(modName, "Unknown failure type!\n");
        assert(false);
    }
}


int vsTransformPrepare(struct VSTransformData* aTd,
                       const VSFrame*          aSrc,
                       VSFrame*                aDest)
{
    return VS_OK;
}


int vsDoTransform(struct VSTransformData* aTd,
                  struct VSTransform      aT)
{
    try
    {
        Corrector& td = VSTR2Inst(aTd);
        td();
    }
    catch (std::exception& exc)
    {
        const char*  modName = ((nullptr != aTd) ? aTd->conf.modName : "vid.stab");
        vs_log_error(modName, "[filter] Failed!\n");
        vs_log_error(modName, "%s\n", exc.what());
        return VS_ERROR;
    }
    catch (...)
    {
        const char*  modName = ((nullptr != aTd) ? aTd->conf.modName : "vid.stab");
        vs_log_error(modName, "[filter] Failed!\n");
        vs_log_error(modName, "Unknown failure type!\n");
        return VS_ERROR;
    }
    
    return VS_OK;
}


int vsTransformFinish(struct VSTransformData* aTd)
{
    return VS_OK;
}


struct VSTransform vsGetNextTransform(const struct VSTransformData* td, struct VSTransformations* trans)
{
    if (trans->len <= 0 )
    {
        return null_transform();
    }
    if (trans->current >= trans->len)
    {
        trans->current = trans->len;
        if (!trans->warned_end)
        {
            vs_log_warn(td->conf.modName, "not enough transforms found, use last transformation!\n");
        }
        trans->warned_end = 1;
    }
    else
    {
        trans->current++;
    }
    return trans->ts[trans->current - 1];
}

void vsTransformationsInit(struct VSTransformations* trans)
{
    trans->ts         = 0;
    trans->len        = 0;
    trans->current    = 0;
    trans->warned_end = 0;
}

void vsTransformationsCleanup(struct VSTransformations* trans)
{
    if (trans->ts)
    {
        vs_free(trans->ts);
        trans->ts = NULL;
    }
    trans->len = 0;
}


/**
 * vsPreprocessTransforms: camera path optimization, relative to absolute conversion,
 *  and cropping of too large transforms.
 *
 * Parameters:
 *            td: transform private data structure
 *         trans: list of transformations (changed)
 * Return value:
 *     1 for success and 0 for failure
 * Preconditions:
 *     None
 * Side effects:
 *     td->trans will be modified
 */
int vsPreprocessTransforms(struct VSTransformData*   td,
                           struct VSTransformations* trans)
{
    return VS_OK;
}


/**
 * vsLowPassTransforms: single step smoothing of transforms, using only the past.
 *  see also vsPreprocessTransforms. Here only relative transformations are
 *  considered (produced by motiondetection). Also cropping of too large transforms.
 *
 * Parameters:
 *            td: transform private data structure
 *           mem: memory for sliding average transformation
 *         trans: current transform (from previous to current frame)
 * Return value:
 *         new transformation for current frame
 * Preconditions:
 *     None
 */
struct VSTransform vsLowPassTransforms(struct VSTransformData* td, struct VSSlidingAvgTrans* mem,
                                       const struct VSTransform* trans)
{

    if (!mem->initialized)
    {
        // use the first transformation as the average camera movement
        mem->avg = *trans;
        mem->initialized = 1;
        mem->zoomavg = 0.0;
        mem->accum = null_transform();
        return mem->accum;
    }
    else
    {
        double s = 1.0 / (td->conf.smoothing + 1);
        double tau = 1.0 / (3.0 * (td->conf.smoothing + 1));
        if (td->conf.smoothing > 0)
        {
            // otherwise do the sliding window
            mem->avg = add_transforms_(mult_transform(&mem->avg, 1 - s),
                                       mult_transform(trans, s));
        }
        else
        {
            mem->avg = *trans;
        }
        
        /* lowpass filter:
         * meaning high frequency must be transformed away
         */
        struct VSTransform newtrans = sub_transforms(trans, &mem->avg);
        
        /* relative to absolute */
        if (td->conf.relative)
        {
            newtrans = add_transforms(&newtrans, &mem->accum);
            mem->accum = newtrans;
            if (td->conf.smoothing > 0)
            {
                // kill accumulating effects
                mem->accum = mult_transform(&mem->accum, 1.0 - tau);
            }
        }
        
        /* crop at maximal shift */
        if (td->conf.maxShift != -1)
        {
            newtrans.x     = VS_CLAMP(newtrans.x, -td->conf.maxShift, td->conf.maxShift);
            newtrans.y     = VS_CLAMP(newtrans.y, -td->conf.maxShift, td->conf.maxShift);
        }
        if (td->conf.maxAngle != - 1.0)
        {
            newtrans.alpha = VS_CLAMP(newtrans.alpha, -td->conf.maxAngle, td->conf.maxAngle);
        }
        
        /* Calc sliding optimal zoom
         *  cheap algo is to only consider translations and to sliding avg
         */
        if (td->conf.optZoom != 0 && td->conf.smoothing > 0)
        {
            // the zoom value only for x
            double zx = 2 * newtrans.x / td->fiSrc.width;
            // the zoom value only for y
            double zy = 2 * newtrans.y / td->fiSrc.height;
            double reqzoom = 100 * VS_MAX(fabs(zx), fabs(zy)); // maximum is requried zoom
            mem->zoomavg = (mem->zoomavg * (1 - s) + reqzoom * s);
            // since we only use past it is good to aniticipate
            //  and zoom a little in any case (so set td->zoom to 2 or so)
            newtrans.zoom = mem->zoomavg;
        }
        if (td->conf.zoom != 0)
        {
            newtrans.zoom += td->conf.zoom;
        }
        return newtrans;
    }
}
