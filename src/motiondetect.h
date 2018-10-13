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
#include "transformtype.h"
#include "transformtype_operations.h"
#include "vidstabdefines.h"
#include "frameinfo.h"

#include "frame_canvas.h"
#include "cl/opencl.h"
#include <string>
#include <cassert>
#include <list>

#include "frame_pyramid.h"
#include "vs_vector.h"












/** returns the default config
 */
VSMotionDetectConfig vsMotionDetectGetDefaultConfig(const char* modName);

/// returns the frame info
const VSFrameInfo* vsMotionDetectGetFrameInfo(const VSMotionDetect* md);


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


namespace VidStab
{
    /**
     * @brief   Contrast index item
     */
    struct ContrastIdx
    {
        double contrast;    /**< @brief Contrast */
        int    index;       /**< @brief Index */
    };
    
    
    template <typename _PixT> struct Pyramids
    {
        typedef _PixT pix_t;
        
        
        Pyramids(const Common::Vect<unsigned>& aDim,
                 unsigned                      aMin)
            :
            fm
        {
            { aDim, aMin },
            { aDim, aMin },
            { aDim, aMin },
            { aDim, aMin },
            { aDim, aMin },
            { aDim, aMin }
        }
        {
        
        }
        
        
        Frame::Pyramid<_PixT> fm[6];
    };
    
    
    struct Cell
    {
        Common::Vect<unsigned> position;
        Common::Vect<unsigned> size;
        Common::Vect<int>      direction;
        unsigned               contrasQFactor;
    };
    
    
    /**
     * @brief   Data structure for motion detection part of deshaking
     */
    class VSMD
    {
    public:
        /* type for a function that calculates the transformation of a certain field
         */
        typedef LocalMotion (*calcFieldTransFunc)(VSMD&,
                                                  const VSMotionDetectFields&,
                                                  const Field&,
                                                  int);
                                                  
                                                  
        /* type for a function that calculates the contrast of a certain field
         */
        typedef double (*contrastSubImgFunc)(VSMD&,
                                             const Field&);
                                             
                                             
        /**
         * @brief   Construct data structure for motion detection part of deshaking
         *
         * @param   aModName    Module name
         * @param   aMd         Parrent C instance used by external tools such as ffmpeg
         * @param   aConf       Initial configuration
         * @param   aFi         Frame info
         */
        VSMD(const char*                 aModName,
             VSMotionDetect*             aMd,
             const VSMotionDetectConfig* aConf,
             const VSFrameInfo*          aFi);
             
             
        /**
         * @brief   Destroy VSDM
         */
        virtual ~VSMD();
        
        
        /**
         * @brief   process motion detection
         * @param   aMotions    Last frame motions
         * @param   aFrame      Current frame
         */
        void operator()(LocalMotions* aMotions,
                        VSFrame&      aFrame);
                        
                        
        /**
         * @brief   Reference to parent C instance
         */
        VSFrameInfo&         fiInfoC;
        
        VSMotionDetectConfig conf;
        
        VSMotionDetectFields fieldscoarse;
        VSMotionDetectFields fieldsfine;
        
        
        
    private:
        /**
         * @brief Current pre-processed frame storage
         */
        VSFrame _currPrepFrameC;
        
        /**
         * @brief Temporary buffer for blurring storage
         */
        VSFrame _currTmpFrameC;
        
        /**
         * @brief Frame buffer for last frame (copied) storage
         */
        VSFrame _prevFrameC;
        
        
    public:
        bool firstFrame;       // true if we have a valid previous frame
        
        int frameNum;
        
        
        
        
        /**
         * @brief   Processing frames info
         * @note    Wrapper around C interface structure
         *          @ref VidStab::VSMD::fiInfoC
         */
        Frame::Info fi;
        
        /**
         * @brief   Current frame
         */
        Frame::Frame curr;
        
        /**
         * @brief   Current preprocessed frame
         * @note    Wrapper around C interface structure
         *          @ref VidStab::VSMD::currPrepFrameC
         */
        Frame::Frame currPrep;
        
        /**
         * @brief   Temporary buffer used for bluring
         * @note    Wrapper around C interface structure
         *          @ref VidStab::VSMD::currTmpFrameC
         */
        Frame::Frame currTmp;
        
        /**
         * @brief   Copy of previous frame
         * @note    Wrapper around C interface structure
         *          @ref VidStab::VSMD::prevFrameC
         */
        Frame::Frame prev;
        
