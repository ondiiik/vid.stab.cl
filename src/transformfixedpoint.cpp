/*
 *  transformfixedpoint.cpp
 *
 *  Fixed point implementation of image transformations (see also transformfloat.cpp/h)
 *
 *  Copyright (C) Georg Martius - June 2011
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
 *
 */
#include "transformfixedpoint.h"
#include "transform.h"
#include "transformtype_operations.h"

// the orc code does not work at the moment (BUG in ORC?)
// #include "orc/transformorc.h"

//#include <math.h>
//#include <libgen.h>

#define iToFp8(v)  ((v)<<8)
#define fToFp8(v)  ((int32_t)((v)*((float)0xFF)))
#define iToFp16(v) ((v)<<16)
#define fToFp16(v) ((int32_t)((v)*((double)0xFFFF)))
#define fp16To8(v) ((v)>>8)
//#define fp16To8(v) ( (v) && 0x80 == 1 ? ((v)>>8 + 1) : ((v)>>8) )
#define fp24To8(v) ((v)>>16)

#define fp8ToI(v)  ((v)>>8)
#define fp16ToI(v) ((v)>>16)
#define fp8ToF(v)  ((v)/((double)(1<<8)))
#define fp16ToF(v) ((v)/((double)(1<<16)))

// #define fp8ToIRound(v) ( (((v)>>7) & 0x1) == 0 ? ((v)>>8) : ((v)>>8)+1 )
#define fp8_0_5 (1<<7)
#define fp8ToIRound(v) (((v) + fp8_0_5) >> 7)
//#define fp16ToIRound(v) ( (((v)>>15) & 0x1) == 0 ? ((v)>>16) : ((v)>>16)+1 )
#define fp16_0_5 (1<<15)
#define fp16ToIRound(v) (((v) + fp16_0_5) >> 16)


/** taken from http://en.wikipedia.org/wiki/Bicubic_interpolation for alpha=-0.5
    in matrix notation:
    a0-a3 are the neigthboring points where the target point is between a1 and a2
    t is the point of interpolation (position between a1 and a2) value between 0 and 1
    | 0, 2, 0, 0 |  |a0|
    |-1, 0, 1, 0 |  |a1|
    (1,t,t^2,t^3) | 2,-5, 4,-1 |  |a2|
    |-1, 3,-3, 1 |  |a3|
*/
/* inline static short bicub_kernel(fp16 t, short a0, short a1, short a2, short a3){ */
/*   // (2*a1 + t*((-a0+a2) + t*((2*a0-5*a1+4*a2-a3) + t*(-a0+3*a1-3*a2+a3) )) ) / 2; */
/*   return ((iToFp16(2*a1) + t*(-a0+a2 */
/*             + fp16ToI(t*((2*a0-5*a1+4*a2-a3) */
/*              + fp16ToI(t*(-a0+3*a1-3*a2+a3)) )) ) */
/*      ) ) >> 17; */
/* } */

inline static int bicub_kernel(float t,
                               int   a0,
                               int   a1,
                               int   a2,
                               int   a3)
{
    return int(((float(2 * a1) + t * (-a0 + a2 +
                                      int((t * ((2 * a0 - 5 * a1 + 4 * a2 - a3) +
                                                int((t * (-a0 + 3 * a1 - 3 * a2 + a3)) + 0.5) )) + 0.5) )) / 2) + 0.5);
}

/** interpolateBiCub: bi-cubic interpolation function using 4x4 pixel, see interpolate */
void interpolateBiCub(uint8_t*       rv,
                      float          x,
                      float          y,
                      const uint8_t* img,
                      int            img_linesize,
                      int            width,
                      int            height,
                      uint8_t        def)
{
    // do a simple linear interpolation at the border
    int ix_f = x;
    int iy_f = y;
    
    if (unlikely((ix_f < 1          ) ||
                 (ix_f > (width - 3)) ||
                 (iy_f < 1          ) ||
                 (iy_f > (height - 3))))
    {
        interpolateBiLinBorder(rv,
                               x,
                               y,
                               img,
                               img_linesize,
                               width,
                               height,
                               def);
    }
    else
    {
        float x_f = float(ix_f);
        float y_f = float(iy_f);
        float tx  = x - x_f;
        
        int v1 = bicub_kernel(tx,
                              PIX(img, img_linesize, ix_f - 1, iy_f - 1),
                              PIX(img, img_linesize, ix_f,     iy_f - 1),
                              PIX(img, img_linesize, ix_f + 1, iy_f - 1),
                              PIX(img, img_linesize, ix_f + 2, iy_f - 1));
                              
        int v2 = bicub_kernel(tx,
                              PIX(img, img_linesize, ix_f - 1, iy_f),
                              PIX(img, img_linesize, ix_f,     iy_f),
                              PIX(img, img_linesize, ix_f + 1, iy_f),
                              PIX(img, img_linesize, ix_f + 2, iy_f));
                              
        int v3 = bicub_kernel(tx,
                              PIX(img, img_linesize, ix_f - 1, iy_f + 1),
                              PIX(img, img_linesize, ix_f,     iy_f + 1),
                              PIX(img, img_linesize, ix_f + 1, iy_f + 1),
                              PIX(img, img_linesize, ix_f + 2, iy_f + 1));
                              
        int v4 = bicub_kernel(tx,
                              PIX(img, img_linesize, ix_f - 1, iy_f + 2),
                              PIX(img, img_linesize, ix_f,     iy_f + 2),
                              PIX(img, img_linesize, ix_f + 1, iy_f + 2),
                              PIX(img, img_linesize, ix_f + 2, iy_f + 2));
                              
        int res = bicub_kernel(y - y_f, v1, v2, v3, v4);
        
        *rv = (res >= 0) ? ((res < 255) ? res : 255) : 0;
    }
}


