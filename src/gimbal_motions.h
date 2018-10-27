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

