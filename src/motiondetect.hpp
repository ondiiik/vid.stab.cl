#pragma once
/*
 * motiondetect.hpp
 *
 *  Created on: 15. 9. 2018
 *      Author: ondiiik
 */


#include "motiondetect.h"
#include "cl/opencl.hpp"
#include <string>


namespace VidStab
{
    /**
     * @brief   Data structure for motion detection part of deshaking
     */
    class VSMD
    {
    public:
        /* type for a function that calculates the transformation of a certain field
         */
        typedef LocalMotion (*calcFieldTransFunc)(VSMD*,
                                                  VSMotionDetectFields*,
                                                  const Field*,
                                                  int);
                                                  
                                                  
        /* type for a function that calculates the contrast of a certain field
         */
        typedef double (*contrastSubImgFunc)(VSMD*,
                                             const Field*);
                                             
                                             
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
        void operator()(LocalMotions*  aMotions,
                        const VSFrame* aFrame);
                        
                        
        /**
         * @brief   Reference to parent C instance
         */
        VSFrameInfo&         fi;
        
        VSMotionDetectConfig conf;
        
        VSMotionDetectFields fieldscoarse;
        VSMotionDetectFields fieldsfine;
        
        const VSFrame*       curr;       // Current pre-processed frame
        VSFrame              currPrep;   // Current pre-processed frame
        VSFrame              currorig;   // current frame buffer (original) (only pointer)
        VSFrame              currtmp;    // temporary buffer for blurring
        VSFrame              prev;       // frame buffer for last frame (copied)
        bool                 firstFrame; // true if we have a valid previous frame
        
        int                  frameNum;
        
        
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
        
        
        /**
         * @brief   Select OpenCL device
         */
        void _initOpenCl_selectDevice();
        
        
        /**
         * @brief   Prepare OpenCL kernels (calculator code)
         */
        void _initOpenCl_prepareKernels();
        
        
        /**
         * @brief   Initialise measurement fields on the frame.
         *
         * The size of the fields and the maxshift is used to
         * calculate an optimal distribution in the frame.
         * if border is set then they are placed savely
         * away from the border for maxShift
        */
        void _initFields(VSMotionDetectFields* fs,
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
        LocalMotions _calcTransFields(VSMotionDetectFields* fields,
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
        void _blur(const VSFrame* aFrame);
        
        
        /**
         * @brief   Performs a boxblur operation on src and stores results in dest
         *
         * It uses an accumulator method and separate horizontal and vertical runs
         *
         * @param   aDst        Destination buffer
         * @param   aSrc        Source buffer
         * @param   aBuffer     Frame buffer
         * @param   aFi         Frame information
         * @param   aStepSize   Size of bluring kernel (min 3 and it is made odd)
         * @param   aColormode  Which color mode is used
         *
         * @return  Blured frame
         */
        const VSFrame* _blurBox(VSFrame&           aDst,
                                const VSFrame&     aSrc,
                                VSFrame&           aBuffer,
                                const VSFrameInfo& aFi,
                                unsigned int       aStepSize,
                                _BoxBlurColorMode  aColormode);
                                
                                
        void _blurBoxHV(unsigned char*       dst,
                        unsigned char*       tmp,
                        const unsigned char* src,
                        int                  width,
                        int                  height,
                        int                  dst_strive,
                        int                  src_strive,
                        int                  size);
                        
                        
        static void _blurBoxH(unsigned char*       dst,
                              const unsigned char* src,
                              int                  width,
                              int                  height,
                              int                  dst_strive,
                              int                  src_strive,
                              int                  size);
                              
                              
        static void _blurBoxV(unsigned char*       dst,
                              const unsigned char* src,
                              int                  width,
                              int                  height,
                              int                  dst_strive,
                              int                  src_strive,
                              int                  size);
                              
                              
        /**
         * @brief       Detect motion
         *
         * @param[out]  aMotions    Motions vector
         * @param[in]   aFrame      Current frame
         */
        void _detect(LocalMotions*  aMotions,
                     const VSFrame* aFrame);
                     
                     
        /**
         * @brief   Detect motion - contrast part
         * @param   aMotionscoarse    Coarse motion results
         * @return  Number of detected motions
         */
        int _detectContrast(LocalMotions& aMotionscoarse);
        
        
        /**
         * @brief   Draw results of detection
         * @param   num_motions Number of motions
         */
        void _draw(int           num_motions,
                   LocalMotions& motionscoarse,
                   LocalMotions& motionsfine);
                   
                   
        /** draws the field scanning area */
        void _drawFieldScanArea(const LocalMotion* lm,
                                int                maxShift);
                                
                                
        /** draws the field */
        void _drawField(const LocalMotion* lm,
                        short              box);
                        
                        
        /** draws the transform data of this field */
        void _drawFieldTrans(const LocalMotion* lm,
                             int                color);
                             
                             
        /* select only the best 'maxfields' fields
           first calc contrasts then select from each part of the
           frame some fields
           We may simplify here by using random. People want high quality, so typically we use all.
        */
        VSVector _selectfields(VSMotionDetectFields* fs,
                               contrastSubImgFunc    contrastfunc);
                               
                               
        /**
         * @brief   Module name
         */
        std::string _mn;
        
        
        /**
         * @brief   OpenCL device
         */
        cl::Device _clDevice;
        
        
        /**
         * @brief   OpenCL context (communication channel to device)
         */
        cl::Context* _clContext;
        
        
        /**
         * @brief   Sources of code we want to run over OpenCL
         */
        cl::Program::Sources _clSources;
        
        
        /**
         * @brief   Compiled binary of code we want to run over OpenCL
         */
        cl::Program* _clProgram;
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
    
    
    /**
     * @brief   Convert motion detect instance to C++ representation
     * @param   aMd     Motion detect instance
     * @return  C++ representation of motion detect instance
     */
    inline const VSMD& VSMD2Inst(const VSMotionDetect* aMd)
    {
        assert(nullptr != aMd);
        const VSMD* const md = (VSMD*)aMd->_inst;
        return *md;
    }
    
    
    /* calculates the optimal transformation for one field in Packed
     * slower than the Planar version because it uses all three color channels
     */
    LocalMotion visitor_calcFieldTransPacked(VSMD*                 md,
                                             VSMotionDetectFields* fs,
                                             const Field*          field,
                                             int                   fieldnum);
                                             
                                             
    /* calculates the optimal transformation for one field in Planar frames
     * (only luminance)
     */
    LocalMotion visitor_calcFieldTransPlanar(VSMD*                 md,
                                             VSMotionDetectFields* fs,
                                             const Field*          field,
                                             int                   fieldnum);
                                             
                                             
    /**
       \see contrastSubImg_Michelson three times called with bytesPerPixel=3
       for all channels
    */
    double visitor_contrastSubImgPacked(VSMD*        md,
                                        const Field* field);
                                        
                                        
    /** \see contrastSubImg*/
    double visitor_contrastSubImgPlanar(VSMD*        md,
                                        const Field* field);
}
