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
#include <stddef.h>
#include <stdlib.h>
#include <inttypes.h>


#ifdef __cplusplus
extern "C" {
#endif


// use it to calculate the CHROMA sizes (rounding is correct)
#define CHROMA_SIZE(width,log2sub)  (-(-(width) >> (log2sub)))


/// returns the subsampling shift amount, horizonatally for the given plane
int vsGetPlaneWidthSubS(const VSFrameInfo* fi, int plane);


/// returns the subsampling shift amount, vertically for the given plane
int vsGetPlaneHeightSubS(const VSFrameInfo* fi, int plane);


/// zero initialization
void vsFrameNull(VSFrame* frame);


/// returns true if frame is null (data[0]==0)
int vsFrameIsNull(const VSFrame* frame);


/// compares two frames for identity (based in data[0])
int vsFramesEqual(const VSFrame* frame1, const VSFrame* frame2);


/// allocates memory for a frame
void vsFrameAllocate(VSFrame*           frame,
                     const VSFrameInfo* fi);


/// copies the given plane number from src to dest
void vsFrameCopyPlane(VSFrame* dest, const VSFrame* src,
                      const VSFrameInfo* fi, int plane);


/// copies src to dest
void vsFrameCopy(VSFrame* dest, const VSFrame* src, const VSFrameInfo* fi);


/** fills the data pointer so that it corresponds to the img saved in the linear buffer.
    No copying is performed.
    Do not call vsFrameFree() on it.
 */
void vsFrameFillFromBuffer(VSFrame* frame, uint8_t* img, const VSFrameInfo* fi);


/// frees memory
void vsFrameFree(VSFrame* frame);


#ifdef __cplusplus
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
