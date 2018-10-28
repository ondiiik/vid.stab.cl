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
#define iToFp8(v)  ((v)<<8)
#define fToFp8(v)  ((int32_t)((v)*((float)0xFF)))
#define iToFp16(v) ((v)<<16)
#define fToFp16(v) ((int32_t)((v)*((double)0xFFFF)))
#define fp16To8(v) ((v)>>8)
#define fp24To8(v) ((v)>>16)

#define fp8ToI(v)  ((v)>>8)
#define fp16ToI(v) ((v)>>16)
#define fp8ToF(v)  ((v)/((double)(1<<8)))
#define fp16ToF(v) ((v)/((double)(1<<16)))

#define fp8_0_5 (1<<7)
#define fp8ToIRound(v) (((v) + fp8_0_5) >> 7)
#define fp16_0_5 (1<<15)
#define fp16ToIRound(v) (((v) + fp16_0_5) >> 16)




#include "transform.h"
#include "transform_internal.h"
#include "transformtype_operations.h"

#include "transformfixedpoint.h"

#include "common_exception.h"
#include "vs_transformation_barrel.h"

#include "sys_omp.h"
#include <math.h>
#include <libgen.h>
#include <string.h>


using namespace Gimbal;


namespace
{
    const char moduleName[] { "Transform" };
    
    
    /**
     * @brief   Convert motion transform instance to C++ representation
     * @param   aMd     Motion transform instance
     * @return  C++ representation of motion transform instance
     */
    inline VSTR& VSTR2Inst(VSTransformData* aTd)
    {
        if (nullptr != aTd)
        {
            VSTR* const td = (VSTR*)aTd->_inst;
            return *td;
        }
        else
        {
            throw Common::EXCEPTION("Transform data C structure is NULL!");
        }
    }
    
    
    inline int _pix(const uint8_t* aBuf,
                    int            aLs,
                    int            aX,
                    int            aY)
    {
        return aBuf[aLs * aY + aX];
    }
    
    
    inline int _pixel(const uint8_t* aBuf,
                      int            aLs,
                      int            aX,
                      int            aY,
                      int            aWidth,
                      int            aHeight,
                      uint8_t        aDefault)
    {
        if ((aX >= 0) && (aY >= 0) && (aX < aWidth) && (aY  < aHeight))
        {
            return _pix(aBuf, aLs, aX, aY);
        }
        else
        {
            return aDefault;
        }
    }
    
    
    inline int _pixClamp(int aVal)
    {
        if (0 > aVal)
        {
            return 0;
        }
        else if (255 < aVal)
        {
            return 255;
        }
        else
        {
            return aVal;
        }
    }
    
    
    void _ipNone(uint8_t*       rv,
                 float          x,
                 float          y,
                 const uint8_t* img,
                 int            img_linesize,
                 int            width,
                 int            height,
                 uint8_t        def)
    {
        int res = _pixel(img,
                         img_linesize,
                         x + 0.5,
                         y + 0.5,
                         width,
                         height,
                         def);
                         
        *rv = _pixClamp(res);
    }
    
    
    void _ipLinear(uint8_t*       rv,
                   float          x,
                   float          y,
                   const uint8_t* img,
                   int            img_linesize,
                   int            width,
                   int            height,
                   uint8_t        def)
    {
        int   x_f = int(x + 0.5);
        int   x_c = x_f + 1;
        int   y_n = int(y + 0.5);
        int   v1  = _pixel(img, img_linesize, x_c, y_n, width, height, def);
        int   v2  = _pixel(img, img_linesize, x_f, y_n, width, height, def);
        float s   = v1 * (x - x_f) + v2 * (x_c - x);
        
        *rv = _pixClamp(s + 0.5);
    }
    
    
    /** _ipBilinearBorder: bi-linear interpolation function that also works at the border.
        This is used by many other interpolation methods at and outsize the border, see interpolate */
    void _ipBilinearBorder(uint8_t*       rv,
                           float          x,
                           float          y,
                           const uint8_t* img,
                           int            img_linesize,
                           int            width,
                           int            height,
                           uint8_t        def)
    {
        int   x_f = int(x + 0.5);
        int   x_c = x_f + 1;
        int   y_f = int(y + 0.5);
        int   y_c = y_f + 1;
        int   v1 = _pixel(img, img_linesize, x_c, y_c, width, height, def);
        int   v2 = _pixel(img, img_linesize, x_c, y_f, width, height, def);
        int   v3 = _pixel(img, img_linesize, x_f, y_c, width, height, def);
        int   v4 = _pixel(img, img_linesize, x_f, y_f, width, height, def);
        float s  = (v1 * (x - x_f) + v3 * (x_c - x)) * (y - y_f) +
                   (v2 * (x - x_f) + v4 * (x_c - x)) * (y_c - y);
                   
        *rv = _pixClamp(int(s + 0.5));
    }
    
    
    /** _ipBilinear: bi-linear interpolation function, see interpolate */
    void _ipBilinear(uint8_t*       rv,
                     float          x,
                     float          y,
                     const uint8_t* img,
                     int            img_linesize,
                     int            width,
                     int            height,
                     uint8_t        def)
    {
        if ((x < 0            ) ||
            (x > (width - 2)  ) ||
            (y < 0            ) ||
            (y > (height - 2)))
        {
            _ipBilinearBorder(rv,
                              x,
                              y,
                              img,
                              img_linesize,
                              width,
                              height,
                              def);
        }
        else
        {
            int   x_f = myfloor(x);
            int   x_c = x_f + 1;
            int   y_f = myfloor(y);
            int   y_c = y_f + 1;
            int   v1 = _pix(img, img_linesize, x_c, y_c);
            int   v2 = _pix(img, img_linesize, x_c, y_f);
            int   v3 = _pix(img, img_linesize, x_f, y_c);
            int   v4 = _pix(img, img_linesize, x_f, y_f);
            float s  = (v1 * (x - x_f) + v3 * (x_c - x)) * (y - y_f) +
                       (v2 * (x - x_f) + v4 * (x_c - x)) * (y_c - y);
                       
            *rv = _pixClamp(s + 0.5);
        }
    }
    
    
    /** taken from http://en.wikipedia.org/wiki/Bicubic_interpolation for alpha=-0.5
        in matrix notation:
        a0-a3 are the neigthboring points where the target point is between a1 and a2
        t is the point of interpolation (position between a1 and a2) value between 0 and 1
        | 0, 2, 0, 0 |  |a0|
        |-1, 0, 1, 0 |  |a1|
        (1,t,t^2,t^3) | 2,-5, 4,-1 |  |a2|
        |-1, 3,-3, 1 |  |a3|
    */
    inline int bicub_kernel(float t, short a0, short a1, short a2, short a3)
    {
        return (2 * a1 + t * ((-a0 + a2) + t * ((2 * a0 - 5 * a1 + 4 * a2 - a3) + t * (-a0 + 3 * a1 - 3 * a2 + a3) )) ) / 2;
    }
    
    
    /** interpolateBiCub: bi-cubic interpolation function using 4x4 pixel, see interpolate */
    void _ipBicubic(uint8_t*       rv,
                    float          x,
                    float          y,
                    const uint8_t* img,
                    int            img_linesize,
                    int            width,
                    int            height,
                    uint8_t        def)
    {
        if ((x < 0            ) ||
            (x > (width - 2)  ) ||
            (y < 0            ) ||
            (y > (height - 2)))
        {
            _ipBilinearBorder(rv,
                              x,
                              y,
                              img,
                              img_linesize,
                              width,
                              height,
                              def);
        }
        else
        {
            int x_f = int(x + 0.5);
            int y_f = int(y + 0.5);
            
            float tx = x - x_f;
            
            int   v1 = bicub_kernel(tx,
                                    _pix(img, img_linesize, x_f - 1, y_f - 1),
                                    _pix(img, img_linesize, x_f,     y_f - 1),
                                    _pix(img, img_linesize, x_f + 1, y_f - 1),
                                    _pix(img, img_linesize, x_f + 2, y_f - 1));
                                    
            int   v2 = bicub_kernel(tx,
                                    _pix(img, img_linesize, x_f - 1, y_f),
                                    _pix(img, img_linesize, x_f,     y_f),
                                    _pix(img, img_linesize, x_f + 1, y_f),
                                    _pix(img, img_linesize, x_f + 2, y_f));
                                    
            int   v3 = bicub_kernel(tx,
                                    _pix(img, img_linesize, x_f - 1, y_f + 1),
                                    _pix(img, img_linesize, x_f,     y_f + 1),
                                    _pix(img, img_linesize, x_f + 1, y_f + 1),
                                    _pix(img, img_linesize, x_f + 2, y_f + 1));
                                    
            int   v4 = bicub_kernel(tx,
                                    _pix(img, img_linesize, x_f - 1, y_f + 2),
                                    _pix(img, img_linesize, x_f,     y_f + 2),
                                    _pix(img, img_linesize, x_f + 1, y_f + 2),
                                    _pix(img, img_linesize, x_f + 2, y_f + 2));
                                    
            int res = bicub_kernel(y - y_f, v1, v2, v3, v4);
            
            *rv = _pixClamp(res);
        }
    }
}


