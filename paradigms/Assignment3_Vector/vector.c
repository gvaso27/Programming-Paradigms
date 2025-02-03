#include "vector.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

void VectorNew(vector *v, int elemSize, VectorFreeFunction freeFn, int initialAllocation)
{
    assert(elemSize > 0);
    assert(initialAllocation > -1);
    if (initialAllocation == 0) initialAllocation = 4;
    v->alloc_len = initialAllocation;
    v->elem_size = elemSize;
    v->log_len = 0;
    v->freeFn = freeFn;
    v->elems = malloc(v->alloc_len * v->elem_size);
}

void VectorDispose(vector *v)
{
    if(v->freeFn != NULL){
        for(int i = 0; i < v->log_len; i++){
            v->freeFn((char*)v->elems + v->elem_size * i);
        }
    }
    free(v->elems);
}

int VectorLength(const vector *v)
{ 
    return v->log_len; 
}

void *VectorNth(const vector *v, int position)
{ 
    assert(position >= 0 && position < v->log_len);
    return ((char*)v->elems + position * v->elem_size); 
}

void VectorReplace(vector *v, const void *elemAddr, int position)
{
    assert(position >= 0 && position < v->log_len);
    memcpy((char*)v->elems + position * v->elem_size, elemAddr, v->elem_size);
}

void VectorInsert(vector *v, const void *elemAddr, int position)
{
    assert(position >= 0 && position <= v->log_len);
    if(v->log_len == v->alloc_len){
        v->alloc_len *= 2;
        v->elems = realloc(v->elems, v->alloc_len * v->elem_size); 
        assert(v->elems != NULL);
    }
    for(int i = v->log_len-1; i >= position; i--){
        memcpy(((char*)v->elems + (i+1) * v->elem_size), ((char*)v->elems + i * v->elem_size), v->elem_size);
    }
    memcpy(((char*)v->elems + position * v->elem_size), elemAddr, v->elem_size);
    v->log_len++;
}

void VectorAppend(vector *v, const void *elemAddr)
{
    if(v->log_len == v->alloc_len){
        v->alloc_len *= 2;
        v->elems = realloc(v->elems, v->alloc_len * v->elem_size);
    }
    memcpy(((char*)v->elems + v->log_len * v->elem_size), elemAddr, v->elem_size);
    v->log_len++;
}

void VectorDelete(vector *v, int position)
{
    assert(position >= 0 && position < v->log_len);
    for(int i = position; i < v->log_len - 1; i++){
        memmove(((char*)v->elems + v->elem_size * i), ((char*)v->elems + v->elem_size * (i + 1)), v->elem_size);
    }
    if(position == v->log_len-1 && v->freeFn != NULL){
        v->freeFn(((char*)v->elems + position * v->elem_size));
    }
    v->log_len--;
}

void VectorSort(vector *v, VectorCompareFunction compare)
{
    assert(compare != NULL);
    qsort(v->elems, v->log_len, v->elem_size, compare);
}

void VectorMap(vector *v, VectorMapFunction mapFn, void *auxData)
{
    assert(mapFn != NULL);
    for (int i = 0; i < v->log_len; i++) {
        mapFn(((char*)v->elems + i * v->elem_size), auxData);
    }
}

static const int kNotFound = -1;
int VectorSearch(const vector *v, const void *key, VectorCompareFunction searchFn, int startIndex, bool isSorted)
{
    assert(startIndex >= 0 && startIndex <= v->log_len);
    assert(key != NULL && searchFn != NULL);
    void* helper = NULL;
    if(isSorted){
        helper = bsearch(key, (char*)v->elems + startIndex * v->elem_size, v->log_len - startIndex, v->elem_size, searchFn);
        if (helper != NULL) {
            return ((char *)helper - (char *)v->elems) / v->elem_size;
        }
    }else{
        for(int i = startIndex; i < v->log_len; i++){
            if(searchFn(key, ((char*)v->elems + i * v->elem_size))==0)return i;
        }
    }
    return kNotFound;
} 