/** interpolateLin: linear (only x) interpolation function, see interpolate */
void interpolateLin(uint8_t*       rv,
                    float          x,
                    float          y,
                    const uint8_t* img,
                    int            img_linesize,
                    int            width,
                    int            height,
                    uint8_t        def)
{
    int   ix_f = int(x);
    int   ix_c = ix_f + 1;
    float x_c  = float(ix_c);
    float x_f  = float(ix_f);
    int   y_n  = int(y + 0.5);
    
    int v1 = PIXEL(img,
                   img_linesize,
                   ix_c,
                   y_n,
                   width,
                   height,
                   def);
                   
    int v2 = PIXEL(img,
                   img_linesize,
                   ix_f,
                   y_n,
                   width,
                   height,
                   def);
                   
    float s   = v1 * (x - x_f) + v2 * (x_c - x);
    int   res = int(s);
    *rv       = (res >= 0) ? ((res < 255) ? res : 255) : 0;
}


/** interpolateZero: nearest neighbor interpolation function, see interpolate */
void interpolateZero(uint8_t*       rv,
                     float          x,
                     float          y,
                     const uint8_t* img,
                     int            img_linesize,
                     int            width,
                     int            height,
                     uint8_t        def)
{
    int res = PIXEL(img,
                    img_linesize,
                    x + 0.5,
                    y + 0.5,
                    width,
                    height,
                    def);
                    
    *rv = (res >= 0) ? ((res < 255) ? res : 255) : 0;
}


/**
 * interpolateN: Bi-linear interpolation function for N channel image.
 *
 * Parameters:
 *             rv: destination pixel (call by reference)
 *            x,y: the source coordinates in the image img. Note this
 *                 are real-value coordinates, that's why we interpolate
 *            img: source image
 *   width,height: dimension of image
 *              N: number of channels
 *        channel: channel number (0..N-1)
 *            def: default value if coordinates are out of range
 * Return value:  None
 */
void interpolateN(uint8_t*       rv,
                  float          x,
                  float          y,
                  const uint8_t* img,
                  int            img_linesize,
                  int            width,
                  int            height,
                  uint8_t        N,
                  uint8_t        channel,
                  uint8_t        def)
{
    int ix_f = int(x);
    int iy_f = int(y);
    
    if ((ix_f < 0          ) ||
        (ix_f > (width - 1)) ||
        (iy_f < 0          ) ||
        (iy_f > (height - 1)))
    {
        *rv = def;
    }
    else
    {
        int   ix_c = ix_f + 1;
        int   iy_c = iy_f + 1;
        int   v1   = PIXN(img, img_linesize, ix_c, iy_c, N, channel);
        int   v2   = PIXN(img, img_linesize, ix_c, iy_f, N, channel);
        int   v3   = PIXN(img, img_linesize, ix_f, iy_c, N, channel);
        int   v4   = PIXN(img, img_linesize, ix_f, iy_f, N, channel);
        float x_f  = iToFp16(ix_f) / 256;
        float x_c  = iToFp16(ix_c) / 256;
        float y_f  = iToFp16(iy_f) / 256;
        float y_c  = iToFp16(iy_c) / 256;
        float s    = float(v1 * (x - x_f) + v3 * (x_c - x)) * float(y - y_f) +
                     float(v2 * (x - x_f) + v4 * (x_c - x)) * float(y_c - y);
        int res = int(s + 0.5);
        *rv = (res >= 0) ? ((res < 255) ? res : 255) : 0;
    }
}


/**
 * transformPacked: applies current transformation to frame
 * Parameters:
 *         td: private data structure of this filter
 * Return value:
 *         0 for failture, 1 for success
 * Preconditions:
 *  The frame must be in Packed format
 */
