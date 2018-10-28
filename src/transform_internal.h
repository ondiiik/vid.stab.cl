#pragma once
/*
 *  transform_internal.h
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
#include "gimbal_corrector.h"
#include "transformfixedpoint.h"


#ifdef __cplusplus
extern "C" {
#endif


/** performs the smoothing of the camera path and modifies the transforms
    to compensate for the jiggle
    */
int cameraPathOptimization(struct VSTransformData* td, struct VSTransformations* trans);

int cameraPathAvg(struct VSTransformData* td, struct VSTransformations* trans);
int cameraPathGaussian(struct VSTransformData* td, struct VSTransformations* trans);
int cameraPathOptimalL1(struct VSTransformData* td, struct VSTransformations* trans);


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
