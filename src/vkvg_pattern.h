#ifndef VKVG_PATTERN_H
#define VKVG_PATTERN_H

#include "vkvg_internal.h"
#include "vkvg.h"
#include "vkh.h"

typedef enum _vkvg_extend {
    VKVG_EXTEND_NONE,
    VKVG_EXTEND_REPEAT,
    VKVG_EXTEND_REFLECT,
    VKVG_EXTEND_PAD
} vkvg_extend_t;

typedef struct _vkvg_pattern_t {
    VkvgDevice		dev;
    VkDescriptorSet	descriptorSet;
    vkvg_extend_t	extend;
    VkhImage		img;
}vkvg_pattern;
#endif
