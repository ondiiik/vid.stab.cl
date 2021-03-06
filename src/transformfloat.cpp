/*
 *  transformfloat.cpp
 *
 *  Floating point image transformations
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
 */
#include "transformfloat.h"

#include "gimbal_corrector.h"
#include "transformtype_operations.h"


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
void interpolateN(uint8_t* rv, float x, float y,
                        const uint8_t* img, int img_linesize,
                        int width, int height,
                        uint8_t N, uint8_t channel,
                        uint8_t def)
{
    if (x < - 1 || x > width || y < -1 || y > height)
    {
        *rv = def;
    }
    else
    {
        int x_f = myfloor(x);
        int x_c = x_f + 1;
        int y_f = myfloor(y);
        int y_c = y_f + 1;
        short v1 = PIXELN(img, img_linesize, x_c, y_c, width, height, N, channel, def);
        short v2 = PIXELN(img, img_linesize, x_c, y_f, width, height, N, channel, def);
        short v3 = PIXELN(img, img_linesize, x_f, y_c, width, height, N, channel, def);
        short v4 = PIXELN(img, img_linesize, x_f, y_f, width, height, N, channel, def);
        float s  = (v1 * (x - x_f) + v3 * (x_c - x)) * (y - y_f) +
                   (v2 * (x - x_f) + v4 * (x_c - x)) * (y_c - y);
        int32_t res = (int32_t)s;
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
 /// TODO Add zoom!
 /// Add bytes per pixel usage
 */
int transformPacked(struct VSTransformData* td, struct VSTransform t)
{
    int x = 0, y = 0, z = 0;
    uint8_t* D_1, *D_2;
    char crop = td->conf.crop;
    
    D_1  = td->src.data[0];
    D_2  = td->destbuf.data[0];
    float c_s_x = td->fiSrc.width / 2.0;
    float c_s_y = td->fiSrc.height / 2.0;
    float c_d_x = td->fiDest.width / 2.0;
    float c_d_y = td->fiDest.height / 2.0;
    
    /* for each pixel in the destination image we calc the source
     * coordinate and make an interpolation:
     *      p_d = c_d + M(p_s - c_s) + t
     * where p are the points, c the center coordinate,
     *  _s source and _d destination,
     *  t the translation, and M the rotation matrix
     *      p_s = M^{-1}(p_d - c_d - t) + c_s
     */
    int channels = td->fiSrc.bytesPerPixel;
    /* All channels */
    if (fabs(t.alpha) > 0.1 * M_PI / 180.0) // 0.1 deg
    {
        for (x = 0; x < td->fiDest.width; x++)
        {
            for (y = 0; y < td->fiDest.height; y++)
            {
                float x_d1 = (x - c_d_x);
                float y_d1 = (y - c_d_y);
                float x_s  =  cos(-t.alpha) * x_d1
                              + sin(-t.alpha) * y_d1 + c_s_x - t.x;
                float y_s  = -sin(-t.alpha) * x_d1
                             + cos(-t.alpha) * y_d1 + c_s_y - t.y;
                for (z = 0; z < channels; z++)   // iterate over colors
                {
                    uint8_t* dest = &D_2[x + y * td->destbuf.linesize[0] + z];
                    interpolateN(dest, x_s, y_s, D_1, td->src.linesize[0],
                                       td->fiSrc.width, td->fiSrc.height,
                                       channels, z, crop ? 16 : *dest);
                }
            }
        }
    }
    else
    {
        /* no rotation, just translation
         *(also no interpolation, since no size change (so far)
         */
        int round_tx = myround(t.x);
        int round_ty = myround(t.y);
        for (x = 0; x < td->fiDest.width; x++)
        {
            for (y = 0; y < td->fiDest.height; y++)
            {
                for (z = 0; z < channels; z++)   // iterate over colors
                {
                    short p = PIXELN(D_1, td->src.linesize[0], x - round_tx, y - round_ty,
                                     td->fiSrc.width, td->fiSrc.height, channels, z, -1);
                    if (p == -1)
                    {
                        if (crop == 1)
                        {
                            D_2[(x + y * td->destbuf.linesize[0])*channels + z] = 16;
                        }
                    }
                    else
                    {
                        D_2[(x + y * td->destbuf.linesize[0])*channels + z] = (uint8_t)p;
                    }
                }
            }
        }
    }
    return 1;
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
 */
int transformPlanar(struct VSTransformData* td, struct VSTransform t)
{
    int x = 0, y = 0;
    uint8_t* dat_1, *dat_2;
    char crop = td->conf.crop;
    
    if (t.alpha == 0 && t.x == 0 && t.y == 0 && t.zoom == 0)
    {
        if (vsFramesEqual(&td->src, &td->destbuf))
        {
            return VS_OK;    // noop
        }
        else
        {
            vsFrameCopy(&td->destbuf, &td->src, &td->fiSrc);
            return VS_OK;
        }
    }
    int plane;
    for (plane = 0; plane < td->fiSrc.planes; plane++)
    {
        dat_1  = td->src.data[plane];
        dat_2  = td->destbuf.data[plane];
        
        int wsub = vsGetPlaneWidthSubS(&td->fiSrc, plane);
        int hsub = vsGetPlaneHeightSubS(&td->fiSrc, plane);
        float c_s_x = (td->fiSrc.width  >> wsub) / 2.0;
        float c_s_y = (td->fiSrc.height >> hsub) / 2.0;
        float c_d_x = (td->fiDest.width >> wsub) / 2.0;
        float c_d_y = (td->fiDest.height >> hsub) / 2.0;
        uint8_t black = plane == 0 ? 0 : 0x80;
        
        float z = 1.0 - t.zoom / 100;
        float zcos_a = z * cos(-t.alpha); // scaled cos
        float zsin_a = z * sin(-t.alpha); // scaled sin
        float tx = t.x / (float)(1 << wsub);
        float ty = t.y / (float)(1 << hsub);
        
        /* for each pixel in the destination image we calc the source
         * coordinate and make an interpolation:
         *      p_d = c_d + M(p_s - c_s) + t
         * where p are the points, c the center coordinate,
         *  _s source and _d destination,
         *  t the translation, and M the rotation and scaling matrix
         *      p_s = M^{-1}(p_d - c_d - t) + c_s
         */
        int w = CHROMA_SIZE(td->fiDest.width, wsub);
        int h = CHROMA_SIZE(td->fiDest.height, hsub);
        int sw = CHROMA_SIZE(td->fiSrc.width, wsub);
        int sh = CHROMA_SIZE(td->fiSrc.height, hsub);
        for (x = 0; x < w; x++)
        {
            for (y = 0; y < h; y++)
            {
                float x_d1 = (x - c_d_x);
                float y_d1 = (y - c_d_y);
                float x_s  =  zcos_a * x_d1
                              + zsin_a * y_d1 + c_s_x - tx;
                float y_s  = -zsin_a * x_d1
                             + zcos_a * y_d1 + c_s_y - ty;
                uint8_t* dest = &dat_2[x + y * td->destbuf.linesize[plane]];
                td->interpolate(dest, x_s, y_s, dat_1, td->src.linesize[plane],
                                      sw, sh, crop ? black : *dest);
            }
        }
    }
    return VS_OK;
}

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
