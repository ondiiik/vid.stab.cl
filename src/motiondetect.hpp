#pragma once
/*
 * motiondetect.hpp
 *
 *  Created on: 15. 9. 2018
 *      Author: ondiiik
 */


#include "motiondetect.h"


namespace VidStab
{
    /**
     * @brief   Data structure for motion detection part of deshaking
     * @note    Structure must be also type because is used in C
     *          in ffmpeg
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
         * @param   aMd     Parrent C instance used by external tools such as ffmpeg
         * @param   aConf   Initial configuration
         * @param   aFi     Frame info
         */
        VSMD(VSMotionDetect*             aMd,
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
        VSFrameInfo& fi;
        
        VSMotionDetectConfig conf;
        
        VSMotionDetectFields fieldscoarse;
        VSMotionDetectFields fieldsfine;
        
        VSFrame curr;                 // blurred version of current frame buffer
        VSFrame currorig;             // current frame buffer (original) (only pointer)
        VSFrame currtmp;              // temporary buffer for blurring
        VSFrame prev;                 // frame buffer for last frame (copied)
        bool    hasSeenOneFrame;      // true if we have a valid previous frame
        int     initialized;          // 1 if initialized and 2 if configured
        
        int frameNum;
        
        
    private:
        /** initialise measurement fields on the frame.
            The size of the fields and the maxshift is used to
            calculate an optimal distribution in the frame.
            if border is set then they are placed savely away from the border for maxShift
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
