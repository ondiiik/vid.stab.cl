/*
 * gimbal_serializer.cpp
 *
 *  Created on: 28. 10. 2018
 *      Author: ondiiik
 */
#include "gimbal_serializer.h"


#define VERSION_ID  'G', 'B', 'L', 'F', '0', '0', '0', '1'


namespace
{
    const char version_id[8] = { VERSION_ID };
}



namespace Gimbal
{
    Serializer::Serializer(const std::string& aFileName) noexcept
        :
        _fileName
    {
        aFileName
    },
    _file {}
    {
    
    }
    
    
    Serializer::~Serializer()
    {
        if (_file.is_open())
        {
            _file.close();
        }
    }
    
    
    void Serializer::create(const SerializerHdr& aHdr)
    {
        _file.open(_fileName.c_str(), std::ios::out | std::ios::binary);
        SerializerHdr*                      hdr = new SerializerHdr(aHdr);
        _file.write(reinterpret_cast<char*>(hdr), sizeof(*hdr));
        _file.flush();
    }
    
    
    void Serializer::write(const Cells&   aCels,
                           const unsigned aIdx)
    {
        SerializerBlockHdr*                 hdr = new SerializerBlockHdr(aCels.list.size());
        _file.write(reinterpret_cast<char*>(hdr), sizeof(*hdr));
        
        for (auto& cell : aCels.list)
        {
            SerializerCell*                     cs = new SerializerCell(cell, aIdx);
            _file.write(reinterpret_cast<char*>(cs), sizeof(*cs));
        }
        
        _file.flush();
    }
    
    
    Deserializer::Deserializer(const std::string& aFileName) noexcept
        :
        _fileName
    {
        aFileName
    },
    _file {}
    {
    
    }
    
    
    SerializerHdr::SerializerHdr(const Common::Vect<unsigned>& aDim)
        :
        id        { VERSION_ID },
        frameSize { aDim       }
    {
    
    }
}
