/*
 * serialize.cpp
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

#include <assert.h>
#include <string.h>

#include "serialize.h"

#include "gimbal_detector.h"
#include "transformtype.h"
#include "transformtype_operations.h"


/*
 * C++ includes
 */

using namespace Gimbal;


const char* modname = "vid.stab - serialization";


int storeLocalmotion(FILE* f, const LocalMotion* lm)
{
    return fprintf(f, "(LM %i %i %i %i %i %lf %lf)", lm->v.x, lm->v.y, lm->f.x, lm->f.y, lm->f.size,
                   lm->contrast, lm->match);
}

/// restore local motion from file
LocalMotion restoreLocalmotion(FILE* f)
{
    LocalMotion lm;
    char c;
    if (fscanf(f, "(LM %i %i %i %i %i %lf %lf", &lm.v.x, &lm.v.y, &lm.f.x, &lm.f.y, &lm.f.size,
               &lm.contrast, &lm.match) != 7)
    {
        vs_log_error(modname, "Cannot parse localmotion!\n");
        return null_localmotion();
    }
    while ((c = fgetc(f)) && c != ')' && c != EOF);
    if (c == EOF)
    {
        vs_log_error(modname, "Cannot parse localmotion missing ')'!\n");
        return null_localmotion();
    }
    return lm;
}

int vsStoreLocalmotions(FILE*               f,
                        const LocalMotions* lmsC)
{
    const Gimbal::LmList lms { *const_cast<LocalMotions*>(lmsC) };
    int                   len { lms.size()                       };
    
    fprintf(f, "List %i [", len);
    
    for (int i = 0; i < len; i++)
    {
        if (i > 0)
        {
            fprintf(f, ",");
        }
        
        if (storeLocalmotion(f, LMGet(lmsC, i)) <= 0)
        {
            return 0;
        }
    }
    
    fprintf(f, "]");
    
    return 1;
}

/// restores local motions from file
LocalMotions vsRestoreLocalmotions(FILE* f)
{
    LocalMotions          lmsC;
    Gimbal::LmList lms { lmsC };
    int i;
    char c;
    int len;
    
    lms.init(0);
    
    if (fscanf(f, "List %i [", &len) != 1)
    {
        vs_log_error(modname, "Cannot parse localmotions list expect 'List len ['!\n");
        return lmsC;
    }
    
    if (len > 0)
    {
        lms.init(len);
        
        for (i = 0; i < len; i++)
        {
            if (i > 0) while ((c = fgetc(f)) && c != ',' && c != EOF);
            LocalMotion lm = restoreLocalmotion(f);
            vs_vector_append_dup(&lmsC, &lm, sizeof(LocalMotion));
        }
    }
    
    if (len != lms.size())
    {
        vs_log_error(modname, "Cannot parse the given number of localmotions!\n");
        return lmsC;
    }
    
    while ((c = fgetc(f)) && c != ']' && c != EOF);
    
    if (c == EOF)
    {
        vs_log_error(modname, "Cannot parse localmotions list missing ']'!\n");
        return lmsC;
    }
    
    return lmsC;
}


/// reads the header of the file and return the version number
int vsReadFileVersion(FILE* f)
{
    if (!f)
    {
        return VS_ERROR;
    }
    int version;
    if (fscanf(f, "VID.STAB %i\n", &version) != 1)
    {
        return VS_ERROR;
    }
    else
    {
        return version;
    }
}

int vsReadFromFile(FILE* f, LocalMotions* lms)
{
    char c = fgetc(f);
    if (c == 'F')
    {
        int num;
        if (fscanf(f, "rame %i (", &num) != 1)
        {
            vs_log_error(modname, "cannot read file, expect 'Frame num (...'");
            return VS_ERROR;
        }
        *lms = vsRestoreLocalmotions(f);
        if (fscanf(f, ")\n") < 0)
        {
            vs_log_error(modname, "cannot read file, expect '...)'");
            return VS_ERROR;
        }
        return num;
    }
    else if (c == '#')
    {
        char l[1024];
        if (fgets(l, sizeof(l), f) == 0)
        {
            return VS_ERROR;
        }
        return vsReadFromFile(f, lms);
    }
    else if (c == '\n' || c == ' ')
    {
        return vsReadFromFile(f, lms);
    }
    else if (c == EOF)
    {
        return VS_ERROR;
    }
    else
    {
        vs_log_error(modname, "cannot read frame local motions from file, got %c (%i)",
                     c, (int) c);
        return VS_ERROR;
    }
}

int vsReadLocalMotionsFile(FILE* f, VSManyLocalMotions* mlmsC)
{
    return VS_OK;
}


/**
 * vsReadOldTransforms: read transforms file (Deprecated format)
 *  The format is as follows:
 *   Lines with # at the beginning are comments and will be ignored
 *   Data lines have 5 columns seperated by space or tab containing
 *   time, x-translation, y-translation, alpha-rotation, extra
 *   where time and extra are integers
 *   and the latter is unused at the moment
 *
 * Parameters:
 *         f:  file description
 *         trans: place to store the transforms
 * Return value:
 *         number of transforms read
 * Preconditions: f is opened
 */
int vsReadOldTransforms(const struct VSTransformData* td, FILE* f, struct VSTransformations* trans)
{
    return 0;
}
