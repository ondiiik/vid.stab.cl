#pragma once
/*
 * serialize.h
 *
 *  Copyright (C) Georg Martius - January 2013
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
#include "gimbal_detector.h"
#include "transformtype.h"
#include "transform.h"


#ifdef __cplusplus
extern "C" {
#endif


/// helper macro to access a localmotions vector in the VSVector of all Frames
#define VSMLMGet(manylocalmotions,index) \
    ((LocalMotions*)vs_vector_get(manylocalmotions,index))


/// stores local motions to file
int vsStoreLocalmotions(FILE* f, const LocalMotions* lms);

/// restores local motions from file
LocalMotions vsRestoreLocalmotions(FILE* f);

/// reads the header of the file and return the version number (used by readLocalmotionsFile)
int vsReadFileVersion(FILE* f);

/*
 * reads the next set of localmotions from the file, return VS_ERROR on error or
 * if nothing is read (used by readLocalmotionsFile)
 */
int vsReadFromFile(FILE* f, LocalMotions* lms);


#ifdef __cplusplus
}
#endif
