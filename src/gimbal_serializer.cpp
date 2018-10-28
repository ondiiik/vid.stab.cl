/*
 * gimbal_serializer.cpp
 *
 *  Created on: 28. 10. 2018
 *      Author: ondiiik
 */
#include "gimbal_serializer.h"
#include "common_exception.h"
#include <cstring>

#if defined(USE_COUT)
#include <iostream>
#endif


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
    
    
    void Serializer::write(const DetectorCells& aCels,
                           const unsigned       aIdx)
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
        SerializerHdr h;
        _file.read(reinterpret_cast<char*>(&h), sizeof(h));
        
        h.checkValidity();
        
        _dim = h.frameSize;
        
        
        /*
         * Now we shall read all blocks for each frame
         */
        while (!_file.eof())
        {
            /*
             * Read and validate block header
             */
            SerializerBlockHdr bh;
            _file.read(reinterpret_cast<char*>(&bh), sizeof(bh));
            
            if (_file.eof())
            {
                break;
            }
            
            bh.checkValidity();
            
            /*
             * Create new cells element and append it to list
             */
            CorrectorCells  c;
            cells.push_back(c);
            
            /*
             * Use last append element as cell to be filled in.
             * We should prepare block of memory to read all blocks.
             */
            CorrectorCells& cls = cells[cells.size() - 1];
            cls.list.reserve(bh.cnt);
            
            for (unsigned i = 0; i < bh.cnt; ++i)
            {
                SerializerCell                      sc;
                _file.read(reinterpret_cast<char*>(&sc), sizeof(sc));
                sc.checkValidity();
                
                CorrectorCell cell;
                sc(cell);
                cls.list.push_back(cell);
            }
        }
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
    
    
    SerializerBlockHdr::SerializerBlockHdr(unsigned aCnt)
        :
        id  { 'B', 'L'       },
        cnt { uint16_t(aCnt) }
    {
    
    }
    
    
    SerializerBlockHdr::SerializerBlockHdr()
        :
        id  { 'X', 'X' },
        cnt { 0U       }
    {
    
    }
    
    
    void SerializerBlockHdr::checkValidity() const
    {
        if (('B' != id[0]) || ('L' != id[1]))
        {
            throw Common::EXCEPTION("Block read mismatch!");
        }
    }
    
    
    SerializerCell::SerializerCell(const DetectorCell& aCell,
                                   unsigned               aIdx)
        :
        id       { 'C', 'L'       },
        position { aCell.position }
    {
        for (unsigned i = 0; i < sizeof(direction) / sizeof(direction[0]); ++i)
        {
            auto& dst    = direction[i];
            auto& src    = aCell.direction[i];
            auto& velo   = src.velo[aIdx];
            
            dst.meas     = velo.meas;
            dst.esti     = velo.esti;
            dst.val      = velo.val;
            dst.contrast = velo.contrast;
            dst.dist     = velo.dist;
            dst.valid    = src.valid;
        }
    }
    
    
    SerializerCell::SerializerCell()
        :
        id       { 'X', 'X' },
        position {          }
    {
    
    }
    
    
    void SerializerCell::checkValidity() const
    {
        if (('C' != id[0]) || ('L' != id[1]))
        {
            throw Common::EXCEPTION("Block read mismatch!");
        }
    }
    
    
    void SerializerCell::operator()(CorrectorCell& aCell)
    {
        aCell.position = position;
        
        for (unsigned i = 0; i < sizeof(direction) / sizeof(direction[0]); ++i)
        {
            auto& dst = aCell.direction[i];
            auto& src = direction[i];
            
            dst.meas     = src.meas;
            dst.esti     = src.esti;
            dst.val      = src.val;
            dst.contrast = src.contrast;
            dst.dist     = src.dist;
            dst.valid    = src.valid;
        }
    }
}
