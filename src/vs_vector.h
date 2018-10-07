#pragma once
/*
 * vsvector.h -- a dynamic array
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
#include "libvidstab.h"
#include <cstddef>
#include <cstdio>
#include <cstring>

#include "common_exception.h"


namespace Common
{
    template <typename _Tp> class VsVector
    {
    public:
        typedef _Tp value_type;
        
        
        typedef size_t size_type;
        
        
        typedef value_type& reference;
        
        
        typedef const value_type& const_reference;
        
        
        typedef value_type* pointer;
        
        
        typedef const value_type* const_pointer;
        
        
        typedef value_type* iterator;
        
        
        typedef const value_type* const_iterator;
        
        
        VsVector(LocalMotions& aM)
            :
            _m { aM }
        {
        
        }
        
        
        ~VsVector()
        {
//            if (nullptr != _m._data)
//            {
//                for (int i = 0; i < _m._nelems; i++)
//                {
//                    if (nullptr != _m._data[i])
//                    {
//                        vs_free(_m._data[i]);
//                    }
//                }
//            }
//
//            if (_m._data)
//            {
//                vs_free(_m._data);
//            }
//
//            _m._data       = 0;
//            _m._buffersize = 0;
//            _m._nelems     = 0;
        }
        
        
        inline int size() const noexcept
        {
            return _m._nelems;
        }
        
        
        inline reference operator[](std::size_t aIdx)
        {
            if (nullptr == _m._data)
            {
                throw Common::VS_EXCEPTION_M(vsvector, "Container destroyed!");
            }
            
            if (std::size_t(_m._buffersize) <= aIdx)
            {
                throw Common::VS_EXCEPTION_M(vsvector, "Index %i out of range (0 - %i)!", aIdx, _m._buffersize);
            }
            
            if (nullptr == _m._data[aIdx])
            {
                throw Common::VS_EXCEPTION_M(vsvector, "No data on index %i!", aIdx);
            }
            
            return *(pointer(_m._data[aIdx]));
        }
        
        
        inline const_reference operator[](std::size_t aIdx) const
        {
            if (nullptr == _m._data)
            {
                throw Common::VS_EXCEPTION_M(vsvector, "Container destroyed!");
            }
            
            if (std::size_t(_m._buffersize) <= aIdx)
            {
                throw Common::VS_EXCEPTION_M(vsvector, "Index %i out of range (0 - %i)!", aIdx, _m._buffersize);
            }
            
            if (nullptr == _m._data[aIdx])
            {
                throw Common::VS_EXCEPTION_M(vsvector, "No data on index %i!", aIdx);
            }
            
            return *(const_pointer(_m._data[aIdx]));
        }
        
        
        /**
         * @brief   Initialize buffers
         * @param   aCnt    Count of items
         */
        void init(std::size_t aCnt)
        {
            if (aCnt > 0)
            {
                _m._data       = (void**)vs_zalloc(sizeof(pointer) * aCnt);
                _m._buffersize = aCnt;
                
                if (nullptr == _m._data)
                {
                    throw Common::VS_EXCEPTION_M(vsvector, "Allocation error!");
                }
            }
            else
            {
                _m._data       = nullptr;
                _m._buffersize = aCnt;
            }
            
            _m._nelems = 0;
        }
        
        
        void destroy()
        {
            if (nullptr == _m._data)
            {
                erase();
                vs_free(_m._data);
                
                _m._data       = nullptr;
                _m._buffersize = 0;
            }
        }
        
        
        void erase()
        {
            if (nullptr == _m._data)
            {
                throw Common::VS_EXCEPTION_M(vsvector, "Container destroyed!");
            }
            
            for (int i = 0; i < _m._nelems; i++)
            {
                if (nullptr != _m._data[i])
                {
                    vs_free(_m._data[i]);
                    _m._data[i] = nullptr;
                }
            }
            
            _m._nelems = 0;
        }
        
        
        void reserve(int aNewsize)
        {
            if (aNewsize < 1)
            {
                aNewsize = 1;
            }
            
            if (_m._buffersize < aNewsize)
            {
                _m._data = (void**)vs_realloc(_m._data, aNewsize * sizeof(pointer));
                
                if (nullptr == _m._data)
                {
                    throw Common::VS_EXCEPTION_M(vsvector, "Allocation error!");
                }
                
                _m._buffersize = aNewsize;
            }
        }
        
        
        void push_back(pointer aData)
        {
            _initBase();
            
            if (_m._nelems >= _m._buffersize)
            {
                reserve(_m._buffersize * 2);
            }
            
            _m._data[_m._nelems] = aData;
            ++_m._nelems;
        }
        
        
        void push_back(const_reference aData)
        {
            _initBase();
            
            pointer d = pointer(vs_malloc(sizeof(value_type)));
            
            if (nullptr == d)
            {
                throw Common::VS_EXCEPTION_M(vsvector, "Allocation error!");
            }
            
            *d = aData;
            push_back(d);
        }
        
        
        pointer exchange(int     aIdx,
                         pointer aNewData)
        {
            _initBase();
            
            if (_m._buffersize <= aIdx)
            {
                throw Common::VS_EXCEPTION_M(vsvector, "Index %i out of rang (0 - %i)!", aIdx, _m._buffersize);
            }
            
            
            int    cnt  = _m._buffersize;
            while (cnt <= aIdx)
            {
                cnt *= 2;
            }
            
            reserve(cnt);
            
            if (aIdx >= _m._nelems)
            {
                for (int i = _m._nelems; i < (aIdx + 1); ++i)
                {
                    _m._data[i] = nullptr;
                }
                
                _m._nelems = aIdx + 1;
            }
            
            pointer  old   = pointer(_m._data[aIdx]);
            _m._data[aIdx] = aNewData;
            return old;
        }
        
        
        pointer exchange(int       aIdx,
                         reference aData)
        {
            pointer d = pointer(vs_malloc(sizeof(value_type)));
            
            if (nullptr == d)
            {
                throw Common::VS_EXCEPTION_M(vsvector, "Allocation error!");
            }
            
            *d = aData;
            return exchange(aIdx, d);
        }
        
        
        template <typename _Tp2> void filter(VsVector&   result,
                                             bool      (*pred)(const_reference, const _Tp2&),
                                             const _Tp2& param) const
        {
            if (nullptr == _m._data)
            {
                throw Common::VS_EXCEPTION_M(vsvector, "Container destroyed!");
            }
            
            result.init(_m._nelems);
            
            for (int i = 0; i < _m._nelems; ++i)
            {
                if (pred(*(pointer(_m._data[i])), param))
                {
                    result.push_back(pointer(_m._data + i));
                }
            }
        }
        
        
        void concat(const VsVector& aSrc1,
                    const VsVector& aSrc2)
        {
            init(aSrc1._m._nelems + aSrc2._m._nelems);
            memcpy(_m._data,                    aSrc1._m._data, sizeof(pointer) * aSrc1._m._nelems);
            memcpy(_m._data + aSrc1._m._nelems, aSrc2._m._data, sizeof(pointer) * aSrc2._m._nelems);
            
            _m._nelems = aSrc1._m._nelems + aSrc2._m._nelems;
        }
        
        
        
        
        
        
        /*
         * Deprecated
         */
        LocalMotions& LocalMotionsC()
        {
            return _m;
        }
        
        const LocalMotions& LocalMotionsC() const
        {
            return _m;
        }
        
        
        
        
    private:
        inline void _initBase()
        {
            if ((nullptr == _m._data) || (_m._buffersize < 1))
            {
                init(4);
            }
        }
        
        
        LocalMotions& _m;
    };
}



