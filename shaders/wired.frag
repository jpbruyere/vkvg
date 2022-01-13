#version 450

#extension GL_ARB_separate_shader_objects	: enable
#extension GL_ARB_shading_language_420pack	: enable
#extension GL_EXT_scalar_block_layout		: require

layout (set=0, binding = 0) uniform sampler2DArray fontMap;
layout (set=1, binding = 0) uniform sampler2D		source;
layout (scalar, set=2, binding = 0) uniform _uboGrad {
	vec4	colors[16];
	float	stops[16];
	vec4    cp[2];
	uint	count;
}uboGrad;

layout (location = 0) in vec3		inFontUV;	//if it is a text drawing, inFontUV.z hold fontMap layer
layout (location = 1) in vec4		inSrc;		//source bounds or color depending on pattern type
layout (location = 2) in flat int	inPatType;	//pattern type
layout (location = 3) in flat float	inOpacity;
layout (location = 4) in mat3x2		inMat;

layout (location = 0) out vec4 outFragColor;

layout (constant_id = 0) const int NUM_SAMPLES = 8;

void main()
{
	outFragColor = vec4(1,1,1,1);//inSrc;
}