namespace Gimbal
{
    VSTR::VSTR(const char*              aModName,
               VSTransformData&         aTd)
        :
        fiSrc       { aTd.fiSrc         },
        fiDest      { aTd.fiDest        },
        src         { aTd.src           },
        destbuf     { aTd.destbuf       },
        dest        { aTd.dest          },
        srcMalloced { aTd.srcMalloced   },
        interpolate { aTd.interpolate   },
        conf        { aTd.conf          },
        initialized { aTd.initialized   },
#ifdef USE_OMP
        numThreads  { 1                 },
#endif
        isrc        { fiSrc             },
        idst        { fiDest            },
        fsrc        { aTd.src,     isrc },
        fdst        { aTd.dest,    idst },
        fdstB       { aTd.destbuf, idst },
        _lensTrn    {                   }
    {
        _initVsTransform();
    }
    
    
    VSTR::~VSTR()
    {
    
    }
    
    
    void VSTR::prepare(const VSFrame* aSrc,
                       VSFrame*       aDest)
    {
        // we first copy the frame to td->src and then overwrite the destination
        // with the transformed version
        dest = *aDest;
        
        Frame::Info        isrc { fiSrc  };
        const Frame::Frame fsrc { *aSrc, isrc };
        
        if ((aSrc == aDest) || srcMalloced) // in place operation: we have to copy the src first
        {
            Frame::Frame ftd  { src, isrc };
            
            if (ftd.empty())
            {
                ftd.alloc();
                srcMalloced = 1;
            }
            
            ftd = fsrc;
        }
        else // otherwise no copy needed
        {
            src = *aSrc;
        }
        
        
        if (conf.crop == VSKeepBorder)
        {
            Frame::Frame ftd { destbuf, isrc };
            
            if (ftd.empty())
            {
                // if we keep the borders, we need a second buffer to store
                //  the previous stabilized frame, so we use destbuf
                ftd.alloc();
                
                // if we keep borders, save first frame into the background buffer (destbuf)
                ftd = fsrc;
            }
        }
        else   // otherwise we directly operate on the destination
        {
            destbuf = *aDest;
        }
    }
    
    
    void VSTR::process(VSTransform& aT)
    {
        if (fiSrc.pFormat < PF_PACKED)
        {
            _transformPlanar(aT);
        }
        else
        {
            _transformPacked(aT);
        }
    }
    
    
    void VSTR::finish()
    {
        if (conf.crop == VSKeepBorder)
        {
            // we have to store our result to video buffer
            // note: destbuf stores stabilized frame to be the default for next frame
            fdst = fsrc;
        }
    }
    
    
    void VSTR::_initVsTransform()
    {
#ifdef USE_OMP
        numThreads = VS_MAX(omp_get_max_threads(), 1);
        vs_log_info(conf.modName, "[filter] Multithreading: use %i threads\n", numThreads);
#endif
        
        fsrc.forget();
        fdst.forget();
        fdstB.forget();
        
        srcMalloced = 0;
        
        if (conf.maxShift > fiDest.width / 2)
        {
            conf.maxShift = fiDest.width / 2;
        }
        if (conf.maxShift > fiDest.height / 2)
        {
            conf.maxShift = fiDest.height / 2;
        }
        
        conf.interpolType = VS_MAX(VS_MIN(conf.interpolType, VS_BiCubic), VS_Zero);
        
        // not yet implemented
        if (conf.camPathAlgo == VSOptimalL1)
        {
            conf.camPathAlgo = VSGaussian;
        }
        
        switch (conf.interpolType)
        {
            case VS_Zero:
                interpolate = _ipNone;
                break;
            case VS_Linear:
                interpolate = _ipLinear;
                break;
            case VS_BiLinear:
                interpolate = _ipBilinear;
                break;
            case VS_BiCubic:
                interpolate = _ipBicubic;
                break;
            default:
                interpolate = _ipBilinear;
        }
    }
    
    
    void VSTR::_transformPacked(VSTransform& aT)
    {
        uint8_t* D_1   { src.data[0]               };
        uint8_t* D_2   { destbuf.data[0]           };
        fp16     c_s_x { iToFp16(fiSrc.width  / 2) };
        fp16     c_s_y { iToFp16(fiSrc.height / 2) };
        int32_t  c_d_x { fiDest.width  / 2         };
        int32_t  c_d_y { fiDest.height / 2         };
        
        
        /* for each pixel in the destination image we calculate the source
         * coordinate and make an interpolation:
         *      p_d = c_d + M(p_s - c_s) + t
         * where p are the points, c the center coordinate,
         *  _s source and _d destination,
         *  t the translation, and M the rotation matrix
         *      p_s = M^{-1}(p_d - c_d - t) + c_s
         */
        float z      { float(1.0 - aT.zoom / 100.0) };
        fp16  zcos_a { fToFp16(z * cos(-aT.alpha))  }; // scaled cos
        fp16  zsin_a { fToFp16(z * sin(-aT.alpha))  }; // scaled sin
        fp16  c_tx   { c_s_x - fToFp16(aT.x)        };
        fp16  c_ty   { c_s_y - fToFp16(aT.y)        };
        int channels { fiSrc.bytesPerPixel          };
        
        /* All channels */
        OMP_ALIAS(td, this)
        OMP_ALIAS(db, &destbuf)
        OMP_ALIAS(sc, &src)
        OMP_PARALLEL_FOR(numThreads,
                         omp parallel for shared(td, db, sc),
                         (int32_t y = 0; y < fiDest.height; ++y))
        {
            int32_t y_d1 { y - c_d_y };
            
            for (int32_t x = 0; x < fiDest.width; ++x)
            {
                int32_t x_d1 { x - c_d_x                                 };
                fp16    x_s  { ( zcos_a * x_d1) + (zsin_a * y_d1) + c_tx };
                fp16    y_s  { (-zsin_a * x_d1) + (zcos_a * y_d1) + c_ty };
                
                for (int32_t k = 0; k < channels; ++k)   // iterate over colors
                {
                    uint8_t* dest { &D_2[x + y * destbuf.linesize[0] + k] };
                    interpolateN(dest,
                                 x_s,
                                 y_s,
                                 D_1,
                                 src.linesize[0],
                                 fiSrc.width,
                                 fiSrc.height,
                                 channels,
                                 k,
                                 conf.crop ? 16 : *dest);
                }
            }
        }
    }
    
    
    void VSTR::_transformPlanar(VSTransform& aTransform)
    {
        /*
         * Don't change anything if did not detected shakes
         */
        if ((aTransform.alpha == 0) && (aTransform.x == 0) && (aTransform.y == 0) && (aTransform.zoom == 0))
        {
            if (fsrc != fdst)
            {
                fdst = fsrc;
            }
            
            return;
        }
        
        
        /*
         * Deshake all planes
         */
        for (int plane = 0; plane < fiSrc.planes; plane++)
        {
            uint8_t*          dataSrc     { src.data[             plane ]    };
            uint8_t*          dataDst     { destbuf.data[         plane ]    };
            
            int               wsub        { isrc.subsampleWidth(  plane )    };
            int               hsub        { isrc.subsampleHeight( plane )    };
            
            Common::Vect<int> dimDst      { CHROMA_SIZE(fiDest.width,  wsub),
                                            CHROMA_SIZE(fiDest.height, hsub) };
            Common::Vect<int> dimSrc      { CHROMA_SIZE(fiSrc.width,   wsub),
                                            CHROMA_SIZE(fiSrc.height,  hsub) };
                                            
            uint8_t  black                { (plane == 0) ? uint8_t(0) : uint8_t(0x80) };
            
            Common::Vect<float> centerSrc { dimSrc / 2                       };
            Common::Vect<int>   centerDst { dimDst / 2                       };
            Common::Vect<float> rotA      { float( cos(-aTransform.alpha)),
                                            float(-sin(-aTransform.alpha))   };
            rotA                         *= float(1.0 - (aTransform.zoom / 100.0));
            Common::Vect<float> rotB      { -rotA.y, rotA.x                  };
            Common::Vect<int>   sub       { (wsub + 1), (hsub + 1)           };
            Common::Vect<float> tr        { aTransform.x, aTransform.y       };
            
            Common::Vect<float> centerTr  { centerSrc - tr.div(sub)          };
            
            
            /*
             * for each pixel in the destination image we calc the source
             * coordinate and make an interpolation:
             *      p_d = c_d + M(p_s - c_s) + t
             * where p are the points, c the center coordinate,
             *  _s source and _d destination,
             *  t the translation, and M the rotation and scaling matrix
             *      p_s = M^{-1}(p_d - c_d - t) + c_s
             */
            OMP_ALIAS(td, this)
            OMP_ALIAS(db, &destbuf)
            OMP_ALIAS(sc, &src)
            OMP_PARALLEL_FOR(numThreads,
                             omp parallel for shared(td, db, sc),
                             (int y = 0; y < dimDst.y; ++y))
            {
                /*
                 * swapping of the loops brought 15% performace gain
                 */
                for (int x = 0; x < dimDst.x; ++x)
                {
                    Transformation::Vect lin  {      };
                    Transformation::Vect lens { x, y };
                    
                    _lensTrn.from(lin, lens, wsub + 1);
                    
                    Common::Vect<int>   delta      { lin - centerDst };
                    Common::Vect<float> stabilized { rotA* delta.x + rotB* delta.y + centerTr };
                    
                    _lensTrn.to(lens, stabilized, wsub + 1);
                    
                    uint8_t* output { &dataDst[y * destbuf.linesize[plane] + x] };
                    
                    interpolate(output,
                                lens.x,
                                lens.y,
                                dataSrc,
                                src.linesize[plane],
                                dimSrc.x,
                                dimSrc.y,
                                conf.crop ? black : *output);
                }
            }
        }
    }
}






