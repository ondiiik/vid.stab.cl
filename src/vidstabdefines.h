#pragma once
/*
 * vidstabdefines.h
 *
 *  Created on: Feb 23, 2011
 *      Author: georg
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
#include <cstddef>
#include <cstdlib>


#ifdef __GNUC__
#define likely(x)       __builtin_expect(!!(x), 1)
#define unlikely(x)     __builtin_expect(!!(x), 0)
#else
#define likely(x)        (x)
#define unlikely(x)      (x)
#endif

#define VS_MAX(a, b)    (((a) > (b)) ?(a) :(b))
#define VS_MIN(a, b)    (((a) < (b)) ?(a) :(b))
/* clamp x between a and b */
#define VS_CLAMP(x, a, b)  VS_MIN(VS_MAX((a), (x)), (b))

#define VS_DEBUG 2

/// pixel in single layer image
#define PIXEL(img, linesize, x, y, w, h, def) \
    (((x) < 0 || (y) < 0 || (x) >= (w) || (y) >= (h)) ? (def) : img[unsigned(x) + unsigned(y) * (linesize)])
/// pixel in single layer image without rangecheck
#define PIX(img, linesize, x, y) (img[(x) + (y) * (linesize)])
/// pixel in N-channel image. channel in {0..N-1}
#define PIXELN(img, linesize, x, y, w, h, N, channel, def) \
    (((x) < 0 || (y) < 0 || (x) >= (w) || (y) >= (h)) ? (def) : img[((x) + (y) * (linesize))*(N) + (channel)])
/// pixel in N-channel image without rangecheck. channel in {0..N-1}
#define PIXN(img, linesize, x, y, N, channel) (img[((x) + (y) * (linesize))*(N) + (channel)])

/**** Configurable memory and logging functions. Defined in libvidstab.cpp ****/


#define vs_log_error(tag, format, args...) \
    vs_log(VS_ERROR_TYPE, tag, format , ## args)
#define vs_log_warn(tag, format, args...) \
    vs_log(VS_WARN_TYPE, tag, format , ## args)
#define vs_log_info(tag, format, args...) \
    vs_log(VS_INFO_TYPE, tag, format , ## args)
#define vs_log_msg(tag, format, args...) \
    vs_log(VS_MSG_TYPE, tag, format , ## args)
