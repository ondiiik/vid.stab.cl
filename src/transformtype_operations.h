#pragma once
/*
 *  transformtype_operations.h
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
#include "libvidstab.h"
#include "transformtype.h"
#include "vidstabdefines.h"
#include "frameinfo.h"
#include "vs_vector.h"


#ifdef __cplusplus


/// helper macro to access a localmotion in the VSVector
#define LMGet(localmotions,index) \
    ((LocalMotion*)vs_vector_get(localmotions,index))

/* helper functions to create and operate with transforms.
 * all functions are non-destructive
 * the "_" version uses non-pointer Transforms. This is slower
 * but useful when cascading calculations like
 * add_transforms_(mult_transform(&t1, 5.0), &t2)
 */
struct VSTransform null_transform(void);
struct VSTransform new_transform(double x, double y, double alpha,
                          double zoom, double barrel, double rshutter, int extra);
struct VSTransform add_transforms(const struct VSTransform* t1, const struct VSTransform* t2);
struct VSTransform add_transforms_(const struct VSTransform t1, const struct VSTransform t2);
struct VSTransform sub_transforms(const struct VSTransform* t1, const struct VSTransform* t2);
struct VSTransform mult_transform(const struct VSTransform* t1, double f);
struct VSTransform mult_transform_(const struct VSTransform t1, double f);

void storeVSTransform(FILE* f, const struct VSTransform* t);


typedef struct _preparedtransform
{
    const struct VSTransform* t;
    double zcos_a;
    double zsin_a;
    double c_x;
    double c_y;
} PreparedTransform;

// transforms vector
PreparedTransform prepare_transform(const struct VSTransform* t, const VSFrameInfo* fi);
// transforms vector (attention, only integer)
Vec transform_vec(const PreparedTransform* t, const Vec* v);
void transform_vec_double(double* x, double* y, const PreparedTransform* t, const Vec* v);

/* compares a transform with respect to x (for sort function) */
int cmp_trans_x(const void* t1, const void* t2);
/* compares a transform with respect to y (for sort function) */
int cmp_trans_y(const void* t1, const void* t2);
/* static int cmp_trans_alpha(const void *t1, const void* t2); */

/* compares two double values (for sort function)*/
int cmp_double(const void* t1, const void* t2);
/* compares two int values (for sort function)*/
int cmp_int(const void* t1, const void* t2);


/** square of a number */
double sqr(double x);

/* calculates the median of an array of transforms,
 * considering only x and y
 */
struct VSTransform median_xy_transform(const struct VSTransform* transforms, int len);
/* median of a double array */
double median(double* ds, int len);
/* mean of a double array */
double mean(const double* ds, int len);
/* standard deviation of a double array */
double stddev(const double* ds, int len, double mean);
/* mean with cutted upper and lower pentile
 * (min and max are optionally returned)
 */
double cleanmean(double* ds, int len, double* minimum, double* maximum);
/* calulcates the cleaned mean of an array of transforms,
 * considerung only x and y
 */
struct VSTransform cleanmean_xy_transform(const struct VSTransform* transforms, int len);

/* calculates the cleaned (cutting of x-th percentil)
 * maximum and minimum of an array of transforms,
 * considerung only x and y
 */
void cleanmaxmin_xy_transform(const struct VSTransform* transforms, int len,
                              int percentil,
                              struct VSTransform* min, struct VSTransform* max);

/* calculates the required zoom value to have no borders visible
 */
double transform_get_required_zoom(const struct VSTransform* transform, int width, int height);

/* helper function to work with local motions */

LocalMotion null_localmotion(void);
/// a new array of the v.x values is returned (vs_free has to be called)
int* localmotions_getx(const LocalMotions* localmotions);
/// a new array of the v.y values is returned (vs_free has to be called)
int* localmotions_gety(const LocalMotions* localmotions);
/// lm1 - lm2 only for the Vec (the remaining values are taken from lm1)
LocalMotion sub_localmotion(const LocalMotion* lm1, const LocalMotion* lm2);

/* calulcates the cleaned mean of the vector of localmotions
 * considerung only v.x and v.y
 */
LocalMotion cleanmean_localmotions(const LocalMotions* localmotions);

VSArray localmotionsGetMatch(const LocalMotions* localmotions);

/* helper functions */

/* optimized round function */
inline static int myround(float x)
{
    if (x > 0)
    {
        return x + 0.5;
    }
    else
    {
        return x - 0.5;
    }
}


/* optimized floor function
   This does not give the correct value for negative integer values like -1.0. In this case
   it will produce -2.0.
*/
inline static int myfloor(float x)
{
    if (x < 0)
    {
        return x - 1;
    }
    else
    {
        return x;
    }
}


#endif


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
