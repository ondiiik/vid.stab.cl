#pragma once
/*
 *  transform.h
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


#include "transformtype.h"
#include "frameinfo.h"
#include "vidstabdefines.h"
#include "vs_exception.h"
#include <cmath>
#include <libgen.h>
#include <cassert>


typedef struct VSSlidingAvgTrans
{
    struct VSTransform avg;         // average transformation
    struct VSTransform accum;       // accumulator for relative to absolute conversion
    double             zoomavg;     // average zoom value
    short              initialized; // whether it was initialized or not
}
VSSlidingAvgTrans;


/** returns the default config
 */
extern struct VSTransformConfig vsTransformGetDefaultConfig(const char* modName);


/**
 * @brief   Initializes transformations instance
 */
extern void vsTransformationsInit(struct VSTransformations* trans);

/**
 * vsLowPassTransforms: single step smoothing of transforms, using only the past.
 *  see also vsPreprocessTransforms. */
extern struct VSTransform vsLowPassTransforms(struct VSTransformData*   td,
                                              struct VSSlidingAvgTrans* mem,
                                              const struct VSTransform* trans);


namespace VidStab
{
    class VSTR
    {
    public:
        /**
         * @brief   Construct data structure for motion detection part of transforming
         *
         * @param   aModName    Module name
         * @param   aTd         Parrent C instance used by external tools such as ffmpeg
         */
        VSTR(const char*              aModName,
             VSTransformData&         aTd);
             
             
             
        ~VSTR();
        
        
        /**
         * @brief   Prepare transformation frame
         * @param   src     Source frame
         * @param   dest    Destination frame
         */
        void prepare(const VSFrame* src,
                     VSFrame*       dest);
                     
                     
        /**
         * @brief   Process frame transform
         * @param   aT  REquested transformation
         */
        void process(VSTransform& aT);
        
        
        /**
         * @brief   Finish transformation step
         */
        void finish();
        
        
        const VSFrameInfo&       fiSrc;
        const VSFrameInfo&       fiDest;
        
        VSFrame&                 src;           // copy of the current frame buffer
        VSFrame&                 destbuf;       // pointer to an additional buffer or
        // to the destination buffer (depending on crop)
        VSFrame&                 dest;          // pointer to the destination buffer
        short&                   srcMalloced;   // 1 if the source buffer was internally malloced
        
        vsInterpolateFun&        interpolate;   // pointer to interpolation function
        
        /* Options */
        VSTransformConfig&       conf;
        
        int&                     initialized; // 1 if initialized and 2 if configured
        
        
        
        
#ifdef USE_OMP
        int                      numThreads;
#endif

        const Frame::Info        isrc;
        const Frame::Info        idst;
        
        Frame::Frame             fsrc;
        Frame::Frame             fdst;
        Frame::Frame             fdstB;
        
        
    private:
        void _initVsTransform();


        /**
         * @brief Does the actual transformation in Packed space
         *
         * @param   aT  Private data structure of this filter
         */
        void _transformPacked(VSTransform& aT);


        /**
         * @brief Does the actual transformation in Planar space
         *
         * applies current transformation to frame
         *
         * @param   aT  Private data structure of this filter
         */
        void _transformPlanar(VSTransform& aT);
    };
}
