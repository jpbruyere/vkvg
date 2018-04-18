#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec2	inPos;
layout (location = 1) in vec3	inUV;

layout (location = 0) out vec3	outUV;
layout (location = 1) out vec4	outSrc;
layout (location = 2) out flat int outPatType;

out gl_PerVertex
{
	vec4 gl_Position;
};

layout(push_constant) uniform PushConsts {
	vec4 source;
	vec2 size;
	int  srcType;
} pushConsts;

#define SOLID			0
#define SURFACE			1
#define LINEAR			2
#define RADIAL			3
#define MESH			4
#define RASTER_SOURCE	5

void main()
{
	outUV		= inUV;
	outPatType	= pushConsts.srcType;
	outSrc		= pushConsts.source;

	gl_Position = vec4(inPos.xy * vec2(2) / pushConsts.size - vec2(1), 0.0, 1.0);
}
