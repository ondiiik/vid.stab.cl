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


#include <math.h>
#include <libgen.h>
#include "transformtype.h"
#include "frameinfo.h"
#include "vidstabdefines.h"


#ifdef __cplusplus
extern "C" {
#endif


typedef struct VSSlidingAvgTrans
{
    struct VSTransform avg;         // average transformation
    struct VSTransform accum;       // accumulator for relative to absolute conversion
    double      zoomavg;     // average zoom value
    short       initialized; // whether it was initialized or not
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
                                              const struct VSTransform*        trans);


#ifdef __cplusplus
}
#endif


/*
 * Local variables:
 *   c-file-style: "stroustrup"
 *   c-file-offsets: ((case-label . *) (statement-case-intro . *))
 *   indent-tabs-mode: nil
 * End:
 *
 * vim: expandtab shiftwidth=4:
 */
