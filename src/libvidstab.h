#pragma once
/*
 *  libvidstab.h
 *
 *  Created on: Feb 21, 2011
 *  Copyright (C) Georg Martius - June 2007
 *
 *  This file is part of transcode, a video stream processing tool
 *
 *  transcode is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  transcode is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with GNU Make; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */
#define LIBVIDSTAB_VERSION "v1.1 (2015-05-16)"


#include <stdio.h>
#include <stdint.h>


#ifdef __cplusplus
extern "C" {
#endif


/// pixel formats
typedef enum VSPixelFormat
{
    PF_NONE = -1,
    PF_GRAY8,     ///<        Y        ,  8bpp
    PF_YUV420P,   ///< planar YUV 4:2:0, 12bpp, (1 Cr & Cb sample per 2x2 Y samples)
    PF_YUV422P,   ///< planar YUV 4:2:2, 16bpp, (1 Cr & Cb sample per 2x1 Y samples)
    PF_YUV444P,   ///< planar YUV 4:4:4, 24bpp, (1 Cr & Cb sample per 1x1 Y samples)
    PF_YUV410P,   ///< planar YUV 4:1:0,  9bpp, (1 Cr & Cb sample per 4x4 Y samples)
    PF_YUV411P,   ///< planar YUV 4:1:1, 12bpp, (1 Cr & Cb sample per 4x1 Y samples)
    PF_YUV440P,   ///< planar YUV 4:4:0 (1 Cr & Cb sample per 1x2 Y samples)
    PF_YUVA420P,  ///< planar YUV 4:2:0, 20bpp, (1 Cr & Cb sample per 2x2 Y & A samples)
    PF_PACKED,    ///< dummy: packed formats start here
    PF_RGB24,     ///< packed RGB 8:8:8, 24bpp, RGBRGB...
    PF_BGR24,     ///< packed RGB 8:8:8, 24bpp, BGRBGR...
    PF_RGBA,      ///< packed RGBA 8:8:8:8, 32bpp, RGBARGBA...
    PF_NUMBER     ///< number of pixel formats
}
VSPixelFormat;


/** frame information for deshaking lib
    This only works for planar image formats
 */
typedef struct VSFrameInfo
{
    int           width;
    int           height;
    int           planes;        // number of planes (1 luma, 2,3 chroma, 4 alpha)
    int           log2ChromaW;   // subsampling of width in chroma planes
    int           log2ChromaH;   // subsampling of height in chroma planes
    VSPixelFormat pFormat;
    int           bytesPerPixel; // number of bytes per pixel (for packed formats)
}
VSFrameInfo;


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


/**
   A vector for arbitrary elements that resizes
*/
typedef struct _VSVector
{
    void** _data;
    int    _buffersize;
    int    _nelems;
}
_VSVector;


typedef _VSVector LocalMotions;
typedef _VSVector VSManyLocalMotions;


/** frame data according to frameinfo
 */
typedef struct VSFrame
{
    uint8_t* data[4];     // data in planes. For packed data everthing is in plane 0
    int      linesize[4]; // line size of each line in a the planes
}
VSFrame;


typedef struct VSTransformations
{
    struct VSTransform* ts;         // array of transformations
    int                 current;    // index to current transformation
    int                 len;        // length of trans array
    short               warned_end; // whether we warned that there is no transform left
}
VSTransformations;



/**
 * interpolate: general interpolation function pointer for one channel image data
 *
 * Parameters:
 *             rv: destination pixel (call by reference)
 *            x,y: the source coordinates in the image img. Note this
 *                 are real-value coordinates, that's why we interpolate
 *            img: source image
 *   img_linesize: length of one line in bytes (>= width)
 *   width,height: dimension of image
 *            def: default value if coordinates are out of range
 * Return value:  None
 */
typedef void (*vsInterpolateFun)(uint8_t*       rv,
                                 int            x,
                                 int            y,
                                 const uint8_t* img,
                                 int            img_linesize,
                                 int            width,
                                 int            height,
                                 uint8_t        def);
                                 
                                 
typedef enum VSCamPathAlgo
{
    VSOptimalL1,
    VSGaussian,
    VSAvg
}
VSCamPathAlgo;


typedef enum VSBorderType
{
    VSKeepBorder,
    VSCropBorder
}
VSBorderType;


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
    
    VSFrame                  src;           // copy of the current frame buffer
    VSFrame                  destbuf;       // pointer to an additional buffer or
    // to the destination buffer (depending on crop)
    VSFrame                  dest;          // pointer to the destination buffer
    
    short                    srcMalloced;   // 1 if the source buffer was internally malloced
    
    vsInterpolateFun         interpolate;   // pointer to interpolation function
    
    /* Options */
    struct VSTransformConfig conf;
    
    int                      initialized; // 1 if initialized and 2 if configured

    /**
     * @brief   Internal transform plugin instance
     */
    void* _inst;
}
VSTransformData;


