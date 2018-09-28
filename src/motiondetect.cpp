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

#ifdef USE_OMP
#   include <omp.h>
#   define  OMP_CRITICAL _Pragma("omp critical(localmotions_append)")
#   define  OMP_SET_THREADS(_nt_) \
    omp_set_num_threads(    _nt_); \
    VSMD* md __attribute__((unused)) = this; \
    _Pragma("omp parallel for shared(goodflds, md, localmotions)")
#else
#   define  OMP_CRITICAL
#   define  OMP_SET_THREADS(_nt_)
#endif

#include "vidstabdefines.h"
#include "localmotion2transform.h"
#include "transformtype_operations.h"
#include "transformtype_operations.h"


#define USE_SPIRAL_FIELD_CALC


/*
 * C++ includes
 */
#include "motiondetect.hpp"
#include "md_exception.hpp"
#include "frame_canvas.h"

#include "dbg_profiler.h"

#include "cl/opencl.hpp"
#include "cl/opencl___blur_h.h"
#include "cl/opencl___blur_v.h"
#include "cl/opencl___correlate.h"

#include <vector>
#include <algorithm>
#include <cstdio>




using namespace VidStab;
using namespace std;


typedef struct contrast_idx
{
    double contrast;
    int    index;
}
contrast_idx;


namespace
{
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
    bool _cmp_Contrast(const contrast_idx& ci1,
                       const contrast_idx& ci2)
    {
        return ci1.contrast < ci2.contrast;
    }
}


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
    const VSMD& md = VSMD2Inst(aMd);
    assert(nullptr != aConf);
    *aConf = md.conf;
}


