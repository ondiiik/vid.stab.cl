/*
 * gimbal_serializer.cpp
 *
 *  Created on: 28. 10. 2018
 *      Author: ondiiik
 */
#include "gimbal_serializer.h"
#include "common_exception.h"
#include <cstring>


#define VERSION_ID  'G', 'B', 'L', 'F', '0', '0', '0', '1'


namespace
{
    const char moduleName[]  { "serializer" };
    const char version_id[8] { VERSION_ID   };
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
    
    
    void Serializer::write(const Cells<dtHistCnt>& aCels,
                           const unsigned      aIdx)
    {
        /*
         * Search for cells which contains at least one valid
         * layer.
         */
        unsigned cnt { 0 };
        
        for (auto& cell : aCels.list)
        {
            for (auto& i : cell.direction)
            {
                if (i.isValid())
                {
                    ++cnt;
                    break;
                }
            }
        }
        
        
        /*
         * Write header of frame
         */
        SerializerBlockHdr*                 hdr = new SerializerBlockHdr(cnt);
        _file.write(reinterpret_cast<char*>(hdr), sizeof(*hdr));
        
        
        /*
         * Skip writing when there are no valid cells
         */
        if (0 != cnt)
        {
            /*
             * Go through all cells and find all valid.
             * They will be written. The rest will be skipped.
             */
            for (auto& cell : aCels.list)
            {
                for (auto& i : cell.direction)
                {
                    if (i.isValid())
                    {
                        SerializerCell*                     cs = new SerializerCell(cell, aIdx);
                        _file.write(reinterpret_cast<char*>(cs), sizeof(*cs));
                        break;
                    }
                }
            }
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
    
    
    void Deserializer::load()
    {
        /*
         * Read and check head first
         */
        _file.open(_fileName.c_str(), std::ios::in | std::ios::binary);
        SerializerHdr hdr;
        _file.read(reinterpret_cast<char*>(&hdr), sizeof(hdr));
        
        hdr.checkValidity();
        
        _dim = hdr.frameSize;
    }
    
    
    SerializerHdr::SerializerHdr()
        :
        id        { VERSION_ID },
        frameSize {            }
    {
    
    }
    
    
    SerializerHdr::SerializerHdr(const Common::Vect<unsigned>& aDim)
        :
        id        { VERSION_ID },
        frameSize { aDim       }
    {
    
    }
    
    
    void SerializerHdr::checkValidity() const
    {
        if (0 != memcmp(version_id, id, sizeof(version_id)))
        {
            throw Common::EXCEPTION("Incompatible detector version "
                                    "(required %c%c%c%c%c%c%c%c, but got %c%c%c%c%c%c%c%c)!",
                                    version_id[0],
                                    version_id[1],
                                    version_id[2],
                                    version_id[3],
                                    version_id[4],
                                    version_id[5],
                                    version_id[6],
                                    version_id[7],
                                    id[        0],
                                    id[        1],
                                    id[        2],
                                    id[        3],
                                    id[        4],
                                    id[        5],
                                    id[        6],
                                    id[        7]);
        }
    }
}
