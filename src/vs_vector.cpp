/*
 * dcvector.c -- a dynamic array
 * (C) 2011 - Georg Martius
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
 */

#include "vidstabdefines.h"
#include <assert.h>
#include <string.h>
#include "vs_vector.h"


/*************************************************************************/
int vs_vector_resize(_VSVector* V, int newsize);

/*************************************************************************/

int vs_vector_init(_VSVector* V, int buffersize)
{
    assert(V);

    if (buffersize > 0)
    {
        V->_data = (void**)vs_zalloc(sizeof(void*)*buffersize);
        if (!V->_data)
        {
            return VS_ERROR;
        }
    }
    else
    {
        V->_data = 0;
        buffersize = 0;
    }

    V->_buffersize = buffersize;
    V->_nelems = 0;
    return VS_OK;
}

int vs_vector_fini(_VSVector* V)
{
    assert(V);
    if (V->_data)
    {
        vs_free(V->_data);
    }
    V->_data = 0;
    V->_buffersize = 0;
    V->_nelems = 0;
    return VS_OK;
}

int vs_vector_del(_VSVector* V)
{
    vs_vector_zero(V);
    return vs_vector_fini(V);
}

int vs_vector_zero(_VSVector* V)
{
    assert(V);
    assert(V->_nelems < 1 || V->_data);
    int i;
    for (i = 0; i < V->_nelems; i++)
    {
        if (V->_data[i])
        {
            vs_free(V->_data[i]);
        }
    }
    V->_nelems = 0;
    return VS_OK;
    
}

int vs_vector_size(const _VSVector* V)
{
    assert(V);
    return V->_nelems;
}


int vs_vector_append(_VSVector* V, void* data)
{
    assert(V && data);
    if (!V->_data || V->_buffersize < 1)
    {
        vs_vector_init(V, 4);
    }
    if (V->_nelems >= V->_buffersize)
    {
        if (vs_vector_resize(V, V->_buffersize * 2) != VS_OK)
        {
            return VS_ERROR;
        }
    }
    V->_data[V->_nelems] = data;
    V->_nelems++;
    return VS_OK;
}


int vs_vector_append_dup(_VSVector* V, void* data, int data_size)
{
    assert(V && data);

    if ((nullptr == V->_data) || (V->_buffersize < 1))
    {
        vs_vector_init(V, 4);
    }

    void* d = vs_malloc(data_size);

    if (nullptr == d)
    {
        return VS_ERROR;
    }

    memcpy(d, data, data_size);
    return vs_vector_append(V, d);
}


void* vs_vector_get(const _VSVector* V, int pos)
{
    assert(V && V->_data);
    if (pos < 0 || pos >= V->_nelems)
    {
        return 0;
    }
    else
    {
        return V->_data[pos];
    }
}

void* vs_vector_set(_VSVector* V, int pos, void* data)
{
    assert(V && data && pos >= 0);
    if (!V->_data || V->_buffersize < 1)
    {
        vs_vector_init(V, 4);
    }
    if (V->_buffersize <= pos)
    {
        int nsize = V->_buffersize;
        while (nsize <= pos)
        {
            nsize *= 2;
        }
        if (vs_vector_resize(V, nsize) != VS_OK)
        {
            return 0;    // insuficient error handling here! VS_ERROR
        }
    }
    if (pos >= V->_nelems) // insert after end of vector
    {
        int i;
        for (i = V->_nelems; i < pos + 1; i++)
        {
            V->_data[i] = 0;
        }
        V->_nelems = pos + 1;
    }
    void* old = V->_data[pos];
    V->_data[pos] = data;
    return old;
}

void* vs_vector_set_dup(_VSVector* V, int pos, void* data, int data_size)
{
    void* d = vs_malloc(data_size);
    if (!d)
    {
        return 0;    // insuficient error handling here! VS_ERROR
    }
    memcpy(d, data, data_size);
    return vs_vector_set(V, pos, d);
}


int vs_vector_resize(_VSVector* V, int newsize)
{
    assert(V && V->_data);
    if (newsize < 1)
    {
        newsize = 1;
    }
    V->_data = (void**)vs_realloc(V->_data, newsize * sizeof(void*));
    V->_buffersize = newsize;
    if (V->_nelems > V->_buffersize)
    {
        V->_nelems = V->_buffersize;
    }
    if (!V->_data)
    {
        vs_log_error("VS_Vector", "out of memory!");
        return VS_ERROR;
    }
    else
    {
        return VS_OK;
    }
}

_VSVector vs_vector_filter(const _VSVector* V, short (*pred)(void*, void*), void* param)
{
    _VSVector result;
    assert(V);
    vs_vector_init(&result, V->_nelems);
    for (int i = 0; i < V->_nelems; i++)
    {
        if (pred(param, V->_data[i]))
        {
            vs_vector_append(&result, V->_data[i]);
        }
    }
    return result;
}

_VSVector vs_vector_concat(const _VSVector* V1, const _VSVector* V2)
{
    _VSVector result;
    assert(V1 && V2);
    vs_vector_init(&result, V1->_nelems + V2->_nelems);
    memcpy(result._data, V1->_data, sizeof(void*)* V1->_nelems);
    memcpy(result._data + V1->_nelems, V2->_data, sizeof(void*)* V2->_nelems);
    result._nelems = V1->_nelems + V2->_nelems;
    return result;
}


/* ARRAY */

VSArray vs_array_new(int len)
{
    VSArray a;
    a.dat = (double*)vs_zalloc(sizeof(double) * len);
    a.len = len;
    return a;
}

VSArray vs_array(double vals[], int len)
{
    VSArray a = vs_array_new(len);
    memcpy(a.dat, vals, sizeof(double)*len);
    return a;
}

VSArray* vs_array_plus(VSArray* c, VSArray a, VSArray b)
{
    int i;
    assert(a.len == b.len);
    if (c->len == 0 )
    {
        *c = vs_array_new(a.len);
    }
    for (i = 0; i < a.len; i++)
    {
        c->dat[i] = a.dat[i] + b.dat[i];
    }
    return c;
}

VSArray* vs_array_scale(VSArray* c, VSArray a, double f)
{
    if (c->len == 0 )
    {
        *c = vs_array_new(a.len);
    }
    for (int i = 0; i < a.len; i++)
    {
        c->dat[i] = a.dat[i] * f;
    }
    return c;
}

VSArray vs_array_copy(VSArray a)
{
    VSArray c = vs_array_new(a.len);
    memcpy(c.dat, a.dat, a.len * sizeof(double));
    return c;
}

void vs_array_zero(VSArray* a)
{
    memset(a->dat, 0, sizeof(double)*a->len);
}

void vs_array_swap(VSArray* a, VSArray* b)
{
    VSArray tmp;
    tmp = *a;
    *a = *b;
    *b = tmp;
}

void vs_array_free(VSArray a)
{
    vs_free(a.dat);
    a.dat = 0;
    a.len = 0;
}

void vs_array_print(VSArray a, FILE* f)
{
    for (int i = 0; i < a.len; i++)
    {
        fprintf(f, "%g ", a.dat[i]);
    }
}
