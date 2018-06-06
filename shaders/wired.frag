#version 450

#extension GL_ARB_separate_shader_objects	: enable
#extension GL_ARB_shading_language_420pack	: enable

layout (set=0, binding = 0) uniform sampler2DArray fontMap;
layout (set=1, binding = 0) uniform sampler2D		source;

layout (location = 0) in vec3 inFontUV;		//if it is a text drawing, inFontUV.z hold fontMap layer
layout (location = 1) in vec4 inColor;		//source rgba
layout (location = 2) in flat int inPatType;
layout (location = 3) in mat3x2 inMat;

layout (location = 0) out vec4 outFragColor;

layout (constant_id = 0) const int NUM_SAMPLES = 8;

void main()
{
	outFragColor = inColor;
}
