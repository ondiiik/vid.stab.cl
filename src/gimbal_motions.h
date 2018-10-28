#pragma once
/*
 * gimbal_motions.h
 *
 *  Created on: 27. 10. 2018
 *      Author: ondiiik
 */
#include "common_vect.h"
#include <string>
#include <fstream>
#include <vector>


namespace Gimbal
{
    /**
     * @brief   Defines filter layer index
     *
     * There is several filters layers for various purposes used
     * during detection
     */
    enum FilterLayer
    {
        FLR_FAST0,    /**!< @brief Auxiliary layer used for flip with base fast filter layer*/
        FLR_FAST,     /**!< @brief Base fast filter layer */
        FLR_SLOW_A,   /**!< @brief Slow filter - in phase */
        FLR_SLOW_B,   /**!< @brief Slow filter - half phase */
        FLR_STATIC_A, /**!< @brief Static filter - in phase */
        FLR_STATIC_B, /**!< @brief Static filter - half phase */
        __FLR_CNT     /**!< @brief Count of filters */
    };
    
    
    /**
     * @brief   Direction value
     */
    struct DirVal
    {
        DirVal()
            :
            meas     {   },
            esti     {   },
            val      {   },
            contrast { 0 },
            dist     { 0 }
        {
        
        }
        
        /**
         * @brief   Measured value
         */
        Common::Vect<int> meas;
        
        /**
         * @brief   Estimated value
         */
        Common::Vect<int> esti;
        
        /**
         * @brief   Filtered value
         */
        Common::Vect<int> val;
        
        /**
         * @brief   Contrast factor
         */
        unsigned contrast;
        
        /**
         * @brief   Distance of closest neighbors during estimation
         */
        unsigned dist;
    };
    
    
    /**
     * @brief   Direction vector structure
     */
    class Direction
    {
    public:
        /**
         * @brief   Default constructor
         */
        Direction()
            :
            velo   {                        },
            _valid { Direction::DIR___VALID }
        {
        
        }
        
        
        /**
         * @brief   Direction is valid
         */
        static const unsigned DIR___VALID { 0b000U };
        
        /**
         * @brief   Direction disabled by low contrast detection
         */
        static const unsigned DIR___CONTRAST { 0b001U };
        
        /**
         * @brief   Direction disabled by not enough surroundings points
         */
        static const unsigned DIR___SURROUNDINGS { 0b010U };
        
        /**
         * @brief   Direction disabled by big estimation deviation
         */
        static const unsigned DIR___ESTI_DEV { 0b100U };
        
        
        /**
         * @brief   Check if flag is set
         * @param   aFlags  Flag to be checked
         * @return  Result
         */
        inline unsigned flags() const
        {
            return _valid;
        }
        
        
        /**
         * @brief   Check if flag is set
         * @param   aFlags  Flag to be checked
         * @return  Result
         */
        inline bool isValid() const
        {
            return _valid == DIR___VALID;
        }
        
        
        /**
         * @brief   Clear all flags
         */
        inline void init(unsigned aInit = DIR___VALID)
        {
            _valid = aInit;
        }
        
        
        /**
         * @brief   Clear flag
         * @param   aFlags  Flag to be set
         */
        inline void clr(unsigned aFlag)
        {
            _valid &= ~aFlag;
        }
        
        
        /**
         * @brief   Set flag
         * @param   aFlags  Flag to be set
         */
        inline void set(unsigned aFlag)
        {
            _valid |= aFlag;
        }
        
        
        /**
         * @brief   Check if flag is set
         * @param   aFlags  Flag to be checked
         * @return  Result
         */
        inline bool isSet(unsigned aFlag) const
        {
            return 0 != (_valid & aFlag);
        }
        
        
        /**
         * @brief   History count of direction
         */
        static const unsigned hcnt { 8 };
        
        
        /**
         * @brief   Convert frame index to vector index
         * @param   aIdx    Frame index
         * @return  Vector index
         */
        static inline unsigned frame2vidx(unsigned aIdx)
        {
            return aIdx % hcnt;
        }
        
        
        /**
         * @brief   Measured and estimated cell direction vectors
         *
         * Vectors are in array to cover also history of cell movement
         * for better detection algorithm.
         */
        DirVal velo[hcnt];
        
        
    private:
        /**
         * @brief   Cell validity flag
         *
         * Some cells can be invalidated in some post-process step.
         * We uses vector, so is much faster to invalidate item them
         * instead of removing which is slow in the case of vector.
         *
         * Validity is set of flags, however only 0 (DIR___VALID) is
         * represented as valid.
         */
        unsigned _valid;
    };
    
    
    /**
     * @brief   Detection cell
     */
    class Cell
    {
    public:
        static inline unsigned ptype2dir(FilterLayer aPType)
        {
            return aPType - FLR_FAST;
        }
        
        
        inline Direction& operator[](FilterLayer aPType)
        {
            return direction[aPType];
        }
        
        
        /**
         * @brief   Position of center of cell
         */
        Common::Vect<int> position;
        
        /**
         * @brief   Cell size
         */
        Common::Vect<unsigned> size;
        
        /**
         * @brief   Cell index in cells list
         */
        Common::Vect<unsigned> idx;
        
        /**
         * @brief   Detected cell direction
         */
        Direction direction[__FLR_CNT - FLR_FAST];
    };
    
    
    /**
     * @brief   Cells set
     */
    struct Cells
    {
        inline Cell& operator[](const Common::Vect<unsigned>& aVec)
        {
            return list[aVec.y * dim.x + aVec.x];
        }
        
        
        inline Cell& operator()(unsigned aX,
                                unsigned aY)
        {
            return list[aY * dim.x + aX];
        }
        
        
        /**
         * @brief   Cells array dimension
         */
        Common::Vect<unsigned> dim;
        
        
        /**
         * @brief   Cells array
         */
        std::vector<Cell> list;
    };
}
