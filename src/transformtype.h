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



/**
 * @brief   Represents x y coordinates vector
 */
template <typename _Tp> struct Vect
{
    Vect()
        :
        x { _Tp(0) },
        y { _Tp(0) }
    {
    
    }
    
    
    Vect(_Tp aX,
         _Tp aY)
        :
        x { aX },
        y { aY }
    {
    
    }
    
    
    inline Vect& operator-=(const Vect& aSrc)
    {
        this->x -= aSrc.x;
        this->y -= aSrc.y;
        return *this;
    }
    
    
    inline Vect operator-(const Vect& aSrc)
    {
        ;
        Vect r { *this };
        r -= aSrc;
        return r;
    }


    inline Vect& operator+=(const Vect& aSrc)
    {
        this->x += aSrc.x;
        this->y += aSrc.y;
        return *this;
    }


    inline Vect operator+(const Vect& aSrc)
    {
        ;
        Vect r { *this };
        r += aSrc;
        return r;
    }


    _Tp x;
    _Tp y;
};


typedef Vect<int> Vec;



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


/** stores x y and size of a measurement field */
typedef struct Field
{
    int x;     // middle position x
    int y;     // middle position y
    int size;  // size of field
}
Field;


/* structure to hold information about local motion.
 */
typedef struct LocalMotion
{
    Vec v;
    Field f;
    double contrast; // local contrast of the measurement field
    double match;    // quality of match
}
LocalMotion;


typedef VSVector LocalMotions;


#ifdef __cplusplus
}
#endif


