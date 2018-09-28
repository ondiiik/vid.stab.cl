#pragma once
/*
 *  motiondetect_internal.h
 *
 *  Copyright (C) Georg Martius - February 2011
 *   georg dot martius at web dot de
 *  Copyright (C) Alexey Osipov - Jule 2011
 *   simba at lerlan dot ru
 *   speed optimizations (threshold, spiral, SSE, asm)
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
#include "motiondetect.h"


#ifdef __cplusplus
extern "C" {
#endif


double contrastSubImg(unsigned char* const I, const Field* field,
                      int width, int height, int bytesPerPixel);


#ifdef __cplusplus
}
#endif


class Correlate
{
public:
    Correlate(const uint8_t* aCurrent,
              const uint8_t* aPrevious,
              const Field&   aField,
              int            aWidthCurrent,
              int            aWidthPrevious,
              int            aHeight,
              int            aBpp)
        :
        _current       { aCurrent           },
        _previous      { aPrevious          },
        _field         { aField             },
        _widthCurrent  { aWidthCurrent      },
        _widthPrevious { aWidthPrevious     },
        _height        { aHeight            },
        _bpp           { aBpp               },
        _wbpp          { aBpp * aField.size }
    {

    }


    /**
     * @brief    Correlate immages
     */
    unsigned int operator()(int            aOffsetX,
                            int            aOffsetY,
                            unsigned int   aThreshold);


private:
    const uint8_t* const _current;
    const uint8_t* const _previous;
    const Field&         _field;
    const int            _widthCurrent;
    const int            _widthPrevious;
    const int            _height;
    const int            _bpp;
    const int            _wbpp    = _field.size * _bpp;
};


/*
 * Local variables:
 *   c-file-style: "stroustrup"
 *   c-file-offsets: ((case-label . *) (statement-case-intro . *))
 *   indent-tabs-mode: nil
 *   c-basic-offset: 2 t
 * End:
 *
 * vim: expandtab shiftwidth=2:
 */
