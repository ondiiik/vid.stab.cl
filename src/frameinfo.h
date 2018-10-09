#pragma once
/*
 *  frameinfo.h
 *
 *  Copyright (C) Georg Martius - June 2007 - 2011
 *   georg dot martius at web dot de
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
#include "libvidstab.h"
#include "common_vect.h"
#include <cstddef>
#include <cstring>


// use it to calculate the CHROMA sizes (rounding is correct)
#define CHROMA_SIZE(width,log2sub)  (-(-(width) >> (log2sub)))


namespace Frame
{
    /**
     * @brief   Represents information about frame
     */
    class Info
    {
    public:
        Info(const VSFrameInfo& aInfo)
            :
            _info { aInfo }
        {
        
        }
        
        
        /**
         * @brief   Compare two info structures
         * @param   aSrc    Another info structure
         * @return  Result
         * @retval  true    Frames are the same
         * @retval  false   Frames are different
         */
        inline bool operator==(const Info& aSrc) const
        {
            return (0 == memcmp(&_info, &aSrc._info, sizeof(_info)));
        }
        
        
        /**
         * @brief   Compare two info structures
         * @param   aSrc    Another info structure
         * @return  Result
         * @retval  true    Frames are different
         * @retval  false   Frames are the same
         */
        inline bool operator!=(const Info& aSrc) const
        {
            return !(*this == aSrc);
        }
        
        
        inline int planes() const
        {
            return _info.planes;
        }
        
        
        inline int pixFormat() const
        {
            return _info.pFormat;
        }
        
        
        inline int bpp() const
        {
            return _info.bytesPerPixel;
        }
        
        
        inline int width() const
        {
            return _info.width;
        }
        
        
        inline int height() const
        {
            return _info.height;
        }
        
        
        inline Common::Vect<unsigned> dim() const
        {
            return Common::Vect<unsigned>(_info.width, _info.height);
        }
        
        
        /**
         * @brief  Returns the subsampling shift amount horizonatally
         *         for the given plane
         *
         */
        int subsampleWidth(int aPlane) const
        {
            if (aPlane == 1)
            {
                return 1;
            }
            
            if (aPlane != 2)
            {
                return 0;
            }
            
            if (_info.log2ChromaW)
            {
                return 1;
            }
            else
            {
                return 0;
            }
        }
        
        
        /**
         * @brief  Packed line width
         */
        int sublineWidth(int aPlane) const
        {
            return int(unsigned(_info.width) >> subsampleWidth(aPlane));
        }
        
        
        /**
         * @brief  Packed row height
         */
        int sublineHeight(int aPlane) const
        {
            return int(unsigned(_info.height) >> subsampleHeight(aPlane));
        }
        
        
        /**
         * @brief   Returns the subsampling shift amount vertically for
         *          the given plane
         */
        int subsampleHeight(int aPlane) const
        {
            if (aPlane == 1)
            {
                return 1;
            }
            
            if (aPlane != 2)
            {
                return 0;
            }
            
            if (_info.log2ChromaH)
            {
                return 1;
            }
            else
            {
                return 0;
            }
        }
        
        
        std::size_t calcSize(unsigned aPlane) const;
        
        
        inline int calcLineWidth(unsigned aPlane) const
        {
            if (pixFormat() < PF_PACKED)
            {
                return int(unsigned(_info.width) >> subsampleWidth(aPlane));
            }
            else
            {
                return int(_info.width * bpp());
            }
        }
        
        
    private:
        const VSFrameInfo& _info;
    };
    
    
    /*
     * Forward declaration
     */
    class Frame;
    
    
    /**
     * @brief   Represents one frame plane
     */
    class Plane
    {
    public:
        Plane(uint8_t* aBuff,
              int      aLineSize,
              Frame&   aFrame,
              int      aIdx)
            :
            _buff     { aBuff     },
            _lineSize { aLineSize },
            _frame    { aFrame    },
            _idx      { aIdx      }
        {
        
        }
        
        
        Plane(uint8_t*     aBuff,
              int          aLineSize,
              const Frame& aFrame,
              int          aIdx)
            :
            _buff     { aBuff                          },
            _lineSize { aLineSize                      },
            _frame    { *(const_cast<Frame*>(&aFrame)) },
            _idx      { aIdx                           }
        {
        
        }
        
        
        /**
         * @brief   Process copy of plane
         * @param   aSrc    Source plane
         * @return  This
         */
        Plane& operator=(const Plane& aSrc);


        inline uint8_t* buff()
        {
            return _buff;
        }
        
        
        inline const uint8_t* buff() const
        {
            return _buff;
        }
        
        
        inline int lineSize() const
        {
            return _lineSize;
        }
        
        
    private:
        uint8_t* _buff;
        int      _lineSize;
        Frame&   _frame;
        int      _idx;
    };
    
    
    /**
     * @brief   Represents frame
     */
    class Frame
    {
    public:
        Frame(VSFrame&    aFrame,
              const Info& aInfo);
              
              
        Frame(const VSFrame& aFrame,
              const Info&    aInfo);
              
              
        /**
         * @brief   Compare frame
         * @param   aSrc    Source frame
         * @return  Comparison result
         * @retval  true    Frames are coming from the same source
         * @retval  false   Frames are not coming from the same source
         */
        bool operator==(const Frame& aSrc);
        
        
        /**
         * @brief   Compare frame
         * @param   aSrc    Source frame
         * @return  Comparison result
         * @retval  true    Frames are coming from the same source
         * @retval  false   Frames are not coming from the same source
         */
        inline bool operator!=(const Frame& aSrc)
        {
            return !(*this == aSrc);
        }
        
        
        /**
         * @brief   Copy frame
         * @param   aSrc    Source frame
         * @return  This frame
         */
        Frame& operator=(const Frame& aSrc);
        
        
        /**
         * @brief   Get requested plane buffer
         * @param   aPlane  Plane IDX
         * @return  Plane buffer
         */
        inline Plane operator[](std::size_t aPlane)
        {
            return Plane(_frame.data[aPlane],
                         _frame.linesize[aPlane],
                         *this,
                         int(aPlane));
        }
        
        
        /**
         * @brief   Get requested plane buffer
         * @param   aPlane  Plane IDX
         * @return  Plane buffer
         */
        inline const Plane operator[](std::size_t aPlane) const
        {
            return Plane(_frame.data[aPlane],
                         _frame.linesize[aPlane],
                         *this,
                         int(aPlane));
        }
        
        
        /**
         * @brief   Allocates memory for frame
         */
        void alloc();
        
        
        /**
         * @brief   Free memory for frame
         */
        void free();
        
        
        /**
         * @brief   Forget all layers but don't free then
         */
        void forget();
        
        
        /**
         * @brief   Tell us if frame is empty (forgotten)
         * @return  Result
         * @retval  true    Frame is empty (forgotten)
         * @retval  false   There are data in frame
         */
        inline bool empty() const
        {
            return (nullptr == _frame.data[0]);
        }
        
        
        /**
         * @brief   Frame information object getter
         * @return  Frame information object
         */
        inline const Info& info() const
        {
            return _info;
        }
        
        
    private:
        /**
         * @brief   Post-initializer called from constructor
         */
        void _init();
        
        
        /**
         * @brief   C frame reference
         */
        VSFrame& _frame;
        
        /**
         * @brief   C frame information
         */
        const Info& _info;
    };
}