const VSFrameInfo* vsMotionDetectGetFrameInfo(const VSMotionDetect* aMd)
{
    const VSMD& md = VSMD2Inst(aMd);
    return     &md.fi;
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
    catch (exception& exc)
    {
        vs_log_error(modName, "FAILURE: %s\n", exc.what());
        return VS_ERROR;
    }
    
    
    vs_log_info(modName,
                "Initialized: [%ix%i]:%s\n",
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


int vsMotionDetection(VSMotionDetect* aMd,
                      LocalMotions*   aMotions,
                      VSFrame*        aFrame)
{
    VSMD& md = VSMD2Inst(aMd);
    
    try
    {
        md(aMotions, aFrame);
    }
    catch (exception& exc)
    {
        vs_log_error(md.conf.modName, "\tFAILURE: %s\n", exc.what());
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







/**
 * draws a rectangle (not filled) at the given position x,y (center) in the given color
 at the first channel
*/
void drawRectangle(unsigned char* I, int width, int height, int bytesPerPixel, int x,
                   int y, int sizex, int sizey, unsigned char color)
{

    unsigned char* p;
    int k;
    p = I + ((x - sizex / 2) + (y - sizey / 2) * width) * bytesPerPixel;
    for (k = 0; k < sizex; k++)
    {
        *p = color;    // upper line
        p += bytesPerPixel;
    }
    p = I + ((x - sizex / 2) + (y + sizey / 2) * width) * bytesPerPixel;
    for (k = 0; k < sizex; k++)
    {
        *p = color;    // lower line
        p += bytesPerPixel;
    }
    p = I + ((x - sizex / 2) + (y - sizey / 2) * width) * bytesPerPixel;
    for (k = 0; k < sizey; k++)
    {
        *p = color;    // left line
        p += width * bytesPerPixel;
    }
    p = I + ((x + sizex / 2) + (y - sizey / 2) * width) * bytesPerPixel;
    for (k = 0; k < sizey; k++)
    {
        *p = color;    // right line
        p += width * bytesPerPixel;
    }
}


/**
 * @brief    plain C implementation of compareSubImg (without ORC)
 */
unsigned int compareSubImg_thr(const uint8_t* aCurrent,
                               const uint8_t* aPrevious,
                               const Field*   aField,
                               int            aWidthCurrent,
                               int            aWidthPrevious,
                               int            aHeight,
                               int            aBpp,
                               int            aOffsetX,
                               int            aOffsetY,
                               unsigned int   aThreshold)
{
    unsigned s2 = aField->size / 2;
    unsigned x  = aField->x - s2;
    unsigned y  = aField->y - s2;

    unsigned int   sum  = 0;
    const uint8_t* curr = aCurrent  + (x            +  y             * aWidthCurrent)  * aBpp;
    const uint8_t* prev = aPrevious + (x + aOffsetX + (y + aOffsetY) * aWidthPrevious) * aBpp;
    
    for (int j = 0; j < aField->size; ++j)
    {
        for (int k = 0; k < aField->size * aBpp; k++)
        {
            sum += abs(int(*curr) - int(*prev));
            curr++;
            prev++;
        }
        
        if (sum > aThreshold)
        {
            /*
             * No need to calculate any longer: worse than the best match
             */
            break;
        }
        
        curr += (aWidthCurrent  - aField->size) * aBpp;
        prev += (aWidthPrevious - aField->size) * aBpp;
    }
    
    return sum;
}


namespace VidStab
{
    VSMD::VSMD(const char*                 aModName,
               VSMotionDetect*             aMd,
               const VSMotionDetectConfig* aConf,
               const VSFrameInfo*          aFi)
        :
        fi          { aMd->fi  },
        firstFrame  { true     },
        _mn         { aModName },
        _clDevice   {          },
        _clContext  { nullptr  },
        _clProgram  { nullptr  }
    {
        _initOpenCl();
        _initVsDetect(aConf, aFi);
    }
    
    
    VSMD::~VSMD()
    {
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
        
        vsFrameFree(&prev);
        vsFrameFree(&currPrep);
        vsFrameFree(&currtmp);
        
        
        delete _clProgram;
        delete _clContext;
    }
    
    
    void VSMD::_initOpenCl()
    {
        _initOpenCl_selectDevice();
        _initOpenCl_prepareKernels();
    }
    
    
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
            throw mdException("[OpenCL] There is no device available!");
        }
        
        _clContext = new cl::Context({_clDevice});
    }
    
    
    void VSMD::_initOpenCl_prepareKernels()
    {
        _clSources.push_back({opencl___blur_h,    opencl___blur_h_len});
        _clSources.push_back({opencl___blur_v,    opencl___blur_v_len});
        _clSources.push_back({opencl___correlate, opencl___correlate_len});
        
        _clProgram = new cl::Program(*_clContext, _clSources);
        
        
        if (_clProgram->build({_clDevice}) != CL_SUCCESS)
        {
            throw mdException("[OpenCL] OpenCL build error:\n%s\n", _clProgram->getBuildInfo<CL_PROGRAM_BUILD_LOG>(_clDevice).c_str());
        }
        
        vs_log_info(_mn.c_str(), "[OpenCL] Kernels built successfully\n");
    }
    
    
    void VSMD::_initVsDetect(const VSMotionDetectConfig* aConf,
                             const VSFrameInfo*          aFi)
    {
        /*
         * First of all check inputs before we use them
         */
        if (nullptr == aConf)
        {
            throw mdException("Configuration structure is NULL!");
        }
        
        if (nullptr == aFi)
        {
            throw mdException("Frame info is NULL!");
        }
        
        conf = *aConf;
        fi   = *aFi;
        
        
        /*
         * Is requested pixel format supported by us?
         */
        if ((fi.pFormat <= PF_NONE)   ||
            (fi.pFormat == PF_PACKED) ||
            (fi.pFormat >= PF_NUMBER))
        {
            throw mdException("Unsupported Pixel Format (%i)", fi.pFormat);
        }
        
        
#ifdef USE_OMP
        if (conf.numThreads == 0)
        {
            conf.numThreads = VS_MAX(omp_get_max_threads() - 2, 1);
        }
        vs_log_info(conf.modName, "Multithreading: use %i threads\n", conf.numThreads);
#endif
        
        
        vsFrameAllocate(&prev, &fi);
        if (vsFrameIsNull(&prev))
        {
            throw mdException("Allocation failed!");
        }
        
        vsFrameNull(&currPrep);
        vsFrameNull(&currorig);
        vsFrameNull(&currtmp);
        
        frameNum        = 0;
        
        conf.shakiness  = VS_MIN(10, VS_MAX(1, conf.shakiness));
        conf.accuracy   = VS_MIN(15, VS_MAX(1, conf.accuracy));
        
        if (conf.accuracy < conf.shakiness / 2)
        {
            vs_log_info(conf.modName, "Accuracy should not be lower than shakiness/2 -- fixed");
            conf.accuracy = conf.shakiness / 2;
        }
        
        if (conf.accuracy > 9 && conf.stepSize > 6)
        {
            vs_log_info(conf.modName, "For high accuracy use lower stepsize  -- set to 6 now");
            conf.stepSize = 6; // maybe 4
        }
        
        int minDimension  = VS_MIN(fi.width, fi.height);
        //  shift: shakiness 1: height/40; 10: height/4
        //  maxShift = VS_MAX(4,(minDimension*conf.shakiness)/40);
        //  size: shakiness 1: height/40; 10: height/6 (clipped)
        //  fieldSize = VS_MAX(4,VS_MIN(minDimension/6, (minDimension*conf.shakiness)/40));
        
        // fixed size and shift now
        int maxShift      = VS_MAX(16, minDimension / 7);
        int fieldSize     = VS_MAX(16, minDimension / 10);
        int fieldSizeFine = VS_MAX(6,  minDimension / 60);
        
#if defined(USE_SSE2) || defined(USE_SSE2_ASM)
        fieldSize     = (fieldSize     / 16 + 1) * 16;
        fieldSizeFine = (fieldSizeFine / 16 + 1) * 16;
#endif
        
        _initFields(&fieldscoarse, fieldSize,     maxShift,      conf.stepSize, 1, 0,             conf.contrastThreshold);
        _initFields(&fieldsfine,   fieldSizeFine, fieldSizeFine, 2,             1, fieldSizeFine, conf.contrastThreshold / 2);
        
        vsFrameAllocate(&currPrep, &fi);
        vsFrameAllocate(&currtmp,  &fi);
    }
    
    
    void VSMD::_initFields(VSMotionDetectFields* fs,
                           int                   size,
                           int                   maxShift,
                           int                   stepSize,
                           short                 keepBorder,
                           int                   spacing,
                           double                contrastThreshold)
    {
        fs->fieldSize         = size;
        fs->maxShift          = maxShift;
        fs->stepSize          = stepSize;
        fs->useOffset         = 0;
        fs->contrastThreshold = contrastThreshold;
        
        int rows = VS_MAX(3, (fi.height - fs->maxShift * 2) / (size + spacing) -1);
        int cols = VS_MAX(3, (fi.width  - fs->maxShift * 2) / (size + spacing) -1);
        
        // make sure that the remaining rows have the same length
        fs->fieldNum  = rows * cols;
        fs->fieldRows = rows;
        
        if (!(fs->fields = (Field*)vs_malloc(sizeof(Field) * fs->fieldNum)))
        {
            throw mdException("Allocation failed!");
        }
        else
        {
            int border = fs->stepSize;
            // the border is the amount by which the field centers
            // have to be away from the image boundary
            // (stepsize is added in case shift is increased through stepsize)
            if (keepBorder)
            {
                border = size / 2 + fs->maxShift + fs->stepSize;
            }
            int step_x = (fi.width  - 2 * border) / VS_MAX(cols - 1, 1);
            int step_y = (fi.height - 2 * border) / VS_MAX(rows - 1, 1);
            
            for (int j = 0; j < rows; j++)
            {
                for (int i = 0; i < cols; i++)
                {
                    int idx = j * cols + i;
                    
                    fs->fields[idx].x    = border + i * step_x;
                    fs->fields[idx].y    = border + j * step_y;
                    fs->fields[idx].size = size;
                }
            }
        }
        
        
        fs->maxFields = (conf.accuracy) * fs->fieldNum / 15;
        
        vs_log_info(conf.modName,
                    "Fieldsize: %i, Maximal translation: %i pixel\n",
                    fs->fieldSize,
                    fs->maxShift);
                    
        vs_log_info(conf.modName,
                    "Number of used measurement fields: %i out of %i\n",
                    fs->maxFields,
                    fs->fieldNum);
    }
    
    
    LocalMotions VSMD::_calcTransFields(VSMotionDetectFields* fields,
                                        calcFieldTransFunc    fieldfunc,
                                        contrastSubImgFunc    contrastfunc)
    {
        Dbg::Profiler::Data selectfieldsProf;
        Dbg::Profiler::Data fieldfuncProf;
        
        LocalMotions    localmotions;
        vs_vector_init(&localmotions, fields->maxFields);
        
        Dbg::Profiler::Measure prof1 { selectfieldsProf };
        VSVector goodflds = _selectfields(fields, contrastfunc);
        prof1.leave();
        
        
        OMP_SET_THREADS(conf.numThreads)
        
        for (int index = 0; index < vs_vector_size(&goodflds); index++)
        {
            int         i = ((contrast_idx*)vs_vector_get(&goodflds, index))->index;
            
            Dbg::Profiler::Measure prof2 { fieldfuncProf };
            LocalMotion m = fieldfunc(this, fields, &fields->fields[i], i); // e.g. calcFieldTransPlanar
            prof2.leave();
            
            if (m.match >= 0)
            {
                m.contrast = ((contrast_idx*)vs_vector_get(&goodflds, index))->contrast;
                OMP_CRITICAL
                vs_vector_append_dup(&localmotions, &m, sizeof(LocalMotion));
            }
        }
        vs_log_error(conf.modName, "\tPROF _selectfields=(%llu), fieldfunc=(%llu * %i)=(%llu) --> %2.2f x slower fieldfunc\n",
                     selectfieldsProf(),
                     fieldfuncProf(),
                     int(vs_vector_size(&goodflds)),
                     fieldfuncProf() * vs_vector_size(&goodflds),
                     (float(fieldfuncProf()) * vs_vector_size(&goodflds)) / float(selectfieldsProf()));
                     
        vs_vector_del(&goodflds);
        
        return localmotions;
    }
    
    
    void VSMD::operator ()(LocalMotions*  aMotions,
                           const VSFrame* aFrame)
    {
        _blur(aFrame);
        
        if (!firstFrame)
        {
            _detect(aMotions, aFrame);
        }
        else
        {
            vs_vector_init(aMotions, 1);
            firstFrame = false;
        }
        
        if ((conf.virtualTripod < 1) || (frameNum < conf.virtualTripod))
        {
            /*
             * For tripod we keep a certain reference frame
             */
            vsFrameCopy(&prev, curr, &fi);
        }
        
        frameNum++;
    }
    
    
    void VSMD::_blur(const VSFrame* aFrame)
    {
        int stepSize;
        
        if (fi.pFormat > PF_PACKED)
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
        
        curr = _blurBox(currPrep,
                        *aFrame,
                        currtmp,
                        fi,
                        stepSize,
                        _BoxBlurNoColor);
    }
    
    
    const VSFrame* VSMD::_blurBox(VSFrame&           aDst,
                                  const VSFrame&     aSrc,
                                  VSFrame&           aBuffer,
                                  const VSFrameInfo& aFi,
                                  unsigned int       aStepSize,
                                  _BoxBlurColorMode  aColormode)
    {
        if (aStepSize < 2)
        {
            return &aSrc;
        }
        
        
        VSFrame buf;
        int     localbuffer = 0;
        
        if (&aBuffer == nullptr)
        {
            vsFrameAllocate(&buf, &aFi);
            localbuffer = 1;
        }
        else
        {
            buf = aBuffer;
        }
        
        
        // odd and larger than 2 and maximally half of smaller image dimension
        aStepSize  = VS_CLAMP(aStepSize | 1U,
                              3,
                              unsigned(VS_MIN(aFi.height / 2, aFi.width / 2)));
                              
        _blurBoxHV(aDst.data[0],
                   buf.data[0],
                   aSrc.data[0],
                   aFi.width,
                   aFi.height,
                   buf.linesize[0],
                   aSrc.linesize[0],
                   aStepSize);
                   
                   
        int size2 = aStepSize / 2 + 1; // odd and larger than 0
        
        switch (aColormode)
        {
            case _BoxBlurColor:
                // color
                if (size2 > 1)
                {
                    for (int plane = 1; plane < aFi.planes; ++plane)
                    {
                        _blurBoxHV(aDst.data[plane],
                                   buf.data[plane],
                                   aSrc.data[plane],
                                   aFi.width  >> vsGetPlaneWidthSubS( &aFi, plane),
                                   aFi.height >> vsGetPlaneHeightSubS(&aFi, plane),
                                   buf.linesize[plane],
                                   aSrc.linesize[plane],
                                   size2);
                    }
                }
                break;
                
                
            case _BoxBlurKeepColor:
                // copy both color channels
                for (int plane = 1; plane < aFi.planes; plane++)
                {
                    vsFrameCopyPlane(&aDst, &aSrc, &aFi, plane);
                }
                break;
                
                
            default:
                break;
        }
        
        if (localbuffer)
        {
            vsFrameFree(&buf);
        }
        
        return &aDst;
    }
    
    
    void VSMD::_blurBoxHV(unsigned char*       aDst,
                          unsigned char*       aTmp,
                          const unsigned char* aSrc,
                          int                  aWidth,
                          int                  aHeight,
                          int                  aDstStrive,
                          int                  aSrcStrive,
                          int                  aSize)
    {
        /*
         * Prepare CL buffers
         */
        const std::size_t size = std::size_t(aWidth * aHeight * sizeof(aDst[0]));
        
        int args[]
        {
            aWidth,
            aHeight,
            aDstStrive,
            aSrcStrive,
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
        clQ.enqueueWriteBuffer(buffer_src,  CL_TRUE, 0, size,         aSrc);
        
        OpenCl::Kernel blurH(*_clProgram, "blurH");
        blurH.setArg(0, buffer_tmp);
        blurH.setArg(1, buffer_src);
        blurH.setArg(2, buffer_args);
        
        clQ.enqueueNDRangeKernel(blurH, cl::NullRange, cl::NDRange(aHeight));
        
        
        /*
         * Process vertical blur: tmp -> dst
         */
        OpenCl::Kernel blurV(*_clProgram, "blurV");
        blurV.setArg(0, buffer_dst);
        blurV.setArg(1, buffer_tmp);
        blurV.setArg(2, buffer_args);
        
        clQ.enqueueNDRangeKernel(blurV, cl::NullRange, cl::NDRange(aWidth));
        
        clQ.enqueueReadBuffer(buffer_dst, CL_TRUE, 0, size, aDst);
        
        
//        _blurBoxH(aTmp, aSrc, aWidth, aHeight, aDstStrive, aSrcStrive, aSize);
//        _blurBoxV(aDst, aTmp, aWidth, aHeight, aDstStrive, aSrcStrive, aSize);
    }
    
    
    void VSMD::_blurBoxH(unsigned char*       dst,
                         const unsigned char* src,
                         int                  width,
                         int                  height,
                         int                  dst_strive,
                         int                  src_strive,
                         int                  size)
    {
        const int size2 = size / 2; /* Size of one side of the kernel without center */
        
        for (int y = 0; y < height; ++y)
        {
            const unsigned char* inBegin = src + (y * src_strive);
            const unsigned char* inEnd   = inBegin + size2;
            unsigned int         acc     = (*inBegin) * (size2 + 1);
            
            /* Right half of kernel */
            for (const unsigned char* in = inBegin; in != inEnd; ++in)
            {
                acc += in[0];
            }
            
            /* Go through the image */
            unsigned char* out = dst + (y * dst_strive);
            
            for (int x = 0; x < width; ++x)
            {
                acc = acc + inEnd[0] - inBegin[0];
                
                if (x > size2)
                {
                    ++inBegin;
                }
                
                if (x < (width - size2 - 1))
                {
                    ++inEnd;
                }
                
                *out = acc / size;
                ++out;
            }
        }
    }
    
    
    void VSMD::_blurBoxV(unsigned char*       dst,
                         const unsigned char* src,
                         int                  width,
                         int                  height,
                         int                  dst_strive,
                         int                  src_strive,
                         int                  size)
    {
        const int size2 = size / 2; // size of one side of the kernel without center
        
        for (int x = 0; x < width; x++)
        {
            const unsigned char* start   = src + x;                // start and end of kernel
            const unsigned char* end     = start;
            unsigned char*       current = dst + x;               // current destination pixel
            int                  acc     = (*start) * (size2 + 1); // left half of kernel with first pixel
            
            // right half of kernel
            for (int k = 0; k < size2; k++)
            {
                acc += (*end);
                end += src_strive;
            }
            
            // go through the image
            for (int y = 0; y < height; y++)
            {
                acc = acc - (*start) + (*end);
                
                if (y > size2)
                {
                    start += src_strive;
                }
                
                if (y < height - size2 - 1)
                {
                    end += src_strive;
                }
                
                *current = acc / size;
                current += dst_strive;
            }
        }
    }
    
    
    void VSMD::_detect(LocalMotions*  aMotions,
                       const VSFrame* aFrame)
    {
        LocalMotions motionsfine;
        LocalMotions motionscoarse;
        int          num_motions = _detectContrast(motionscoarse);
        
        if (num_motions < 1)
        {
            vs_vector_init(&motionsfine, 0);
            vs_log_warn(conf.modName,
                        "Too low contrast. (no translations are detected in frame %i)\n",
                        frameNum);
        }
        else
        {
            /*
             * Calculates transformation and perform another scan with small fields
             */
            VSTransform        t = vsSimpleMotionsToTransform(fi, conf.modName, &motionscoarse);
            fieldsfine.offset    = t;
            fieldsfine.useOffset = 1;
            
            LocalMotions motions2;
            
            if (fi.pFormat > PF_PACKED)
            {
                motions2 = _calcTransFields(&fieldsfine,
                                            visitor_calcFieldTransPacked,
                                            visitor_contrastSubImgPacked);
            }
            else
            {
                motions2 = _calcTransFields(&fieldsfine,
                                            visitor_calcFieldTransPlanar,
                                            visitor_contrastSubImgPlanar);
            }
            
            /*
             * Through out those with bad match (worse than mean of coarse scan)
             */
            VSArray matchQualities1 = localmotionsGetMatch(&motionscoarse);
            double  meanMatch       = cleanmean(matchQualities1.dat, matchQualities1.len, NULL, NULL);
            
            motionsfine = vs_vector_filter(&motions2, lm_match_better, &meanMatch);
        }
        
        
        /*
         * Draw fields and transforms into frame.
         * Make a copy of current frame so we can modify it
         */
        if (conf.show)
        {
            currorig = *aFrame;
            _draw(num_motions, motionscoarse, motionsfine);
        }
        
        *aMotions = vs_vector_concat(&motionscoarse, &motionsfine);
    }
    
    
    int VSMD::_detectContrast(LocalMotions& aMotionscoarse)
    {
        vs_vector_init(&aMotionscoarse, 0);
        
        if (fi.pFormat > PF_PACKED)
        {
            aMotionscoarse = _calcTransFields(&fieldscoarse,
                                              visitor_calcFieldTransPacked,
                                              visitor_contrastSubImgPacked);
        }
        else
        {
            aMotionscoarse = _calcTransFields(&fieldscoarse,
                                              visitor_calcFieldTransPlanar,
                                              visitor_contrastSubImgPlanar);
        }
        
        return vs_vector_size(&aMotionscoarse);
    }
    
    
    void VSMD::_draw(int           num_motions,
                     LocalMotions& motionscoarse,
                     LocalMotions& motionsfine)
    {
        if (conf.show > 1)
        {
            for (int i = 0; i < num_motions; i++)
            {
                _drawFieldScanArea(LMGet(&motionscoarse, i), fieldscoarse.maxShift);
            }
        }
        
        
        int num_motions_fine = vs_vector_size(&motionsfine);
        
        for (int i = 0; i < num_motions; i++)
        {
            _drawField(LMGet(&motionscoarse, i), 1);
        }
        
        for (int i = 0; i < num_motions_fine; i++)
        {
            _drawField(LMGet(&motionsfine, i), 0);
        }
        
        for (int i = 0; i < num_motions; i++)
        {
            _drawFieldTrans(LMGet(&motionscoarse, i), 180);
        }
        
        for (int i = 0; i < num_motions_fine; i++)
        {
            _drawFieldTrans(LMGet(&motionsfine, i), 64);
        }
    }
    
    
    void VSMD::_drawFieldScanArea(const LocalMotion* lm,
                                  int                maxShift)
    {
        if (fi.pFormat <= PF_PACKED)
        {
            drawRectangle(currorig.data[0],
                          currorig.linesize[0],
                          fi.height,
                          1,
                          lm->f.x,
                          lm->f.y,
                          lm->f.size + 2 * maxShift,
                          lm->f.size + 2 * maxShift,
                          80);
        }
    }
    
    
    void VSMD::_drawField(const LocalMotion* lm,
                          short              box)
    {
        if (fi.pFormat <= PF_PACKED)
        {
            Frame::Canvas canvas { currorig.data[0], currorig.linesize[0], fi.height, 1};
            Vec           size   { lm->f.size, lm->f.size                              };
            
            if (box)
            {
                canvas.drawBox(Vec(lm->f), size, 40);
            }
            else
            {
                drawRectangle(currorig.data[0], currorig.linesize[0], fi.height, 1,
                              lm->f.x, lm->f.y, lm->f.size, lm->f.size, /*lm->match >100 ? 100 :*/ 40);
            }
        }
    }
    
    
    void VSMD::_drawFieldTrans(const LocalMotion* lm,
                               int                color)
    {
        if (fi.pFormat <= PF_PACKED)
        {
            Vec           end    { Vec(lm->f) + lm->v                                  };
            Vec           size   { 5, 5                                                };
            Frame::Canvas canvas { currorig.data[0], currorig.linesize[0], fi.height, 1};
            
            canvas.drawBox( Vec(lm->f),         size, 0);   // draw center
            canvas.drawBox( Vec(lm->f) + lm->v, size, 250); // draw translation
            canvas.drawLine(Vec(lm->f), end, 3, color);
        }
    }
    
    
    /*
     * Select only the best 'maxfields' fields
     *
     * First calculate contrasts then select from each part of the frame
     * some fields. We may simplify here by using random. People want high
     * quality, so typically we use all.
     */
    VSVector VSMD::_selectfields(VSMotionDetectFields* fs,
                                 contrastSubImgFunc    contrastfunc)
    {
        VSVector        goodflds;
        vs_vector_init(&goodflds, fs->fieldNum);
        
        
        /*
         * Calculate contrast for each field
         */
        vector<contrast_idx> ci { size_t(fs->fieldNum) };
        
        
        Field* f   = fs->fields;
        int    idx = 0;
        
        for (auto& i : ci)
        {
            i.contrast = contrastfunc(this, f);
            i.index    = idx;
            
            if (i.contrast < fs->contrastThreshold)
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
        vector<contrast_idx> ci_segms { ci.begin(), ci.end()         };
        int                  numsegms { fs->fieldRows + 1            };
        int                  segmlen  { fs->fieldNum  / numsegms + 1 };
        
        for (int i = 0; i < numsegms; i++)
        {
            int startindex = segmlen * i;
            int endindex   = segmlen * (i + 1);
            endindex       = (endindex > fs->fieldNum) ? fs->fieldNum : endindex;
            
            /*
             * Sort within segment
             */
            auto b = ci_segms.begin() + startindex;
            auto e = b +    (endindex - startindex);
            sort(b,  e, _cmp_Contrast);
            
            /*
             * Take maxfields / numsegms
             */
            for (int j = 0; j < (fs->maxFields / numsegms); j++)
            {
                if ((startindex + j) >= endindex)
                {
                    continue;
                }
                
                if (ci_segms[startindex + j].contrast > 0)
                {
                    vs_vector_append_dup(&goodflds,
                                         &ci[ci_segms[startindex + j].index],
                                         sizeof(contrast_idx));
                    /*
                     * Don't consider them in the later selection process
                     */
                    ci_segms[startindex + j].contrast = 0;
                }
            }
        }
        
        
        /*
         * Split the frame list into rows+1 segments.
         * Check whether enough fields are selected.
         */
        int remaining = fs->maxFields - vs_vector_size(&goodflds);
        
        if (remaining > 0)
        {
            /*
             * Take the remaining from the leftovers
             */
            auto b = ci_segms.begin();
            sort(b, b + fs->fieldNum, _cmp_Contrast);
            
            for (int j = 0; j < remaining; j++)
            {
                if (ci_segms[j].contrast > 0)
                {
                    vs_vector_append_dup(&goodflds, &ci_segms[j], sizeof(contrast_idx));
                }
            }
        }
        
        return goodflds;
    }
    
    
    /* calculates the optimal transformation for one field in Packed
     * slower than the Planar version because it uses all three color channels
     */
    LocalMotion visitor_calcFieldTransPacked(VSMD*                 md,
                                             VSMotionDetectFields* fs,
                                             const Field*          field,
                                             int                   fieldnum)
    {
        vs_log_error(md->conf.modName, "\t\t[DBG]PACKED %i\n", int(sizeof(Field)));
        
        int      tx       = 0;
        int      ty       = 0;
        
        uint8_t* current  = md->curr->data[0];
        uint8_t* previous = md->prev.data[0];
        int      width1   = md->curr->linesize[0] / 3; // linesize in pixels
        int      width2   = md->prev.linesize[0]  / 3; // linesize in pixels
        int      stepSize = fs->stepSize;
        int      maxShift = fs->maxShift;
        
        Vec offset              = { 0, 0};
        LocalMotion lm          = null_localmotion();
        
        if (fs->useOffset)
        {
            PreparedTransform pt = prepare_transform(&fs->offset, &md->fi);
            offset               = transform_vec(&pt, (Vec*)field);
            
            /*
             * Is the field still in the frame
             */
            if (((offset.x - maxShift - stepSize) < 0) || ((offset.x + maxShift + stepSize) >= md->fi.width) ||
                ((offset.y - maxShift - stepSize) < 0) || ((offset.y + maxShift + stepSize) >= md->fi.height))
            {
                lm.match = -1;
                return lm;
            }
        }
        
        /*
         * Here we improve speed by checking first the most probable position
         * then the search paths are most effectively cut. (0,0) is a simple start
         */
        unsigned int minerror = compareSubImg(current,
                                              previous,
                                              field,
                                              width1,
                                              width2,
                                              md->fi.height,
                                              3,
                                              offset.x,
                                              offset.y,
                                              UINT_MAX);
                                              
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
                
                unsigned int error = compareSubImg(current,
                                                   previous,
                                                   field,
                                                   width1,
                                                   width2,
                                                   md->fi.height,
                                                   3,
                                                   i + offset.x,
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
                    unsigned int error = compareSubImg(current,
                                                       previous,
                                                       field,
                                                       width1,
                                                       width2,
                                                       md->fi.height,
                                                       3,
                                                       i + offset.x,
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
        
        lm.f     = *field;
        lm.v.x   = tx + offset.x;
        lm.v.y   = ty + offset.y;
        lm.match = ((double)minerror) / (field->size * field->size);
        
        return lm;
    }
    
    
    /*
     * Calculates the optimal transformation for one field in Planar frames
     * (only luminance)
     */
    LocalMotion visitor_calcFieldTransPlanar(VSMD*                 md,
                                             VSMotionDetectFields* fs,
                                             const Field*          field,
                                             int                   fieldnum)
    {
        vs_log_error(md->conf.modName, "\t\t[DBG]PLANAR %i\n", int(sizeof(Field)));
        Vec t { };
        
        uint8_t* current    = md->curr->data[0];
        uint8_t* previous   = md->prev.data[0];
        int      linesize_c = md->curr->linesize[0];
        int      linesize_p = md->prev.linesize[0];
        
        /*
         * We only use the luminance part of the image
         */
        int         stepSize = fs->stepSize;
        int         maxShift = fs->maxShift;
        LocalMotion lm       = null_localmotion();
        Vec         offset { };
        
        if (fs->useOffset)
        {
            /*
             * Todo: we could put the preparedtransform into fs
             */
            PreparedTransform pt       = prepare_transform(&fs->offset, &md->fi);
            Vec               fieldpos = {field->x, field->y};
            
            offset = transform_vec(&pt, &fieldpos) - fieldpos;
            
            
            /*
             * Is the field still in the frame
             */
            int s2 = field->size / 2;
            
            if (unlikely((fieldpos.x + offset.x - s2 - maxShift - stepSize) < 0             ||
                         (fieldpos.x + offset.x + s2 + maxShift + stepSize) >= md->fi.width ||
                         (fieldpos.y + offset.y - s2 - maxShift - stepSize) < 0             ||
                         (fieldpos.y + offset.y + s2 + maxShift + stepSize) >= md->fi.height))
            {
                lm.match = -1;
                return lm;
            }
        }
        
        
#ifdef USE_SPIRAL_FIELD_CALC
        unsigned int minerror = UINT_MAX;
        
        for (_Spiral sp; sp < maxShift; sp += stepSize)
        {
            unsigned int error = compareSubImg(current,
                                               previous,
                                               field,
                                               linesize_c,
                                               linesize_p,
                                               md->fi.height,
                                               1,
                                               sp.i.x + offset.x,
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
        unsigned int minerror = compareSubImg(current,
                                              previous,
                                              field,
                                              linesize_c,
                                              linesize_p,
                                              md->fi.height,
                                              1,
                                              0,
                                              0,
                                              UINT_MAX);
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
        
                unsigned int error = compareSubImg(current,
                                                   previous,
                                                   field,
                                                   linesize_c,
                                                   linesize_p,
                                                   md->fi.height,
                                                   1,
                                                   i + offset.x,
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
                    
                    unsigned int error = compareSubImg(current,
                                                       previous,
                                                       field,
                                                       linesize_c,
                                                       linesize_p,
                                                       md->fi.height,
                                                       1,
                                                       i + offset.x,
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
        
        lm.f     = *field;
        lm.v.x   = t.x + offset.x;
        lm.v.y   = t.y + offset.y;
        lm.match = ((double) minerror) / (field->size * field->size);
        
        return lm;
    }
    
    
    double visitor_contrastSubImgPacked(VSMD*        md,
                                        const Field* field)
    {
        unsigned char* const I         = md->curr->data[0];
        int                  linesize2 = md->curr->linesize[0] / 3; // linesize in pixels
        
        return (contrastSubImg(I + 0, field, linesize2, md->fi.height, 3) +
                contrastSubImg(I + 1, field, linesize2, md->fi.height, 3) +
                contrastSubImg(I + 2, field, linesize2, md->fi.height, 3)) / 3;
    }
    
    
    double visitor_contrastSubImgPlanar(VSMD*        md,
                                        const Field* field)
    {
#ifdef USE_SSE2
        return contrastSubImg1_SSE(md->curr->data[0], field, md->curr->linesize[0], md->fi.height);
#else
        return contrastSubImg(md->curr->data[0], field, md->curr->linesize[0], md->fi.height, 1);
#endif
    }
}


/*
 * Local variables:
 *   c-file-style: "stroustrup"
 *   c-file-offsets: ((case-label . *) (statement-case-intro . *))
 *   indent-tabs-mode: nil
 *   tab-width:  2
 *   c-basic-offset: 2 t
 * End:
 *
 */