    private:
        /**
         * @brief   Blur mode
         */
        enum _BoxBlurColorMode
        {
            _BoxBlurColor,     /**< @brief Blur also color channels */
            _BoxBlurKeepColor, /**< @brief Copy original color channels */
            _BoxBlurNoColor    /**< @brief Do not touch color channels in dest */
        };
        
        
        /**
         * @brief   Show message with information about filter
         */
        void _initMsg();
        
        
        /**
         * @brief   Core filter initialization
         * @param   aConf   Initial configuration
         * @param   aFi     Frame info
         */
        void _initVsDetect(const VSMotionDetectConfig* aConf,
                           const VSFrameInfo*          aFi);
                           
                           
        /**
         * @brief   Initialize OpenCL engine
         */
        void _initOpenCl();
        
        
#if defined(USE_OPENCL)
        /**
         * @brief   Select OpenCL device
         */
        void _initOpenCl_selectDevice();
        
        
        /**
         * @brief   Prepare OpenCL kernels (calculator code)
         */
        void _initOpenCl_prepareKernels();
#endif /* defined(USE_OPENCL) */
        
        
        /**
         * @brief   Initialise measurement fields on the frame.
         *
         * The size of the fields and the maxshift is used to
         * calculate an optimal distribution in the frame.
         * if border is set then they are placed savely
         * away from the border for maxShift
        */
        void _initFields(VSMotionDetectFields& fs,
                         int                   fieldSize,
                         int                   maxShift,
                         int                   stepSize,
                         short                 border,
                         int                   spacing,
                         double                contrastThreshold);
                         
                         
        /* tries to register current frame onto previous frame.
         *   Algorithm:
         *   discards fields with low contrast
         *   select maxfields fields according to their contrast
         *   check theses fields for vertical and horizontal transformation
         *   use minimal difference of all possible positions
         */
        LocalMotions _calcTransFields(VSMotionDetectFields& fields,
                                      calcFieldTransFunc    fieldfunc,
                                      contrastSubImgFunc    contrastfunc);
                                      
                                      
        /**
         * @brief   Smoothen image to do better motion detection
         *
         * Larger step size or eventually gradient descent
         * (need higher resolution).
         *
         * @param   aFrame  Current frame
         */
        void _blur(const Frame::Frame& aFrame);
        
        
        /**
         * @brief   Performs a boxblur operation on src and stores results in dest
         *
         * It uses an accumulator method and separate horizontal and vertical runs
         *
         * @param   aSrc        Source buffer
         * @param   aBuffer     Frame buffer
         * @param   aFi         Frame information
         * @param   aStepSize   Size of bluring kernel (min 3 and it is made odd)
         * @param   aColormode  Which color mode is used
         *
         * @return  Blured frame
         */
        const Frame::Frame& _blurBox(const Frame::Frame& aSrc,
                                     unsigned int        aStepSize,
                                     _BoxBlurColorMode   aColormode);
                                     
                                     
        void _blurBoxHV(Frame::Plane&        dst,
                        Frame::Plane&        tmp,
                        const Frame::Plane&  src,
                        int                  size);
                        
                        
        void _blurBoxH(Frame::Plane&        dst,
                       const Frame::Plane&  src,
                       const int            size);
                       
                       
        void _blurBoxV(Frame::Plane&        dst,
                       const Frame::Plane&  src,
                       const int            size);
                       
                       
        /**
         * @brief       Detect motion
         *
         * @param[out]  aMotions    Motions vector
         * @param[in]   aFrame      Current frame
         */
        void _vs_detect(LocalMotions*       aMotions,
                        const Frame::Frame& aFrame);
                        
                        
        /**
         * @brief   Detect motion - contrast part
         * @param   aMotionscoarse    Coarse motion results
         * @return  Number of detected motions
         */
        int _detectContrast(LmList& aMotionscoarse);
        
        
        /**
         * @brief   Draw results of detection
         * @param   num_motions Number of motions
         */
        template <typename _PixTp> void _draw(Frame::Canvas<_PixTp>& aCanvas,
                                              int                    aNum_motions,
                                              const LocalMotions&    aMotionscoarse,
                                              const LocalMotions&    aMotionsfineC)
        {
            if (conf.show > 1)
            {
                for (int i = 0; i < aNum_motions; i++)
                {
                    _drawFieldScanArea(aCanvas, LMGet(&aMotionscoarse, i), fieldscoarse.maxShift);
                }
            }
            
            
            const LmList motionsfine { *const_cast<LocalMotions*>(&aMotionsfineC) };
            int num_motions_fine = motionsfine.size();
            
            for (int i = 0; i < aNum_motions; i++)
            {
                _drawField(aCanvas, LMGet(&aMotionscoarse, i), 1);
            }
            
            for (int i = 0; i < num_motions_fine; i++)
            {
                _drawField(aCanvas, LMGet(&aMotionsfineC, i), 0);
            }
            
            for (int i = 0; i < aNum_motions; i++)
            {
                _drawFieldTrans(aCanvas, LMGet(&aMotionscoarse, i), Frame::Pix<_PixTp>(180U, 0U));
            }
            
            for (int i = 0; i < num_motions_fine; i++)
            {
                _drawFieldTrans(aCanvas, LMGet(&aMotionsfineC, i), Frame::Pix<_PixTp>(64U, 0U));
            }
        }
        
        
        
        
        /** draws the field scanning area */
        template <typename _PixTp> void _drawFieldScanArea(Frame::Canvas<_PixTp>& aCanvas,
                                                           const LocalMotion*     aLm,
                                                           int                    aMaxShift)
        {
            _PixTp color { 80 };
            
            if (fiInfoC.pFormat <= PF_PACKED)
            {
                aCanvas.drawRectangle(aLm->f.x,
                                      aLm->f.y,
                                      aLm->f.size + 2 * aMaxShift,
                                      aLm->f.size + 2 * aMaxShift,
                                      color);
            }
        }
        
        
        /** draws the field */
        template <typename _PixTp> void _drawField(Frame::Canvas<_PixTp>& aCanvas,
                                                   const LocalMotion*     aLm,
                                                   int                    aBox)
        {
            _PixTp color { 40 };
            
            if (fiInfoC.pFormat <= PF_PACKED)
            {
                Vec size { aLm->f.size, aLm->f.size };
                
                if (aBox)
                {
                    aCanvas.drawBox(Vec(aLm->f), size, 40);
                }
                else
                {
                    aCanvas.drawRectangle(aLm->f.x,
                                          aLm->f.y,
                                          aLm->f.size,
                                          aLm->f.size,
                                          color);
                }
            }
        }
        
        
        
        
        /** draws the transform data of this field */
        template <typename _PixTp> void _drawFieldTrans(Frame::Canvas<_PixTp>& aCanvas,
                                                        const LocalMotion*     aLm,
                                                        _PixTp                 aColor)
        {
            if (fiInfoC.pFormat <= PF_PACKED)
            {
                Vec end  { Vec(aLm->f) + aLm->v };
                Vec size { 5, 5                 };
                
                _PixTp color1 { 0   };
                _PixTp color2 { 250 };
                aCanvas.drawBox( Vec(aLm->f),          size, color1); // draw center
                aCanvas.drawBox( Vec(aLm->f) + aLm->v, size, color2); // draw translation
                aCanvas.drawLine(Vec(aLm->f), end,     3,    aColor);
            }
        }
        
        
        
        
        /* select only the best 'maxfields' fields
           first calc contrasts then select from each part of the
           frame some fields
           We may simplify here by using random. People want high quality, so typically we use all.
        */
        void _selectfields(std::vector<ContrastIdx>& goodflds,
                           VSMotionDetectFields&     fs,
                           contrastSubImgFunc        contrastfunc);
                           
                           
        /**
         * @brief   Module name
         */
        std::string _mn;
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        /**
         * @brief   Process frame
         * @param   aPt     Pyramid type
         * @param   aFrame  Frame to be processed
         */
        template <typename _PixT> inline void _process(Pyramids<_PixT>& aPt,
                                                       VSFrame&         aFrame)
        {
            _next(     aPt, aFrame);
            _select(   aPt, aFrame);
            _detect(   aPt, aFrame);
            _visualize(aPt, aFrame);
        }
        
        
        /**
         * @brief   Read new frame pyramid including initialization
         * @param   aFrame  New frame
         */
        template <typename _PixT> inline void _next(Pyramids<_PixT>& aPt,
                                                    VSFrame&         aFrame)
        {
            if (0 == _idx)
            {
                _nextPiramid(aPt, aFrame);
            }
            
            _nextPiramid(aPt, aFrame);
        }
        
        
        /**
         * @brief   Read new frame pyramid
         * @param   aFrame  New frame
         */
        template <typename _PixT> void _nextPiramid(Pyramids<_PixT>& aPt,
                                                    VSFrame&         aFrame);
                                                    
                                                    
        /**
         * @brief   Find best places for detection cells
         */
        template <typename _PixT> void _select(Pyramids<_PixT>& aPt,
                                               VSFrame&         aFrame);
                                               
                                               
        /**
         * @brief   Correlate frames to detect movements
         */
        template <typename _PixT> void _detect(Pyramids<_PixT>& aPt,
                                               VSFrame&         aFrame);
                                               
                                               
        /**
         * @brief   Visualize detection
         */
        template <typename _PixT> void _visualize(Pyramids<_PixT>& aPt,
                                                  VSFrame&         aFrame);
                                                  
                                                  
        template <typename _PixT> unsigned _validate(const Frame::Canvas<_PixT>&  aCanvas,
                                                     const Common::Vect<unsigned> aPosition,
                                                     const Common::Vect<unsigned> aRect) const;
                                                     
                                                     
        /**
         * @brief   Calculates correlation of source and destination
         *
         * @param aCurrC    Current canvas
         * @param aPrevC    Previous canvas
         * @param aCurrV    Current rectangle position
         * @param aPrevV    Previous rectangle position
         * @param aRect     Rectangle size
         *
         * @return  Correlation result
         */
        template <typename _PixT> unsigned _corelate(const Frame::Canvas<_PixT>&  aCurrC,
                                                     const Frame::Canvas<_PixT>&  aPrevC,
                                                     const Common::Vect<unsigned> aCurrV,
                                                     const Common::Vect<unsigned> aPrevV,
                                                     const Common::Vect<unsigned> aRect) const;
                                                     
                                                     
        /**
         * @brief   RGB based piramids
         */
        Pyramids<Frame::PixRGB>* _piramidRGB;
        
        
        /**
         * @brief   YUV based piramids
         */
        Pyramids<Frame::PixYUV>* _piramidYUV;
        
        
        /**
         * @brief   Count of threads
         */
        unsigned _threadsCnt;


