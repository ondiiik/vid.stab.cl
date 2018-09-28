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


typedef struct VSMotionDetectConfig
{
    /* meta parameter for maxshift and fieldsize between 1 and 15 */
    int         shakiness;
    int         accuracy;         // meta parameter for number of fields between 1 and 10
    int         stepSize;         // stepsize of field transformation detection
    int         algo;             // deprecated
    int         virtualTripod;
    /* if 1 and 2 then the fields and transforms are shown in the frames */
    int         show;
    /* measurement fields with lower contrast are discarded */
    double      contrastThreshold;
    const char* modName;          // module name (used for logging)
    int         numThreads;       // number of threads to use (automatically set if 0)
}
VSMotionDetectConfig;


/** structure for motion detection fields */
typedef struct VSMotionDetectFields
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
}
VSMotionDetectFields;


/**
 * @brief   Data structure for motion detection part of deshaking
 */
typedef struct VSMotionDetect
{
    /**
     * @brief   Frame info
     * @note    This member is used because of compatibility with
     *          ffmpeg C interface
     */
    VSFrameInfo fi;


    /**
     * @brief   Pointer on C++ instance of motion tetect object
     */
    void* _inst;
}
VSMotionDetect;


/** returns the default config
 */
VSMotionDetectConfig vsMotionDetectGetDefaultConfig(const char* modName);

/** initialized the VSMotionDetect structure and allocates memory
 *  for the frames and stuff
 *  @return VS_OK on success otherwise VS_ERROR
 */
int vsMotionDetectInit(VSMotionDetect*             md,
                       const VSMotionDetectConfig* conf,
                       const VSFrameInfo*          fi);

/**
 *  Performs a motion detection step
 *  Only the new current frame is given. The last frame
 *  is stored internally
 *  @param motions: calculated local motions. (must be deleted manually)
 * */
int vsMotionDetection(VSMotionDetect* md,
                      LocalMotions*   motions,
                      VSFrame*        frame);

/** Deletes internal data structures.
 * In order to use the VSMotionDetect again, you have to call vsMotionDetectInit
 */
void vsMotionDetectionCleanup(VSMotionDetect* md);

/// returns the current config
void vsMotionDetectGetConfig(VSMotionDetectConfig* conf, const VSMotionDetect* md);

/// returns the frame info
const VSFrameInfo* vsMotionDetectGetFrameInfo(const VSMotionDetect* md);


#ifdef __cplusplus
}
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
