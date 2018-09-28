#pragma once
/*
 *  motiondetect.h
 *
 *  Copyright (C) Georg Martius - February 2011
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
#include <stddef.h>
#include <stdlib.h>

#include "transformtype.h"
#include "transformtype_operations.h"
#include "vidstabdefines.h"
#include "vsvector.h"
#include "frameinfo.h"


#ifdef __cplusplus
extern "C" {
#endif


/** returns the default config
 */
VSMotionDetectConfig vsMotionDetectGetDefaultConfig(const char* modName);

/// returns the frame info
const VSFrameInfo* vsMotionDetectGetFrameInfo(const VSMotionDetect* md);


#ifdef __cplusplus
}


/** structure for motion detection fields */
struct VSMotionDetectFields
{
    /* maximum number of pixels we expect the shift of subsequent frames */
    int maxShift;
    int stepSize;                 // stepsize for detection
    int fieldNum;                 // number of measurement fields
    int maxFields;                // maximum number of fields used (selected by contrast)
    double contrastThreshold;     // fields with lower contrast are discarded
    int fieldSize;                // size = min(md->width, md->height)/10;
    int fieldRows;                // number of rows
    Field* fields;                // measurement fields
    short useOffset;              // if true then the offset us used
    struct VSTransform offset;           // offset for detection (e.g. known from coarse scan)
    PreparedTransform pt;
};
#endif


/*
 * Local variables:
 *   c-file-style: "stroustrup"
 *   c-file-offsets: ((case-label . *) (statement-case-intro . *))
 *   indent-tabs-mode: nil
 *   c-basic-offset: 2 t
 * End:
 *
 * vim: expandtab shiftwidth=2:
 */
