/**
 *  transformtype.cpp
 *
 *  Copyright (C) Georg Martius - June 2007
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
#include <vector>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "transformtype.h"
#include "transformtype_operations.h"
#include "vidstabdefines.h"


using namespace std;


/***********************************************************************
 * helper functions to create and operate with transforms.
 * all functions are non-destructive
 */

/* create an initialized transform*/
struct VSTransform new_transform(double x, double y, double alpha,
                          double zoom, double barrel, double rshutter, int extra)
{
    struct VSTransform t;
    t.x        = x;
    t.y        = y;
    t.alpha    = alpha;
    t.zoom     = zoom;
    t.barrel   = barrel;
    t.rshutter = rshutter;
    t.extra    = extra;
    return t;
}

/* create a zero initialized transform*/
struct VSTransform null_transform(void)
{
    return new_transform(0, 0, 0, 0, 0, 0, 0);
}

/* adds two transforms */
struct VSTransform add_transforms(const struct VSTransform* t1, const struct VSTransform* t2)
{
    struct VSTransform t;
    t.x        = t1->x + t2->x;
    t.y        = t1->y + t2->y;
    t.alpha    = t1->alpha + t2->alpha;
    t.zoom     = t1->zoom + t2->zoom;
    t.barrel   = t1->barrel + t2->barrel;
    t.rshutter = t1->rshutter + t2->rshutter;
    t.extra    = t1->extra || t2->extra;
    return t;
}

/* like add_transform but with non-pointer signature */
struct VSTransform add_transforms_(const struct VSTransform t1, const struct VSTransform t2)
{
    return add_transforms(&t1, &t2);
}

/* subtracts two transforms */
struct VSTransform sub_transforms(const struct VSTransform* t1, const struct VSTransform* t2)
{
    struct VSTransform t;
    t.x        = t1->x - t2->x;
    t.y        = t1->y - t2->y;
    t.alpha    = t1->alpha - t2->alpha;
    t.zoom     = t1->zoom - t2->zoom;
    t.barrel   = t1->barrel - t2->barrel;
    t.rshutter = t1->rshutter - t2->rshutter;
    t.extra    = t1->extra || t2->extra;
    return t;
}

/* multiplies a transforms with a scalar */
struct VSTransform mult_transform(const struct VSTransform* t1, double f)
{
    struct VSTransform t;
    t.x        = t1->x        * f;
    t.y        = t1->y        * f;
    t.alpha    = t1->alpha    * f;
    t.zoom     = t1->zoom     * f;
    t.barrel   = t1->barrel   * f;
    t.rshutter = t1->rshutter * f;
    t.extra    = t1->extra;
    return t;
}

/* like mult_transform but with non-pointer signature */
struct VSTransform mult_transform_(const struct VSTransform t1, double f)
{
    return mult_transform(&t1, f);
}

void storeVSTransform(FILE* f, const struct VSTransform* t)
{
    fprintf(f, "Trans %lf %lf %lf %lf %i\n", t->x, t->y, t->alpha, t->zoom, t->extra);
}


PreparedTransform prepare_transform(const struct VSTransform* t,
                                    const        VSFrameInfo* fi)
{
    PreparedTransform pt;
    pt.t = t;

    double  z = 1.0 + t->zoom / 100.0;
    pt.zcos_a = z * cos(t->alpha); // scaled cos
    pt.zsin_a = z * sin(t->alpha); // scaled sin
    pt.c_x    = fi->width / 2;
    pt.c_y    = fi->height / 2;

    return pt;
}

Vec transform_vec(const PreparedTransform* pt,
                  const Vec*               v)
{
    double x, y;
    transform_vec_double(&x, &y, pt, v);
    Vec res {int(x), int(y)};
    return res;
}

void transform_vec_double(double* x, double* y, const PreparedTransform* pt, const Vec* v)
{
    double rx = v->x - pt->c_x;
    double ry = v->y - pt->c_y;
    *x =  pt->zcos_a * rx + pt->zsin_a * ry + pt->t->x + pt->c_x;
    *y = -pt->zsin_a * rx + pt->zcos_a * ry + pt->t->y + pt->c_y;
}

/* compares a transform with respect to x (for sort function) */
int cmp_trans_x(const void* t1, const void* t2)
{
    double a = ((struct VSTransform*)t1)->x;
    double b = ((struct VSTransform*)t2)->x;
    return a < b ? -1 : ( a > b ? 1 : 0 );
}

/* compares a transform with respect to y (for sort function) */
int cmp_trans_y(const void* t1, const void* t2)
{
    double a = ((struct VSTransform*)t1)->y;
    double b = ((struct VSTransform*)t2)->y;
    return a < b ? -1 : ( a > b ? 1 : 0 );
}

/* static int cmp_trans_alpha(const void *t1, const void* t2){ */
/*   double a = ((struct VSTransform*)t1)->alpha; */
/*   double b = ((struct VSTransform*)t2)->alpha; */
/*   return a < b ? -1 : ( a > b ? 1 : 0 ); */
/* } */