/**
 * vs_vector_init:
 *     intializes a vector data structure.
 *     A vector will grow but not shrink if elements are added.
 *
 * Parameters:
 *              V: pointer to list to be initialized.
 *     buffersize: size of buffer (if known, then # of resizes are reduced)
 * Return Value:
 *     VS_OK on success,
 *     VS_ERROR on error.
 */
int vs_vector_init(_VSVector* V, int buffersize);

/**
 * vs_vector_fini:
 *     finalizes a vector data structure. Frees all resources aquired,
 *     but *NOT* the data pointed by vector elements.
 *
 * Parameters:
 *     V: pointer to list to be finalized
 * Return Value:
 *     VS_OK on success,
 *     VS_ERROR on error.
 */
int vs_vector_fini(_VSVector* V);

/**
 * vs_vector_zero:
 *    deletes all data pointed to by the vector elements.
 *    sets the number of elements to 0 but does not delete buffer
*/
int vs_vector_zero(_VSVector* V);

/**
 * vs_vector_size:
 *     gives the number of elements present in the vector
 *     (not the internal buffer size).
 *
 * Parameters:
 *     V: vector to be used.
 * Return Value:
 *    -1 on error,
 *    the number of elements otherwise
 */
int vs_vector_size(const _VSVector* V);


/**
 * vs_vector_append:
 *     append an element to the vector.
 *     The element is added to the end of the vector.
 *
 * Parameters:
 *        V: pointer to vector to be used
 *     data: pointer to data to be appended or prepend.
 *           *PLEASE NOTE* that JUST THE POINTER is copied on the newly-added
 *           element. NO deep copy is performed.
 *           The caller has to allocate memory by itself if it want to
 *           add a copy of the data.
 * Return Value:
 *     VS_OK on success,
 *     VS_ERROR on error.
 */
