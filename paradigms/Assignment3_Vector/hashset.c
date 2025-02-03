#include "hashset.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

void HashSetNew(hashset *h, int elemSize, int numBuckets,
		HashSetHashFunction hashfn, HashSetCompareFunction comparefn, HashSetFreeFunction freefn)
{
	assert(elemSize > 0);
	assert(numBuckets > 0);
	assert(hashfn != NULL);
	assert(comparefn != NULL);
	h->elem_size = elemSize;
	h->log_len = 0;
	h->num_buckets = numBuckets;
	h->hashFn = hashfn;
	h->compareFn = comparefn;
	h->freeFn = freefn;
	h->elems = malloc(numBuckets * sizeof(vector));
	for(int i = 0; i < h->num_buckets; i++){
		VectorNew((vector*)h->elems + i, h->elem_size, freefn, 4);
	}
}

void HashSetDispose(hashset *h)
{
	if(h->freeFn != NULL){
		for(int i = 0; i < h->num_buckets; i++){
			VectorDispose((vector*)h->elems + i);
			h->freeFn((vector*)h->elems + i);
		}
	}
	free(h->elems);
}

int HashSetCount(const hashset *h)
{ return h->log_len; }

void HashSetMap(hashset *h, HashSetMapFunction mapfn, void *auxData)
{
	assert(mapfn != NULL);
	for(int i = 0; i < h->num_buckets; i++){
		VectorMap((vector*)h->elems + i, mapfn, auxData);
	}
}

void HashSetEnter(hashset *h, const void *elemAddr)
{
	assert(elemAddr != NULL);
	int bucket = h->hashFn(elemAddr, h->num_buckets);
	assert(bucket >= 0 && bucket < h->num_buckets);

	vector *v = (vector*)h->elems + bucket;

	if(VectorSearch(v, elemAddr, h->compareFn, 0, false) == -1){
		VectorAppend(v, elemAddr);
	}else{
		VectorReplace(v, elemAddr, VectorSearch(v, elemAddr, h->compareFn, 0, false));
	}

}

void *HashSetLookup(const hashset *h, const void *elemAddr)
{ 
	assert(elemAddr != NULL);
	int bucket = h->hashFn(elemAddr, h->num_buckets);
	assert(bucket >= 0 && bucket < h->num_buckets);

	vector *v = (vector*)h->elems + bucket;

	if(VectorSearch(v, elemAddr, h->compareFn, 0, false) == -1){
		return NULL;
	}else{
		return VectorNth(v, VectorSearch(v, elemAddr, h->compareFn, 0, false));
	}
} 