/* structure to hold information about frame transformations
   x,y are translations, alpha is a rotation around the center in RAD,
   zoom is a percentage to zoom in and
   extra is for additional information like scene cut (unused)
 */
typedef struct VSTransform
{
    double x;
    double y;
    double alpha;
    double zoom;
    double barrel;
    double rshutter;
    int    extra;    /* -1: ignore transform (only internal use);
                     0 for normal trans; 1 for inter scene cut (unused) */
}
VSTransform;




typedef int (*vs_log_t) (int type, const char* tag, const char* format, ...);

extern vs_log_t vs_log;
extern int vs_log_level;

extern int VS_ERROR_TYPE;
extern int VS_WARN_TYPE;
extern int VS_INFO_TYPE;
extern int VS_MSG_TYPE;

extern int VS_ERROR;
extern int VS_OK;




/** initialized the VSMotionDetect structure and allocates memory
 *  for the frames and stuff
 *  @return VS_OK on success otherwise VS_ERROR
 */
extern int vsMotionDetectInit(VSMotionDetect*             md,
                              const VSMotionDetectConfig* conf,
                              const VSFrameInfo*          fi);
                              
                              
/**
 * @brief   Deletes internal data structures.
 * In order to use the VSMotionDetect again, you have to call vsMotionDetectInit
 */
extern void vsMotionDetectionCleanup(VSMotionDetect* md);


/**
 *  Performs a motion detection step
 *  Only the new current frame is given. The last frame
 *  is stored internally
 *  @param motions: calculated local motions. (must be deleted manually)
 * */
extern int vsMotionDetection(VSMotionDetect* md,
                             LocalMotions*   motions,
                             VSFrame*        frame);
                             
                             
/**
 * @brief    writes the header to the file that is to be holding the local motions
 */
extern int vsPrepareFile(const VSMotionDetect* td,
                         FILE*                 f);
                         
                         
/**
 * @brief   Returns the current config
 */
extern void vsMotionDetectGetConfig(VSMotionDetectConfig* conf,
                                    const VSMotionDetect* md);
                                    
                                    
/**
 * @brief   Initializes the frameinfo for the given format
 */
extern int vsFrameInfoInit(VSFrameInfo*  fi,
                           int           width,
                           int           height,
                           VSPixelFormat pFormat);
                           
                           
                           
/**
 * @brief   Appends the given localmotions to the file
 */
extern int vsWriteToFile(const VSMotionDetect* td,
                         FILE*                 f,
                         const LocalMotions*   lms);
                         
                         
/** Deletes internal data structures.
 * In order to use the struct VSTransformData again, you have to call vsTransformDataInit
 */
extern void vsTransformDataCleanup(struct VSTransformData* td);


/**
 * @brief   Deletes struct VSTransformations internal memory
 */
extern void vsTransformationsCleanup(struct VSTransformations* trans);


