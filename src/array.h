#ifndef ARRAY_H_INCLUDED
#define ARRAY_H_INCLUDED

#include <stddef.h>

#define CTOR_ARRAY_EXP(type, expansion)                             \
typedef struct {                                                    \
    uint32_t    count;                                              \
    uint32_t    size;                                               \
    type       *elements;                                           \
} array_##type;                                                     \
static array_##type array_create_##type (uint32_t reservedSize) {   \
    type *elts = (type*)malloc(reservedSize * sizeof(type));        \
    return (array_##type) {0, reservedSize, elts};                  \
}                                                                   \
static int array_add_##type (array_##type* arr, type elt) {         \
    if (arr->size <= arr->count) {                                  \
        uint32_t newSize = arr->size expansion;                     \
        type *elts = (type*)realloc(arr->elements, newSize * sizeof(type)); \
        if (elts == NULL)                                           \
            return -1;                                              \
        arr->elements = elts;                                       \
        arr->size = newSize;                                        \
    }                                                               \
    arr->elements[arr->count++] = elt;                              \
    return arr->count - 1;                                          \
}                                                                   \
static type* array_last_##type (array_##type* arr) {                \
    if (!arr->count)                                                \
        return NULL;                                                \
    return &arr->elements[arr->count - 1];                          \
}                                                                   \
static void array_destroy_##type (array_##type* arr) {              \
    if (arr && arr->size > 0) {                                     \
        free(arr->elements);                                        \
        arr->elements = NULL;                                       \
        arr->count = 0;                                             \
        arr->size = 0;                                              \
    }                                                               \
}
#define ARRAY_DEL_ELT(type)                                         \
static void array_del_##type (array_##type* arr, type elt) {        \
    uint32_t i;                                                     \
    for (i = 0; i < arr->count; i++) {                              \
        if (arr->elements[i] == elt)                                \
            break;                                                  \
    }                                                               \
    if (i == arr->count) {                                          \
        return;                                                     \
    }                                                               \
    if (i < arr->count - 1)                                         \
        memmove (&arr->elements[i], &arr->elements[i + 1], (arr->count - i - 1) * sizeof(type)); \
    arr->count--;                                                   \
}

#define CTOR_ARRAY(type) CTOR_ARRAY_EXP(type,* 2)
#endif
