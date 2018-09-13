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
#ifdef TESTING
#include "transformfloat.h"
#endif


#ifdef __cplusplus
extern "C" {
#endif


typedef struct VSTransformations
{
    struct VSTransform* ts;         // array of transformations
    int                 current;    // index to current transformation
    int                 len;        // length of trans array
    short               warned_end; // whether we warned that there is no transform left
}
VSTransformations;


typedef struct VSSlidingAvgTrans
{
    struct VSTransform avg;         // average transformation
    struct VSTransform accum;       // accumulator for relative to absolute conversion
    double      zoomavg;     // average zoom value
    short       initialized; // whether it was initialized or not
}
VSSlidingAvgTrans;


/// interpolation types
typedef enum VSInterpolType
{
    VS_Zero,
    VS_Linear,
    VS_BiLinear,
    VS_BiCubic,
    VS_NBInterPolTypes
}
VSInterpolType;


/// returns a name for the interpolation type
const char* getInterpolationTypeName(VSInterpolType type);


typedef enum VSBorderType
{
    VSKeepBorder,
    VSCropBorder
}
VSBorderType;


typedef enum VSCamPathAlgo
{
    VSOptimalL1,
    VSGaussian,
    VSAvg
}
VSCamPathAlgo;


/**
 * interpolate: general interpolation function pointer for one channel image data
 *              for fixed point numbers/calculations
 * Parameters:
 *             rv: destination pixel (call by reference)
 *            x,y: the source coordinates in the image img. Note this
 *                 are real-value coordinates (in fixed point format 24.8),
 *                 that's why we interpolate
 *            img: source image
 *   width,height: dimension of image
 *            def: default value if coordinates are out of range
 * Return value:  None
 */
typedef void (*vsInterpolateFun)(uint8_t*       rv,
                                 int32_t        x,
                                 int32_t        y,
                                 const uint8_t* img,
                                 int            linesize,
                                 int            width,
                                 int            height,
                                 uint8_t        def);
                                 
                                 
typedef struct VSTransformConfig
{
    /* whether to consider transforms as relative (to previous frame)
     * or absolute transforms
     */
    int                 relative;
    /* number of frames (forward and backward)
     * to use for smoothing transforms */
    int                 smoothing;
    enum VSBorderType   crop;        // 1: black bg, 0: keep border from last frame(s)
    int                 invert;      // 1: invert transforms, 0: nothing
    double              zoom;        // percentage to zoom: 0->no zooming 10:zoom in 10%
    int                 optZoom;     // 2: optimal adaptive zoom 1: optimal static zoom, 0: nothing
    double              zoomSpeed;   // for adaptive zoom: zoom per frame in percent
    enum VSInterpolType interpolType; // type of interpolation: 0->Zero,1->Lin,2->BiLin,3->Sqr
    int                 maxShift;    // maximum number of pixels we will shift
    double              maxAngle;    // maximum angle in rad
    const char*         modName;     // module name (used for logging)
    int                 verbose;     // level of logging
    // if 1 then the simple but fast method to termine the global motion is used
    int                 simpleMotionCalculation;
    int                 storeTransforms; // stores calculated transforms to file
    int                 smoothZoom;   // if 1 the zooming is also smoothed. Typically not recommended.
    enum VSCamPathAlgo  camPathAlgo;  // algorithm to use for camera path optimization
}
VSTransformConfig;


/**
 * @brief   Transformation data
 * @note    Type is used also in C implementation of ffmpeg filter,
 *          therefore it must be declared also as typedef.
 */
typedef struct VSTransformData
{
    VSFrameInfo              fiSrc;
    VSFrameInfo              fiDest;
    
    VSFrame                  src;         // copy of the current frame buffer
    VSFrame                  destbuf;     // pointer to an additional buffer or
    // to the destination buffer (depending on crop)
    VSFrame                  dest;          // pointer to the destination buffer
    
    short                    srcMalloced;   // 1 if the source buffer was internally malloced
    
    vsInterpolateFun         interpolate;   // pointer to interpolation function
#ifdef TESTING
    _FLT(vsInterpolateFun)   _FLT(interpolate);
#endif
    
    /* Options */
    struct VSTransformConfig conf;
    
    int initialized; // 1 if initialized and 2 if configured
}
VSTransformData;


/** returns the default config
 */
extern struct VSTransformConfig vsTransformGetDefaultConfig(const char* modName);

/** initialized the struct VSTransformData structure using the config and allocates memory
 *  for the frames and stuff
 *  @return VS_OK on success otherwise VS_ERROR
 */
extern int vsTransformDataInit(struct VSTransformData*         td,
                               const struct VSTransformConfig* conf,
                               const VSFrameInfo*              fi_src,
                               const VSFrameInfo*              fi_dest);
                               
                               
/** Deletes internal data structures.
 * In order to use the struct VSTransformData again, you have to call vsTransformDataInit
 */
extern void vsTransformDataCleanup(struct VSTransformData* td);

/// returns the current config
extern void vsTransformGetConfig(struct VSTransformConfig*     conf,
                                 const struct VSTransformData* td);
                                 
/// returns the frame info for the src
extern const VSFrameInfo* vsTransformGetSrcFrameInfo(const struct VSTransformData* td);
/// returns the frame info for the dest
extern const VSFrameInfo* vsTransformGetDestFrameInfo(const struct VSTransformData* td);


/**
 * @brief   Initializes transformations instance
 */
extern void vsTransformationsInit(struct VSTransformations* trans);


/// deletes struct VSTransformations internal memory
extern void vsTransformationsCleanup(struct VSTransformations* trans);

/// return next Transform and increases internal counter
extern struct VSTransform vsGetNextTransform(const struct VSTransformData* td,
                                             struct VSTransformations*     trans);
                                             
/** preprocesses the list of transforms all at once. Here the deshaking is calculated!
 */
extern int vsPreprocessTransforms(struct VSTransformData*   td,
                                  struct VSTransformations* trans);
                                  
/**
 * vsLowPassTransforms: single step smoothing of transforms, using only the past.
 *  see also vsPreprocessTransforms. */
extern struct VSTransform vsLowPassTransforms(struct VSTransformData*   td,
                                              struct VSSlidingAvgTrans* mem,
                                              const struct VSTransform*        trans);
                                              
/** call this function to prepare for a next transformation (transformPacked/transformPlanar)
    and supply the src frame buffer and the frame to write to. These can be the same pointer
    for an inplace operation (working on framebuffer directly)
 */
extern int vsTransformPrepare(struct VSTransformData* td,
                              const VSFrame*          src,
                              VSFrame*                dest);
                              
/// does the actual transformation
extern int vsDoTransform(struct VSTransformData* td,
                         struct VSTransform      t);
                         
                         
/** call this function to finish the transformation of a frame (transformPacked/transformPlanar)
 */
extern int vsTransformFinish(struct VSTransformData* td);


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