/* compares two double values (for sort function)*/
int cmp_double(const void* t1, const void* t2)
{
    double a = *((double*)t1);
    double b = *((double*)t2);
    return a < b ? -1 : ( a > b ? 1 : 0 );
}

/* compares two int values (for sort function)*/
int cmp_int(const void* t1, const void* t2)
{
    int a = *((int*)t1);
    int b = *((int*)t2);
    return a < b ? -1 : ( a > b ? 1 : 0 );
}

/**
 * median_xy_transform: calulcates the median of an array
 * of transforms, considering only x and y
 *
 * Parameters:
 *    transforms: array of transforms.
 *           len: length  of array
 * Return value:
 *     A new transform with x and y beeing the median of
 *     all transforms. alpha and other fields are 0.
 * Preconditions:
 *     len>0
 * Side effects:
 *     None
 */
struct VSTransform median_xy_transform(const struct VSTransform* transforms, int len)
{
    VSTransform* ts = (VSTransform*)vs_malloc(sizeof(struct VSTransform) * len);
    VSTransform  t  = null_transform();
    memcpy(ts, transforms, sizeof(struct VSTransform) * len);
    int half = len / 2;
    qsort(ts, len, sizeof(struct VSTransform), cmp_trans_x);
    t.x = len % 2 == 0 ? ts[half].x : (ts[half].x + ts[half + 1].x) / 2;
    qsort(ts, len, sizeof(struct VSTransform), cmp_trans_y);
    t.y = len % 2 == 0 ? ts[half].y : (ts[half].y + ts[half + 1].y) / 2;
    vs_free(ts);
    return t;
}

/**
 * cleanmean_xy_transform: calulcates the cleaned mean of an array
 * of transforms, considering only x and y
 *
 * Parameters:
 *    transforms: array of transforms.
 *           len: length  of array
 * Return value:
 *     A new transform with x and y beeing the cleaned mean
 *     (meaning upper and lower pentile are removed) of
 *     all transforms. alpha and other fields are 0.
 * Preconditions:
 *     len>0
 * Side effects:
 *     None
 */
struct VSTransform cleanmean_xy_transform(const struct VSTransform* transforms, int len)
{
    VSTransform* ts = (VSTransform*)vs_malloc(sizeof(VSTransform) * len);
    VSTransform  t  = null_transform();
    int i, cut = len / 5;
    memcpy(ts, transforms, sizeof(struct VSTransform) * len);
    qsort(ts, len, sizeof(struct VSTransform), cmp_trans_x);
    for (i = cut; i < len - cut; i++)  // all but cutted
    {
        t.x += ts[i].x;
    }
    qsort(ts, len, sizeof(struct VSTransform), cmp_trans_y);
    for (i = cut; i < len - cut; i++)  // all but cutted
    {
        t.y += ts[i].y;
    }
    vs_free(ts);
    return mult_transform(&t, 1.0 / (len - (2.0 * cut)));
}


/**
 * calulcates the cleaned maximum and minimum of an array of transforms,
 * considerung only x and y
 * It cuts off the upper and lower x-th percentil
 *
 * Parameters:
 *    transforms: array of transforms.
 *           len: length  of array
 *     percentil: the x-th percentil to cut off
 *           min: pointer to min (return value)
 *           max: pointer to max (return value)
 * Return value:
 *     call by reference in min and max
 * Preconditions:
 *     len>0, 0<=percentil<50
 * Side effects:
 *     only on min and max
 */
void cleanmaxmin_xy_transform(const struct VSTransform* transforms, int len,
                              int percentil,
                              struct VSTransform* min, struct VSTransform* max)
{
    VSTransform* ts = (VSTransform*)vs_malloc(sizeof(VSTransform) * len);
    int cut = len * percentil / 100;
    memcpy(ts, transforms, sizeof(struct VSTransform) * len);
    qsort(ts, len, sizeof(struct VSTransform), cmp_trans_x);
    min->x = ts[cut].x;
    max->x = ts[len - cut - 1].x;
    qsort(ts, len, sizeof(struct VSTransform), cmp_trans_y);
    min->y = ts[cut].y;
    max->y = ts[len - cut - 1].y;
    vs_free(ts);
}

/* calculates the required zoom value to have no borders visible
 */
double transform_get_required_zoom(const struct VSTransform* transform, int width, int height)
{
    return 100.0 * (2.0 * VS_MAX(fabs(transform->x) / width, fabs(transform->y) / height) // translation part
                    + fabs(sin(transform->alpha)));          // rotation part
                    
}


/**
 * media: median of a double array
 *
 * Parameters:
 *            ds: array of values
 *           len: length  of array
 * Return value:
 *     the median value of the array
 * Preconditions: len>0
 * Side effects:  ds will be sorted!
 */
double median(double* ds, int len)
{
    int half = len / 2;
    qsort(ds, len, sizeof(double), cmp_double);
    return len % 2 == 0 ? ds[half] : (ds[half] + ds[half + 1]) / 2;
}


/** square of a number */
double sqr(double x)
{
    return x * x;
}