int vs_vector_append(_VSVector* V, void* data);

/**
 * vs_vector_append_dup:
 *  like vs_vector_append but copies data
 */
int vs_vector_append_dup(_VSVector* V, void* data, int data_size);


/* vs_vector_set:
 *      the newly inserted element BECOMES the position `pos' in the vector.
 *      and the old item is returned
 */
void* vs_vector_set(_VSVector* V, int pos, void* data);

/* vs_vector_set_dup:
 *      the newly inserted element is copied and BECOMES the position `pos' in the vector
 *      and the old item is returned
 */
void* vs_vector_set_dup(_VSVector* V, int pos, void* data, int data_size);

/*
 * vs_vector_get:
 *     gives access to the data pointed by the element in the given position.
 *
 * Parameters:
 *       V: vector to be accessed.
 *     pos: position of the element on which the data will be returned.
 * Return Value:
 *     NULL on error (requested element doesn't exist)
 *     a pointer to the data belonging to the requested vector item.
 */
void* vs_vector_get(const _VSVector* V, int pos);

/*
 * vs_vector_filter:
 *      returns a new vector with elements that fulfill predicate
 *      pred(param, elem)
 */
_VSVector vs_vector_filter(const _VSVector* V, short (*pred)(void*, void*), void* param);

/*
 * vs_vector_concat:
 *      returns a new vector with elements of vector V1 and V2 after another
 */
_VSVector vs_vector_concat(const _VSVector* V1, const _VSVector* V2);


/**
   A simple fixed-size double vector
*/
typedef struct vsarray_ VSArray;
struct vsarray_
{
    double* dat;
    int len;
};

/** creates an VSArray from a double array */
VSArray vs_array(double vals[], int len);

/** allocates a new (zero initialized) double array */
VSArray vs_array_new(int len);

/** adds two vectors ands stores results into c (if zero length then allocated) */
VSArray* vs_array_plus(VSArray* c, VSArray a, VSArray b);

/** scales a vector by a factor and stores results into c (if zero length then allocated) */
VSArray* vs_array_scale(VSArray* c, VSArray a, double f);

/** create a new deep copy of the vector */
VSArray vs_array_copy(VSArray a);

/** sets all elements of the vector to 0.0 */
void vs_array_zero(VSArray* a);

/** swaps the content of the two arrays */
void vs_array_swap(VSArray* a, VSArray* b);

/** free data */
void vs_array_free(VSArray a);

/** print array to file */
void vs_array_print(VSArray a, FILE* f);
