/*
 * gimbal_motions.cpp
 *
 *  Created on: 27. 10. 2018
 *      Author: ondiiik
 */
#include "gimbal_motions.h"


namespace Gimbal
{
    MotionsSerializer::MotionsSerializer(const std::string& aFileName) noexcept
        :
        _fileName
    {
        aFileName
    },
    _file {}
    {
    
    }
    
    
    MotionsSerializer::~MotionsSerializer()
    {
        if (_file.is_open())
        {
            _file.close();
        }
    }
    
    
    void MotionsSerializer::create(const MotionsSHdr& aHdr)
    {
        _file.open("example.bin",
                   std::ios::out    |
                   std::ios::app    |
                   std::ios::binary |
                   std::ios::trunc);
                   
        MotionsSHdr*                        hdr = new MotionsSHdr(aHdr);
        _file.write(reinterpret_cast<char*>(hdr), sizeof(*hdr));
    }
    
    
    MotionsDeserializer::MotionsDeserializer(const std::string& aFileName) noexcept
        :
        _fileName
    {
        aFileName
    },
    _file {}
    {
    
    }
}
