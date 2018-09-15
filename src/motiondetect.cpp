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

#include "boxblur.h"
#include "vidstabdefines.h"
#include "localmotion2transform.h"
#include "transformtype_operations.h"
#include "transformtype_operations.h"


#define USE_SPIRAL_FIELD_CALC


/*
 * C++ includes
 */
#include "motiondetect.hpp"
#include <exception>
#include <vector>
#include <algorithm>
#include <cstdio>
#include <cstdarg>

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
    /**
     * @brief   Vidstab exception
     */
    class _vsMdException: public exception
    {
    public:
        _vsMdException(const char* aFmt, ...)
        {
            va_list                            args;
            va_start(                          args, aFmt);
            vsnprintf(_errTxt, _bufSize, aFmt, args);
            va_end(                            args);
        }
        
        
        virtual const char* what() const throw()
        {
            return _errTxt;
        }
        
        
    private:
        static const std::size_t _bufSize { 1024 };
        char             _errTxt[_bufSize];
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
    
    
    /* compares contrast_idx structures respect to the contrast
       (for sort function)
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
        VSMD* md   = new VSMD(aMd, aConf, aFi);
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
        vs_log_error(md.conf.modName, "FAILURE: %s\n", exc.what());
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
 * draws a box at the given position x,y (center) in the given color
 (the same for all channels)
*/
void drawBox(unsigned char* I, int width, int height, int bytesPerPixel, int x,
             int y, int sizex, int sizey, unsigned char color)
{

    unsigned char* p = NULL;
    int j, k;
    p = I + ((x - sizex / 2) + (y - sizey / 2) * width) * bytesPerPixel;
    for (j = 0; j < sizey; j++)
    {
        for (k = 0; k < sizex * bytesPerPixel; k++)
        {
            *p = color;
            p++;
        }
        p += (width - sizex) * bytesPerPixel;
    }
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
 * draws a line from a to b with given thickness(not filled) at the given position x,y (center) in the given color
 at the first channel
*/
void drawLine(unsigned char* I, int width, int height, int bytesPerPixel,
              Vec* a, Vec* b, int thickness, unsigned char color)
{
    unsigned char* p;
    Vec div = sub_vec(*b, *a);
    if (div.y == 0) // horizontal line
    {
        if (div.x < 0)
        {
            *a = *b;
            div.x *= -1;
        }
        for (int r = -thickness / 2; r <= thickness / 2; r++)
        {
            p = I + ((a->x) + (a->y + r) * width) * bytesPerPixel;
            for (int k = 0; k <= div.x; k++)
            {
                *p = color;
                p += bytesPerPixel;
            }
        }
    }
    else
    {
        if (div.x == 0) // vertical line
        {
            if (div.y < 0)
            {
                *a = *b;
                div.y *= -1;
            }
            for (int r = -thickness / 2; r <= thickness / 2; r++)
            {
                p = I + ((a->x + r) + (a->y) * width) * bytesPerPixel;
                for (int k = 0; k <= div.y; k++)
                {
                    *p = color;
                    p += width * bytesPerPixel;
                }
            }
        }
        else
        {
            double m = (double)div.x / (double)div.y;
            int horlen = thickness + fabs(m);
            for ( int c = 0; c <= abs(div.y); c++)
            {
                int dy = div.y < 0 ? -c : c;
                int x = a->x + m * dy - horlen / 2;
                p = I + (x + (a->y + dy) * width) * bytesPerPixel;
                for ( int k = 0; k <= horlen; k++)
                {
                    *p = color;
                    p += bytesPerPixel;
                }
            }
        }
    }
}


// void addTrans(VSMotionDetect* md, struct VSTransform sl) {
//   if (!md->transs) {
//     md->transs = vs_list_new(0);
//   }
//   vs_list_append_dup(md->transs, &sl, sizeof(sl));
// }

// struct VSTransform getLastVSTransform(VSMotionDetect* md){
//   if (!md->transs || !md->transs->head) {
//     return null_transform();
//   }
//   return *((struct VSTransform*)md->transs->tail);
// }


//#ifdef TESTING
/// plain C implementation of compareSubImg (without ORC)
unsigned int compareSubImg_thr(unsigned char* const I1, unsigned char* const I2,
                               const Field* field, int width1, int width2, int height,
                               int bytesPerPixel, int d_x, int d_y,
                               unsigned int threshold)
{
    int k, j;
    unsigned char* p1 = NULL;
    unsigned char* p2 = NULL;
    int s2 = field->size / 2;
    unsigned int sum = 0;
    
    p1 = I1 + ((field->x - s2) + (field->y - s2) * width1) * bytesPerPixel;
    p2 = I2 + ((field->x - s2 + d_x) + (field->y - s2 + d_y) * width2)
         * bytesPerPixel;
    for (j = 0; j < field->size; j++)
    {
        for (k = 0; k < field->size * bytesPerPixel; k++)
        {
            sum += abs((int) * p1 - (int) * p2);
            p1++;
            p2++;
        }
        if ( sum > threshold) // no need to calculate any longer: worse than the best match
        {
            break;
        }
        p1 += (width1 - field->size) * bytesPerPixel;
        p2 += (width2 - field->size) * bytesPerPixel;
    }
    return sum;
}


namespace VidStab
{
    VSMD::VSMD(VSMotionDetect*             aMd,
               const VSMotionDetectConfig* aConf,
               const VSFrameInfo*          aFi)
        :
        fi         { aMd->fi },
        firstFrame { true    }
    {
        /*
         * First of all check inputs before we use them
         */
        if (nullptr == aConf)
        {
            throw _vsMdException("Configuration structure is NULL!");
        }
        
        if (nullptr == aFi)
        {
            throw _vsMdException("Frame info is NULL!");
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
            throw _vsMdException("Unsupported Pixel Format (%i)", fi.pFormat);
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
            throw _vsMdException("Allocation failed!");
        }
        
        vsFrameNull(&curr);
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
        
        vsFrameAllocate(&curr,    &fi);
        vsFrameAllocate(&currtmp, &fi);
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
        vsFrameFree(&curr);
        vsFrameFree(&currtmp);
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
            throw _vsMdException("Allocation failed!");
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
        LocalMotions localmotions;
        vs_vector_init(&localmotions, fields->maxFields);
        
        
        VSVector goodflds = _selectfields(fields, contrastfunc);
        
        OMP_SET_THREADS(conf.numThreads)
        
        for (int index = 0; index < vs_vector_size(&goodflds); index++)
        {
            int         i = ((contrast_idx*)vs_vector_get(&goodflds, index))->index;
            LocalMotion m = fieldfunc(this, fields, &fields->fields[i], i); // e.g. calcFieldTransPlanar
            
            if (m.match >= 0)
            {
                m.contrast = ((contrast_idx*)vs_vector_get(&goodflds, index))->contrast;
                OMP_CRITICAL
                vs_vector_append_dup(&localmotions, &m, sizeof(LocalMotion));
            }
        }
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
            vsFrameCopy(&prev, &curr, &fi);
        }
        
        frameNum++;
    }
    
    
    void VSMD::_blur(const VSFrame* aFrame)
    {
        if (fi.pFormat > PF_PACKED)
        {
            /*
             * We could calculate a gray-scale version and use
             * the PLANAR stuff afterwards so far smoothing is
             * not needed.
             */
            vsFrameCopy(&curr, aFrame, &fi);
        }
        else
        {
            /*
             * Box-kernel smoothing (plain average of pixels),
             * which is fine for us.
             */
            boxblurPlanar(&curr, aFrame, &currtmp, &fi, conf.stepSize, BoxBlurNoColor);
        }
    }
    
    
    void VSMD::_detect(LocalMotions*  aMotions,
                       const VSFrame* aFrame)
    {
        LocalMotions motionscoarse;
        LocalMotions motionsfine;
        
        vs_vector_init(&motionsfine, 0);
        
        if (fi.pFormat > PF_PACKED)
        {
            motionscoarse = _calcTransFields(&fieldscoarse, visitor_calcFieldTransPacked, visitor_contrastSubImgPacked);
        }
        else
        {
            motionscoarse = _calcTransFields(&fieldscoarse, visitor_calcFieldTransPlanar, visitor_contrastSubImgPlanar);
        }
        
        int num_motions = vs_vector_size(&motionscoarse);
        
        if (num_motions < 1)
        {
            vs_log_warn(conf.modName,
                        "too low contrast. (no translations are detected in frame %i)\n",
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
                motions2 = _calcTransFields(&fieldsfine, visitor_calcFieldTransPacked, visitor_contrastSubImgPacked);
            }
            else
            {
                motions2 = _calcTransFields(&fieldsfine, visitor_calcFieldTransPlanar, visitor_contrastSubImgPlanar);
            }
            
            /*
             * Through out those with bad match (worse than mean of coarse scan)
             */
            VSArray matchQualities1 = localmotionsGetMatch(&motionscoarse);
            double  meanMatch       = cleanmean(matchQualities1.dat, matchQualities1.len, NULL, NULL);
            motionsfine             = vs_vector_filter(&motions2, lm_match_better, &meanMatch);
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
            if (box)
            {
                drawBox(currorig.data[0], currorig.linesize[0], fi.height, 1,
                        lm->f.x, lm->f.y, lm->f.size, lm->f.size, /*lm->match >100 ? 100 :*/ 40);
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
            Vec end = add_vec(field_to_vec(lm->f), lm->v);
            drawBox(currorig.data[0], currorig.linesize[0], fi.height, 1,
                    lm->f.x, lm->f.y, 5, 5, 0); // draw center
            drawBox(currorig.data[0], currorig.linesize[0], fi.height, 1,
                    lm->f.x + lm->v.x, lm->f.y + lm->v.y, 5, 5, 250); // draw translation
            drawLine(currorig.data[0], currorig.linesize[0], fi.height, 1,
                     (Vec*)&lm->f, &end, 3, color);
        }
    }
    
    
    /* select only the best 'maxfields' fields
       first calc contrasts then select from each part of the
       frame some fields
       We may simplify here by using random. People want high quality, so typically we use all.
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
        int         tx = 0;
        int         ty = 0;
        
        uint8_t* I_c = md->curr.data[0], *I_p = md->prev.data[0];
        int width1 = md->curr.linesize[0] / 3; // linesize in pixels
        int width2 = md->prev.linesize[0] / 3; // linesize in pixels
        int i, j;
        int stepSize = fs->stepSize;
        int maxShift = fs->maxShift;
        
        Vec offset = { 0, 0};
        LocalMotion lm = null_localmotion();
        if (fs->useOffset)
        {
            PreparedTransform pt = prepare_transform(&fs->offset, &md->fi);
            offset = transform_vec(&pt, (Vec*)field);
            // is the field still in the frame
            if (unlikely(offset.x - maxShift - stepSize < 0 || offset.x + maxShift + stepSize >= md->fi.width ||
                         offset.y - maxShift - stepSize < 0 || offset.y + maxShift + stepSize >= md->fi.height))
            {
                lm.match = -1;
                return lm;
            }
        }
        
        /* Here we improve speed by checking first the most probable position
           then the search paths are most effectively cut. (0,0) is a simple start
        */
        unsigned int minerror = compareSubImg(I_c, I_p, field, width1, width2, md->fi.height,
                                              3, offset.x, offset.y, UINT_MAX);
        // check all positions...
        for (i = -maxShift; i <= maxShift; i += stepSize)
        {
            for (j = -maxShift; j <= maxShift; j += stepSize)
            {
                if ( i == 0 && j == 0 )
                {
                    continue;    //no need to check this since already done
                }
                unsigned int error = compareSubImg(I_c, I_p, field, width1, width2,
                                                   md->fi.height, 3, i + offset.x, j + offset.y, minerror);
                if (error < minerror)
                {
                    minerror = error;
                    tx = i;
                    ty = j;
                }
            }
        }
        if (stepSize > 1)   // make fine grain check around the best match
        {
            int txc = tx; // save the shifts
            int tyc = ty;
            int r = stepSize - 1;
            for (i = txc - r; i <= txc + r; i += 1)
            {
                for (j = tyc - r; j <= tyc + r; j += 1)
                {
                    if (i == txc && j == tyc)
                    {
                        continue;    //no need to check this since already done
                    }
                    unsigned int error = compareSubImg(I_c, I_p, field, width1, width2,
                                                       md->fi.height, 3, i + offset.x, j + offset.y, minerror);
                    if (error < minerror)
                    {
                        minerror = error;
                        tx = i;
                        ty = j;
                    }
                }
            }
        }
        
        if (fabs(tx) >= maxShift + stepSize - 1 || fabs(ty) >= maxShift + stepSize - 1)
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
    
    
    /* calculates the optimal transformation for one field in Planar frames
     * (only luminance)
     */
    LocalMotion visitor_calcFieldTransPlanar(VSMD*                 md,
                                             VSMotionDetectFields* fs,
                                             const Field*          field,
                                             int                   fieldnum)
    {
        int         tx = 0;
        int         ty = 0;
        
        uint8_t* Y_c = md->curr.data[0], *Y_p = md->prev.data[0];
        int linesize_c = md->curr.linesize[0], linesize_p = md->prev.linesize[0];
        // we only use the luminance part of the image
        int i, j;
        int stepSize = fs->stepSize;
        int maxShift = fs->maxShift;
        Vec offset = { 0, 0};
        LocalMotion lm = null_localmotion();
        if (fs->useOffset)
        {
            // Todo: we could put the preparedtransform into fs
            PreparedTransform pt = prepare_transform(&fs->offset, &md->fi);
            Vec fieldpos = {field->x, field->y};
            offset = sub_vec(transform_vec(&pt, &fieldpos), fieldpos);
            // is the field still in the frame
            int s2 = field->size / 2;
            if (unlikely(fieldpos.x + offset.x - s2 - maxShift - stepSize < 0 ||
                         fieldpos.x + offset.x + s2 + maxShift + stepSize >= md->fi.width ||
                         fieldpos.y + offset.y - s2 - maxShift - stepSize < 0 ||
                         fieldpos.y + offset.y + s2 + maxShift + stepSize >= md->fi.height))
            {
                lm.match = -1;
                return lm;
            }
        }
        
        
#ifdef USE_SPIRAL_FIELD_CALC
        unsigned int minerror = UINT_MAX;
        
        // check all positions by outgoing spiral
        i = 0;
        j = 0;
        int limit = 1;
        int step = 0;
        int dir = 0;
        while (j >= -maxShift && j <= maxShift && i >= -maxShift && i <= maxShift)
        {
            unsigned int error = compareSubImg(Y_c, Y_p, field, linesize_c, linesize_p,
                                               md->fi.height, 1, i + offset.x, j + offset.y,
                                               minerror);
                                               
            if (error < minerror)
            {
                minerror = error;
                tx = i;
                ty = j;
            }
            
            //spiral indexing...
            step++;
            switch (dir)
            {
                case 0:
                    i += stepSize;
                    if (step == limit)
                    {
                        dir = 1;
                        step = 0;
                    }
                    break;
                case 1:
                    j += stepSize;
                    if (step == limit)
                    {
                        dir = 2;
                        step = 0;
                        limit++;
                    }
                    break;
                case 2:
                    i -= stepSize;
                    if (step == limit)
                    {
                        dir = 3;
                        step = 0;
                    }
                    break;
                case 3:
                    j -= stepSize;
                    if (step == limit)
                    {
                        dir = 0;
                        step = 0;
                        limit++;
                    }
                    break;
            }
        }
#else
        /* Here we improve speed by checking first the most probable position
           then the search paths are most effectively cut. (0,0) is a simple start
        */
        unsigned int minerror = compareSubImg(Y_c, Y_p, field, linesize_c, linesize_p,
                                              md->fi.height, 1, 0, 0, UINT_MAX);
        // check all positions...
        for (i = -maxShift; i <= maxShift; i += stepSize)
        {
            for (j = -maxShift; j <= maxShift; j += stepSize)
            {
                if ( i == 0 && j == 0 )
                {
                    continue;    //no need to check this since already done
                }
                unsigned int error = compareSubImg(Y_c, Y_p, field, linesize_c, linesize_p,
                                                   md->fi.height, 1, i + offset.x, j + offset.y, minerror);
                if (error < minerror)
                {
                    minerror = error;
                    tx = i;
                    ty = j;
                }
            }
        }
        
#endif
        
        while (stepSize > 1) // make fine grain check around the best match
        {
            int txc = tx; // save the shifts
            int tyc = ty;
            int newStepSize = stepSize / 2;
            int r = stepSize - newStepSize;
            for (i = txc - r; i <= txc + r; i += newStepSize)
            {
                for (j = tyc - r; j <= tyc + r; j += newStepSize)
                {
                    if (i == txc && j == tyc)
                    {
                        continue;    //no need to check this since already done
                    }
                    unsigned int error = compareSubImg(Y_c, Y_p, field, linesize_c, linesize_p,
                                                       md->fi.height, 1, i + offset.x, j + offset.y, minerror);
                    if (error < minerror)
                    {
                        minerror = error;
                        tx = i;
                        ty = j;
                    }
                }
            }
            stepSize /= 2;
        }
        
        if (unlikely(fabs(tx) >= maxShift + stepSize - 1  ||
                     fabs(ty) >= maxShift + stepSize))
        {
            lm.match = -1.0; // to be kicked out
            return lm;
        }
        lm.f = *field;
        lm.v.x = tx + offset.x;
        lm.v.y = ty + offset.y;
        lm.match = ((double) minerror) / (field->size * field->size);
        return lm;
    }
    
    
    double visitor_contrastSubImgPacked(VSMD*        md,
                                        const Field* field)
    {
        unsigned char* const I         = md->curr.data[0];
        int                  linesize2 = md->curr.linesize[0] / 3; // linesize in pixels
        
        return (contrastSubImg(I + 0, field, linesize2, md->fi.height, 3) +
                contrastSubImg(I + 1, field, linesize2, md->fi.height, 3) +
                contrastSubImg(I + 2, field, linesize2, md->fi.height, 3)) / 3;
    }
    
    
    double visitor_contrastSubImgPlanar(VSMD*        md,
                                        const Field* field)
    {
#ifdef USE_SSE2
        return contrastSubImg1_SSE(md->curr.data[0], field, md->curr.linesize[0], md->fi.height);
#else
        return contrastSubImg(md->curr.data[0], field, md->curr.linesize[0], md->fi.height, 1);
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