/** initialized the struct VSTransformData structure using the config and allocates memory
 *  for the frames and stuff
 *  @return VS_OK on success otherwise VS_ERROR
 */
extern int vsTransformDataInit(struct VSTransformData*         td,
                               const struct VSTransformConfig* conf,
                               const VSFrameInfo*              fi_src,
                               const VSFrameInfo*              fi_dest);
                               
                               
/**
 * @brief   Returns the current config
 */
extern void vsTransformGetConfig(struct VSTransformConfig*     conf,
                                 const struct VSTransformData* td);
                                 
                                 
/**
 * @brief   reads the entire file of localmotions, return VS_ERROR on error or if nothing is read
 *
 *  The format is as follows:
 *   The file must begin with 'VID.STAB version\n'
 *   Lines with # at the beginning are comments and will be ignored
 *   Data lines have the structure: Frame NUM (<LocalMotions>)
 *   where LocalMotions ::= List [(LM v.x v.y f.x f.y f.size contrast match),...]
 */
extern int vsReadLocalMotionsFile(FILE* f, VSManyLocalMotions* lms);


/**
 * @brief   converts for each frame the localmotions into a transform
 */
extern int vsLocalmotions2Transforms(struct VSTransformData*   td,
                                     const VSManyLocalMotions* motions,
                                     struct VSTransformations* trans);
                                     
                                     
/**
 * @brief   Read the transformations from the given file (Deprecated format)
 */
extern int vsReadOldTransforms(const struct VSTransformData* td,
                               FILE*                         f,
                               struct VSTransformations*     trans);
                               
                               
/**
 * @brief   Returns the frame info for the src
 */
extern const VSFrameInfo* vsTransformGetSrcFrameInfo(const struct VSTransformData* td);


/**
 * @brief   Preprocesses the list of transforms all at once.
 *
 * Here the deshaking is calculated!
 */
extern int vsPreprocessTransforms(struct VSTransformData*   td,
                                  struct VSTransformations* trans);
                                  
                                  
/**
 * @brief   Call this function to prepare for a next transformation
 *
 * (transformPacked/transformPlanar) and supply the src frame buffer and
 * the frame to write to. These can be the same pointer for an inplace
 * operation (working on framebuffer directly)
 */
extern int vsTransformPrepare(struct VSTransformData* td,
                              const VSFrame*          src,
                              VSFrame*                dest);
                              
                              
/**
 * @brief   Returns the frame info for the dest
 */
extern const VSFrameInfo* vsTransformGetDestFrameInfo(const struct VSTransformData* td);


/**
 * @brief   Does the actual transformation
 */
extern int vsDoTransform(struct VSTransformData* td,
                         struct VSTransform      t);


/**
 * @brief   Return next Transform and increases internal counter
 */
extern struct VSTransform vsGetNextTransform(const struct VSTransformData* td,
                                             struct VSTransformations*     trans);


/**
 * @brief   call this function to finish the transformation of a frame
 *
 * (transformPacked/transformPlanar)
 */
extern int vsTransformFinish(struct VSTransformData* td);


/**
 * @brief   like vs_vector_fini, but also deletes the data pointed by vector elements.
 *
 * Parameters:
 *     V: pointer to list to be finalized
 * Return Value:
 *     VS_OK on success,
 *     VS_ERROR on error.
 */
extern int vs_vector_del(_VSVector* V);


/**
 * @brief   Returns a name for the interpolation type
 */
extern const char* getInterpolationTypeName(VSInterpolType type);


typedef void* (*vs_malloc_t)(  size_t size            );
typedef void* (*vs_realloc_t)( void* ptr, size_t size );
typedef void  (*vs_free_t)(   void* ptr               );
typedef void* (*vs_zalloc_t)( size_t size             );


extern vs_malloc_t  vs_malloc;
extern vs_realloc_t vs_realloc;
extern vs_free_t    vs_free;
extern vs_zalloc_t  vs_zalloc;





#ifdef __cplusplus
}
#endif
