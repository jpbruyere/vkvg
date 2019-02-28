#version 450

#extension GL_ARB_separate_shader_objects	: enable
#extension GL_ARB_shading_language_420pack	: enable

layout (set=0, binding = 0) uniform sampler2DArray fontMap;
layout (set=1, binding = 0) uniform sampler2D		source;
layout (set=2, binding = 0) uniform _uboGrad {
	vec4    cp[3];
	vec4	colors[16];
	vec4	stops[16];
	uint	count;
}uboGrad;

layout (location = 0) in vec3	inFontUV;		//if it is a text drawing, inFontUV.z hold fontMap layer
layout (location = 1) in vec4	inSrc;			//source bounds or color
layout (location = 2) in flat int inPatType;
layout (location = 3) in mat3x2 inMat;

layout (location = 0) out vec4 outFragColor;

layout (constant_id = 0) const int NUM_SAMPLES = 8;

void main()
{
	outFragColor = vec4(1,1,1,1);//inSrc;
}