const char* interpol_type_names[5] = {"No (0)", "Linear (1)", "Bi-Linear (2)",
                                      "Bi-Cubic (3)"
                                     };

const char* getInterpolationTypeName(VSInterpolType type)
{
    if (type >= VS_Zero && type < VS_NBInterPolTypes)
    {
        return interpol_type_names[(int) type];
    }
    else
    {
        return "unknown";
    }
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

void vsTransformGetConfig(struct VSTransformConfig* conf, const struct VSTransformData* td)
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
    const char* modName = ((nullptr != aConf) ? aConf->modName : "vid.stab");
    
    if (nullptr == aTd)
    {
        vs_log_error(modName, "Transform data instance is NULL!\n");
        return VS_ERROR;
    }
    
    if (nullptr == aConf)
    {
        vs_log_error(modName, "Configuration structure is NULL!");
        return VS_ERROR;
    }
    
    if (nullptr == aFiSrc)
    {
        vs_log_error(modName, "Source frame info structure is NULL!");
        return VS_ERROR;
    }
    
    if (nullptr == aFiDst)
    {
        vs_log_error(modName, "Destination frame info structure is NULL!");
        return VS_ERROR;
    }
    
    
    aTd->fiSrc  = *aFiSrc;
    aTd->fiDest = *aFiDst;
    aTd->conf   = *aConf;
    
    
    try
    {
        VSTR* td   = new VSTR(modName, *aTd);
        aTd->_inst = td;
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
        VSTR*  td = &(VSTR2Inst(aTd));
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
    try
    {
        VSTR td = VSTR2Inst(aTd);
        td.prepare(aSrc, aDest);
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


int vsDoTransform(struct VSTransformData* aTd,
                  struct VSTransform      aT)
{
    try
    {
        VSTR td = VSTR2Inst(aTd);
        td.process(aT);
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
    try
    {
        VSTR td = VSTR2Inst(aTd);
        td.finish();
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

/*
 *  This is actually the core algorithm for canceling the jiggle in the
 *  movie. We have different implementations which are patched here.
 */
int cameraPathOptimization(struct VSTransformData* td, struct VSTransformations* trans)
{
    switch (td->conf.camPathAlgo)
    {
        case VSAvg:
            return cameraPathAvg(td, trans);
        case VSOptimalL1: // not yet implenented
        case VSGaussian:
            return cameraPathGaussian(td, trans);
//   case VSOptimalL1: return cameraPathOptimalL1(td,trans);
    }
    return VS_ERROR;
}

/*
 *  We perform a low-pass filter on the camera path.
 *  This supports slow camera movemen, but in a smooth fasion.
 *  Here we use gaussian filter (gaussian kernel) lowpass filter
 */
int cameraPathGaussian(struct VSTransformData* td, struct VSTransformations* trans)
{
    struct VSTransform* ts = trans->ts;
    if (trans->len < 1)
    {
        return VS_ERROR;
    }
    if (td->conf.verbose & VS_DEBUG)
    {
        vs_log_msg(td->conf.modName, "Preprocess transforms:");
    }
    
    /* relative to absolute (integrate transformations) */
    if (td->conf.relative)
    {
        struct VSTransform t = ts[0];
        for (int i = 1; i < trans->len; i++)
        {
            ts[i] = add_transforms(&ts[i], &t);
            t = ts[i];
        }
    }
    
    if (td->conf.smoothing > 0)
    {
        VSTransform* ts2 = (VSTransform*)vs_malloc(sizeof(struct VSTransform) * trans->len);
        memcpy(ts2, ts, sizeof(struct VSTransform) * trans->len);
        int s = td->conf.smoothing * 2 + 1;
        VSArray kernel = vs_array_new(s);
        // initialize gaussian kernel
        int mu        = td->conf.smoothing;
        double sigma2 = sqr(mu / 2.0);
        for (int i = 0; i <= mu; i++)
        {
            kernel.dat[i] = kernel.dat[s - i - 1] = exp(-sqr(i - mu) / sigma2);
        }
        // vs_array_print(kernel, stdout);
        
        for (int i = 0; i < trans->len; i++)
        {
            // make a convolution:
            double weightsum = 0;
            struct VSTransform avg = null_transform();
            for (int k = 0; k < s; k++)
            {
                int idx = i + k - mu;
                if (idx >= 0 && idx < trans->len)
                {
                    if (unlikely(0 && ts2[idx].extra == 1)) // deal with scene cuts or bad frames
                    {
                        if (k < mu) // in the past of our frame: ignore everthing before
                        {
                            avg = null_transform();
                            weightsum = 0;
                            continue;
                        }
                        else             //current frame or in future: stop here
                        {
                            if (k == mu)   //for current frame: ignore completely
                            {
                                weightsum = 0;
                            }
                            break;
                        }
                    }
                    weightsum += kernel.dat[k];
                    avg = add_transforms_(avg, mult_transform(&ts2[idx], kernel.dat[k]));
                }
            }
            if (weightsum > 0)
            {
                avg = mult_transform(&avg, 1.0 / weightsum);
                
                // high frequency must be transformed away
                ts[i] = sub_transforms(&ts[i], &avg);
            }
            if (td->conf.verbose & VS_DEBUG)
            {
                vs_log_msg(td->conf.modName,
                           " avg: %5lf, %5lf, %5lf extra: %i weightsum %5lf",
                           avg.x, avg.y, avg.alpha, ts[i].extra, weightsum
                          );
            }
        }
    }
    return VS_OK;
}

/*
 *  We perform a low-pass filter in terms of transformations.
 *  This supports slow camera movement (low frequency), but in a smooth fasion.
 *  Here a simple average based filter
 */
int cameraPathAvg(struct VSTransformData* td, struct VSTransformations* trans)
{
    struct VSTransform* ts = trans->ts;
    
    if (trans->len < 1)
    {
        return VS_ERROR;
    }
    if (td->conf.verbose & VS_DEBUG)
    {
        vs_log_msg(td->conf.modName, "Preprocess transforms:");
    }
    if (td->conf.smoothing > 0)
    {
        /* smoothing */
        VSTransform* ts2 = (VSTransform*)vs_malloc(sizeof(struct VSTransform) * trans->len);
        memcpy(ts2, ts, sizeof(struct VSTransform) * trans->len);
        
        /*  we will do a sliding average with minimal update
         *   \hat x_{n/2} = x_1+x_2 + .. + x_n
         *   \hat x_{n/2+1} = x_2+x_3 + .. + x_{n+1} = x_{n/2} - x_1 + x_{n+1}
         *   avg = \hat x / n
         */
        int s = td->conf.smoothing * 2 + 1;
        struct VSTransform null = null_transform();
        /* avg is the average over [-smoothing, smoothing] transforms
           around the current point */
        struct VSTransform avg;
        /* avg2 is a sliding average over the filtered signal! (only to past)
         *  with smoothing * 2 horizon to kill offsets */
        struct VSTransform avg2 = null_transform();
        double tau = 1.0 / (2 * s);
        /* initialise sliding sum with hypothetic sum centered around
         * -1st element. We have two choices:
         * a) assume the camera is not moving at the beginning
         * b) assume that the camera moves and we use the first transforms
         */
        struct VSTransform s_sum = null;
        for (int i = 0; i < td->conf.smoothing; i++)
        {
            s_sum = add_transforms(&s_sum, i < trans->len ? &ts2[i] : &null);
        }
        mult_transform(&s_sum, 2); // choice b (comment out for choice a)
        
        for (int i = 0; i < trans->len; i++)
        {
            struct VSTransform* ot = ((i - td->conf.smoothing - 1) < 0)
                                     ? &null : &ts2[(i - td->conf.smoothing - 1)];
            struct VSTransform* nt = ((i + td->conf.smoothing) >= trans->len)
                                     ? &null : &ts2[(i + td->conf.smoothing)];
            s_sum = sub_transforms(&s_sum, ot);
            s_sum = add_transforms(&s_sum, nt);
            
            avg = mult_transform(&s_sum, 1.0 / s);
            
            /* lowpass filter:
             * meaning high frequency must be transformed away
             */
            ts[i] = sub_transforms(&ts2[i], &avg);
            /* kill accumulating offset in the filtered signal*/
            avg2 = add_transforms_(mult_transform(&avg2, 1 - tau),
                                   mult_transform(&ts[i], tau));
            ts[i] = sub_transforms(&ts[i], &avg2);
            
            if (td->conf.verbose & VS_DEBUG)
            {
                vs_log_msg(td->conf.modName,
                           "s_sum: %5lf %5lf %5lf, ts: %5lf, %5lf, %5lf\n",
                           s_sum.x, s_sum.y, s_sum.alpha,
                           ts[i].x, ts[i].y, ts[i].alpha);
                vs_log_msg(td->conf.modName,
                           "  avg: %5lf, %5lf, %5lf avg2: %5lf, %5lf, %5lf",
                           avg.x, avg.y, avg.alpha,
                           avg2.x, avg2.y, avg2.alpha);
            }
        }
        vs_free(ts2);
    }
    /* relative to absolute */
    if (td->conf.relative)
    {
        struct VSTransform t = ts[0];
        for (int i = 1; i < trans->len; i++)
        {
            ts[i] = add_transforms(&ts[i], &t);
            t = ts[i];
        }
    }
    return VS_OK;
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
//    // works inplace on trans
//    if (cameraPathOptimization(td, trans) != VS_OK)
//    {
//        return VS_ERROR;
//    }
//    struct VSTransform* ts = trans->ts;
//    /*  invert? */
//    if (td->conf.invert)
//    {
//        for (int i = 0; i < trans->len; i++)
//        {
//            ts[i] = mult_transform(&ts[i], -1);
//        }
//    }
//
//    /* crop at maximal shift */
//    if (td->conf.maxShift != -1)
//    {
//        for (int i = 0; i < trans->len; i++)
//        {
//            ts[i].x     = VS_CLAMP(ts[i].x, -td->conf.maxShift, td->conf.maxShift);
//            ts[i].y     = VS_CLAMP(ts[i].y, -td->conf.maxShift, td->conf.maxShift);
//        }
//    }
//
//    if (td->conf.maxAngle != - 1.0)
//    {
//        for (int i = 0; i < trans->len; i++)
//        {
//            ts[i].alpha = VS_CLAMP(ts[i].alpha, -td->conf.maxAngle, td->conf.maxAngle);
//        }
//    }
//
//    /* Calc optimal zoom (1)
//     *  cheap algo is to only consider translations
//     *  uses cleaned max and min to eliminate 99% of transforms
//     */
//    if (td->conf.optZoom == 1 && trans->len > 1)
//    {
//        struct VSTransform min_t, max_t;
//        cleanmaxmin_xy_transform(ts, trans->len, 1, &min_t, &max_t);  // 99% of all transformations
//        // the zoom value only for x
//        double zx = 2 * VS_MAX(max_t.x, fabs(min_t.x)) / td->fiSrc.width;
//        // the zoom value only for y
//        double zy = 2 * VS_MAX(max_t.y, fabs(min_t.y)) / td->fiSrc.height;
//        td->conf.zoom += 100 * VS_MAX(zx, zy); // use maximum
//        td->conf.zoom = VS_CLAMP(td->conf.zoom, -60, 60);
//        vs_log_info(td->conf.modName, "Final zoom: %lf\n", td->conf.zoom);
//    }
//    /* Calc optimal zoom (2)
//     *  sliding average to zoom only as much as needed also using rotation angles
//     *  the baseline zoom is the mean required zoom + global zoom
//     *  in order to avoid too much zooming in and out
//     */
//    if (td->conf.optZoom == 2 && trans->len > 1)
//    {
//        double* zooms = (double*)vs_zalloc(sizeof(double) * trans->len);
//        int w = td->fiSrc.width;
//        int h = td->fiSrc.height;
//        double req;
//        double meanzoom;
//        for (int i = 0; i < trans->len; i++)
//        {
//            zooms[i] = transform_get_required_zoom(&ts[i], w, h);
//        }
//
//        double prezoom = 0.;
//        double postzoom = 0.;
//        if (td->conf.zoom > 0.)
//        {
//            prezoom = td->conf.zoom;
//        }
//        else if (td->conf.zoom < 0.)
//        {
//            postzoom = td->conf.zoom;
//        }
//
//        meanzoom = mean(zooms, trans->len) + prezoom; // add global zoom
//        // forward - propagation (to make the zooming smooth)
//        req = meanzoom;
//        for (int i = 0; i < trans->len; i++)
//        {
//            req = VS_MAX(req, zooms[i]);
//            ts[i].zoom = VS_MAX(ts[i].zoom, req);
//            req = VS_MAX(meanzoom, req - td->conf.zoomSpeed); // zoom-out each frame
//        }
//        // backward - propagation
//        req = meanzoom;
//        for (int i = trans->len - 1; i >= 0; i--)
//        {
//            req = VS_MAX(req, zooms[i]);
//            ts[i].zoom = VS_MAX(ts[i].zoom, req) + postzoom;
//            req = VS_MAX(meanzoom, req - td->conf.zoomSpeed);
//        }
//        vs_free(zooms);
//    }
//    else if (td->conf.zoom != 0)   /* apply global zoom */
//    {
//        for (int i = 0; i < trans->len; i++)
//        {
//            ts[i].zoom += td->conf.zoom;
//        }
//    }
    
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


