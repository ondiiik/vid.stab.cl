#pragma once
/*
 *  transform.h
 *
 *  Copyright (C) Georg Martius - June 2007 - 2013
 *
 *  This file is part of transcode, a video stream processing tool
 *
 *  transcode is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  transcode is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with GNU Make; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */
#include <stdio.h>
#include "vsvector.h"


#include "common_vect.h"


typedef Common::Vect<int> Vec;


/**
 * @brief   Represents x y and size of a measurement field
 */
class Field : public Vec
{
public:
    Field()
        :
        Vec  {   },
        size { 0 }
    {
    
    }
    
    
    Field(int aX,
          int aY,
          int aSize)
        :
        Vec  { aX, aY },
        size { aSize  }
    {
    
    }
    
    int size;
};


#ifdef __cplusplus
extern "C" {
#endif


/* structure to hold information about frame transformations
   x,y are translations, alpha is a rotation around the center in RAD,
   zoom is a percentage to zoom in and
   extra is for additional information like scene cut (unused)
 */
typedef struct VSTransform
{
    double x;
    double y;
    double alpha;
    double zoom;
    double barrel;
    double rshutter;
    int    extra;    /* -1: ignore transform (only internal use);
                     0 for normal trans; 1 for inter scene cut (unused) */
}
VSTransform;


/* structure to hold information about local motion.
 */
typedef struct LocalMotion
{
    Vec   v;
    Field f;
    double contrast; // local contrast of the measurement field
    double match;    // quality of match
}
LocalMotion;


typedef VSVector LocalMotions;


#ifdef __cplusplus
}
#endif