int transformPacked(struct VSTransformData* td, struct VSTransform t)
{
    int x = 0, y = 0, k = 0;
    uint8_t* D_1, *D_2;
    
    D_1  = td->src.data[0];
    D_2  = td->destbuf.data[0];
    fp16 c_s_x = iToFp16(td->fiSrc.width / 2);
    fp16 c_s_y = iToFp16(td->fiSrc.height / 2);
    int32_t c_d_x = td->fiDest.width / 2;
    int32_t c_d_y = td->fiDest.height / 2;
    
    /* for each pixel in the destination image we calc the source
     * coordinate and make an interpolation:
     *      p_d = c_d + M(p_s - c_s) + t
     * where p are the points, c the center coordinate,
     *  _s source and _d destination,
     *  t the translation, and M the rotation matrix
     *      p_s = M^{-1}(p_d - c_d - t) + c_s
     */
    float z     = 1.0 - t.zoom / 100.0;
    fp16 zcos_a = fToFp16(z * cos(-t.alpha)); // scaled cos
    fp16 zsin_a = fToFp16(z * sin(-t.alpha)); // scaled sin
    fp16  c_tx    = c_s_x - fToFp16(t.x);
    fp16  c_ty    = c_s_y - fToFp16(t.y);
    int channels = td->fiSrc.bytesPerPixel;
    /* All channels */
    for (y = 0; y < td->fiDest.height; y++)
    {
        int32_t y_d1 = (y - c_d_y);
        for (x = 0; x < td->fiDest.width; x++)
        {
            int32_t x_d1 = (x - c_d_x);
            fp16 x_s  =  zcos_a * x_d1 + zsin_a * y_d1 + c_tx;
            fp16 y_s  = -zsin_a * x_d1 + zcos_a * y_d1 + c_ty;
            
            for (k = 0; k < channels; k++)   // iterate over colors
            {
                uint8_t* dest = &D_2[x + y * td->destbuf.linesize[0] + k];
                interpolateN(dest, x_s, y_s, D_1, td->src.linesize[0],
                             td->fiSrc.width, td->fiSrc.height,
                             channels, k, td->conf.crop ? 16 : *dest);
            }
        }
    }
    return VS_OK;
}

/**
 * transformPlanar: applies current transformation to frame
 *
 * Parameters:
 *         td: private data structure of this filter
 * Return value:
 *         0 for failture, 1 for success
 * Preconditions:
 *  The frame must be in Planar format
 *
 * Fixed-point format 32 bit integer:
 *  for image coords we use val<<8
 *  for angle and zoom we use val<<16
 *
 */
int transformPlanar(VSTransformData* td,
                    VSTransform      t)
{
    int32_t x = 0, y = 0;
    uint8_t* dat_1, *dat_2;
    
    if (t.alpha == 0 && t.x == 0 && t.y == 0 && t.zoom == 0)
    {
        Frame::Frame fsrc { td->src,     td->fiSrc  };
        Frame::Frame fdst { td->destbuf, td->fiDest };
        
        if (fsrc == fdst)
        {
            return VS_OK;    // noop
        }
        else
        {
            Frame::Frame fdst { td->destbuf, td->fiDest };
            Frame::Frame fsrc { td->src,     td->fiSrc  };
            fdst = fsrc;
            return VS_OK;
        }
    }
    
    for (int plane = 0; plane < td->fiSrc.planes; plane++)
    {
        dat_1  = td->src.data[plane];
        dat_2  = td->destbuf.data[plane];
        
        Frame::Info finf { td->fiSrc };
        int wsub =  finf.subsampleWidth( plane);
        int hsub =  finf.subsampleHeight(plane);
        
        int dw = CHROMA_SIZE(td->fiDest.width, wsub);
        int dh = CHROMA_SIZE(td->fiDest.height, hsub);
        int sw = CHROMA_SIZE(td->fiSrc.width, wsub);
        int sh = CHROMA_SIZE(td->fiSrc.height, hsub);
        uint8_t black = plane == 0 ? 0 : 0x80;
        
        fp16 c_s_x = iToFp16(sw / 2);
        fp16 c_s_y = iToFp16(sh / 2);
        int32_t c_d_x = dw / 2;
        int32_t c_d_y = dh / 2;
        
        float z     = 1.0 - t.zoom / 100.0;
        fp16 zcos_a = fToFp16(z * cos(-t.alpha)); // scaled cos
        fp16 zsin_a = fToFp16(z * sin(-t.alpha)); // scaled sin
        fp16  c_tx    = c_s_x - (fToFp16(t.x) >> wsub);
        fp16  c_ty    = c_s_y - (fToFp16(t.y) >> hsub);
        
        /* for each pixel in the destination image we calc the source
         * coordinate and make an interpolation:
         *      p_d = c_d + M(p_s - c_s) + t
         * where p are the points, c the center coordinate,
         *  _s source and _d destination,
         *  t the translation, and M the rotation and scaling matrix
         *      p_s = M^{-1}(p_d - c_d - t) + c_s
         */
        for (y = 0; y < dh; y++)
        {
            // swapping of the loops brought 15% performace gain
            int32_t y_d1 = (y - c_d_y);
            for (x = 0; x < dw; x++)
            {
                int32_t x_d1 = (x - c_d_x);
                fp16 x_s  =  zcos_a * x_d1 + zsin_a * y_d1 + c_tx;
                fp16 y_s  = -zsin_a * x_d1 + zcos_a * y_d1 + c_ty;
                uint8_t* dest = &dat_2[x + y * td->destbuf.linesize[plane]];
                // inlining the interpolation function would bring 10%
                //  (but then we cannot use the function pointer anymore...)
                td->interpolate(dest, x_s, y_s, dat_1,
                                td->src.linesize[plane], sw, sh,
                                td->conf.crop ? black : *dest);
            }
        }
    }
    
    return VS_OK;
}