/**
 * mean: mean of a double array
 *
 * Parameters:
 *            ds: array of values
 *           len: length  of array
 * Return value: the mean value of the array
 * Preconditions: len>0
 * Side effects:  None
 */
double mean(const double* ds, int len)
{
    double sum = 0;
    int i = 0;
    for (i = 0; i < len; i++)
    {
        sum += ds[i];
    }
    return sum / len;
}

/**
 * stddev: standard deviation of a double array
 *
 * Parameters:
 *            ds: array of values
 *           len: length  of array
 *          mean: mean of the array (@see mean())
 * Return value: the standard deviation value of the array
 * Preconditions: len>0
 * Side effects:  None
 */
double stddev(const double* ds, int len, double mean)
{
    double sum = 0;
    int i = 0;
    for (i = 0; i < len; i++)
    {
        sum += sqr(ds[i] - mean);
    }
    return sqrt(sum / len);
}

/**
 * cleanmean: mean with cutted upper and lower pentile
 *
 * Parameters:
 *            ds: array of values
 *           len: length  of array
 *       minimum: minimal value (after cleaning) if not NULL
 *       maximum: maximal value (after cleaning) if not NULL
 * Return value:
 *     the mean value of the array without the upper
 *     and lower pentile (20% each)
 *     and minimum and maximum without the pentiles
 * Preconditions: len>0
 * Side effects:  ds will be sorted!
 */
double cleanmean(double* ds, int len, double* minimum, double* maximum)
{
    int cut    = len / 5;
    double sum = 0;
    int i      = 0;
    qsort(ds, len, sizeof(double), cmp_double);
    for (i = cut; i < len - cut; i++)   // all but first and last
    {
        sum += ds[i];
    }
    if (minimum)
    {
        *minimum = ds[cut];
    }
    if (maximum)
    {
        *maximum = ds[len - cut - 1];
    }
    return sum / (len - (2.0 * cut));
}

/************************************************/
/***************LOCALMOTION**********************/

LocalMotion null_localmotion()
{
    LocalMotion lm;
    memset(&lm, 0, sizeof(lm));
    return lm;
}

int* localmotions_getx(const LocalMotions* localmotionsC)
{
    const VidStab::LmList localmotions { *const_cast<LocalMotions*>(localmotionsC) };
    int len = localmotions.size();
    int* xs = (int*)vs_malloc(sizeof(int) * len);
    int i;
    for (i = 0; i < len; i++)
    {
        xs[i] = LMGet(localmotionsC, i)->v.x;
    }
    return xs;
}

int* localmotions_gety(const LocalMotions* localmotionsC)
{
    const VidStab::LmList localmotions { *const_cast<LocalMotions*>(localmotionsC) };
    int len = localmotions.size();
    int* ys = (int*)vs_malloc(sizeof(int) * len);
    int i;
    for (i = 0; i < len; i++)
    {
        ys[i] = LMGet(localmotionsC, i)->v.y;
    }
    return ys;
}

LocalMotion sub_localmotion(const LocalMotion* lm1, const LocalMotion* lm2)
{
    LocalMotion res = *lm1;
    res.v.x -= lm2->v.x;
    res.v.y -= lm2->v.y;
    return res;
}


/**
 * cleanmean_localmotions: calulcates the cleaned mean of a vector
 * of local motions considering
 *
 * Parameters:
 *    localmotions : vs_vector of local motions
 * Return value:
 *     A localmotion with vec with x and y being the cleaned mean
 *     (meaning upper and lower pentile are removed) of
 *     all local motions. all other fields are 0.
 * Preconditions:
 *     size of vector >0
 * Side effects:
 *     None
 */
LocalMotion cleanmean_localmotions(const LocalMotions* localmotionsC)
{
    const VidStab::LmList localmotions { *const_cast<LocalMotions*>(localmotionsC) };
    int len = localmotions.size();
    int i, cut = len / 5;
    int* xs = localmotions_getx(localmotionsC);
    int* ys = localmotions_gety(localmotionsC);
    LocalMotion m = null_localmotion();
    m.v.x = 0;
    m.v.y = 0;
    qsort(xs, len, sizeof(int), cmp_int);
    for (i = cut; i < len - cut; i++)  // all but cutted
    {
        m.v.x += xs[i];
    }
    qsort(ys, len, sizeof(int), cmp_int);
    for (i = cut; i < len - cut; i++)  // all but cutted
    {
        m.v.y += ys[i];
    }
    vs_free(xs);
    vs_free(ys);
    m.v.x /= (len - (2.0 * cut));
    m.v.y /= (len - (2.0 * cut));
    return m;
}

VSArray localmotionsGetMatch(const LocalMotions* localmotionsC)
{
    const VidStab::LmList localmotions { *const_cast<LocalMotions*>(localmotionsC) };
    VSArray m = vs_array_new(localmotions.size());
    for (int i = 0; i < m.len; i++)
    {
        m.dat[i] = LMGet(localmotionsC, i)->match;
    }
    return m;
}
