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
     * @brief   Motions serializer header
     */
    struct MotionsSHdr
    {
        Common::Vect<unsigned> frameSize;
    };
    
    
    /**
     * @brief   Serializer for detected motions
     */
    class MotionsSerializer
    {
    public:
        MotionsSerializer(const std::string& aFileName) noexcept;
        
        
        ~MotionsSerializer();
        
        
        void create(const MotionsSHdr& aHdr);
        
        
    private:
        const std::string _fileName;
        std::ofstream     _file;
    };
    
    
    /**
     * @brief   Deserializer of detected motions
     */
    class MotionsDeserializer
    {
    public:
        MotionsDeserializer(const std::string& aFileName) noexcept;
        
        
    private:
        const std::string _fileName;
        std::ifstream     _file;
    };
}