        /**
         * @brief   Block of cells
         */
        typedef std::vector<Cell> CellsBlock;
        
        
        /**
         * @brief   Movement per-CPU cells list
         */
        CellsBlock _cells;
        
        
        /**
         * @brief   Frame index
         */
        unsigned _idx;
        
        
        /**
         * @brief   Current frame pyramid index
         */
        unsigned _idxCurrent;
        
        
        /**
         * @brief   Previous frame pyramid index
         */
        unsigned _idxPrev;
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
#if defined(USE_OPENCL)
        /**
         * @brief   Sources of code we want to run over OpenCL
         */
        std::vector<cl::Program::Sources> _clSources;
        
        
    public:
        /**
         * @brief   OpenCL device
         */
        cl::Device _clDevice;
        
        
        /**
         * @brief   OpenCL context (communication channel to device)
         */
        cl::Context* _clContext;
        
        
        /**
         * @brief   Compiled binary of code we want to run over OpenCL
         */
        std::vector<cl::Program*> _clProgram;
        
        
        /**
         * @brief   Compiled binary of code we want to run over OpenCL
         */
        std::vector<const char*> _clProgramName;
        
        
#endif /* defined(USE_OPENCL) */
    };
    
    
    /**
     * @brief   Convert motion detect instance to C++ representation
     * @param   aMd     Motion detect instance
     * @return  C++ representation of motion detect instance
     */
    inline VSMD& VSMD2Inst(VSMotionDetect* aMd)
    {
        assert(nullptr != aMd);
        VSMD* const md = (VSMD*)aMd->_inst;
        return *md;
    }
    
    
    /* calculates the optimal transformation for one field in Packed
     * slower than the Planar version because it uses all three color channels
     */
    LocalMotion visitor_calcFieldTransPacked(VSMD&                       md,
                                             const VSMotionDetectFields& fs,
                                             const Field&                field,
                                             int                         fieldnum);
                                             
                                             
    /* calculates the optimal transformation for one field in Planar frames
     * (only luminance)
     */
    LocalMotion visitor_calcFieldTransPlanar(VSMD&                       md,
                                             const VSMotionDetectFields& fs,
                                             const Field&                field,
                                             int                         fieldnum);
                                             
                                             
    /**
       \see contrastSubImg_Michelson three times called with bytesPerPixel=3
       for all channels
    */
    double visitor_contrastSubImgPacked(VSMD&        md,
                                        const Field& field);
                                        
                                        
    /** \see contrastSubImg*/
    double visitor_contrastSubImgPlanar(VSMD&        md,
                                        const Field& field);
}
