#ifndef ARRAY_H_INCLUDED
#define ARRAY_H_INCLUDED

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#ifndef ARRAY_ELASTICITY
#define ARRAY_ELASTICITY 10
#endif
#ifndef ARRAY_INIT
#define ARRAY_INIT 4
#endif
#ifndef ARRAY_ELEMENT_TYPE
#define ARRAY_ELEMENT_TYPE void *
#endif

typedef struct _array_t {
    uint32_t            count;
    uint32_t            size;
    ARRAY_ELEMENT_TYPE *elements;
} array_t;

struct _array_t *array_create();
void             array_destroy(struct _array_t *arr);
void             array_clean(struct _array_t *arr);
void             array_add(struct _array_t *arr, ARRAY_ELEMENT_TYPE element);

#ifdef ARRAY_IMPLEMENTATION
array_t *array_create() {
    array_t *arr  = (array_t *)malloc(sizeof(array_t));
    arr->elements = (ARRAY_ELEMENT_TYPE *)malloc(ARRAY_INIT * sizeof(ARRAY_ELEMENT_TYPE));
    arr->count    = 0;
    arr->size     = ARRAY_INIT;
    return arr;
}

void array_destroy(array_t *arr) {
    free(arr->elements);
    free(arr);
}

void array_add(array_t *arr, ARRAY_ELEMENT_TYPE element) {
    if (arr->size == arr->count) {
        arr->size += ARRAY_ELASTICITY;
        arr->elements = (ARRAY_ELEMENT_TYPE *)realloc(arr->elements, (arr->size) * sizeof(ARRAY_ELEMENT_TYPE));
    }
    // memcpy (&arr->elements[arr->count], &element, sizeof (ARRAY_ELEMENT_TYPE));
    arr->elements[arr->count] = element;
    arr->count++;
}

void array_reset(array_t *arr) {
    arr->count    = 0;
}
#endif
#endif
