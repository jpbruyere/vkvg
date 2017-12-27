#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (binding = 0) uniform sampler2D tex;

layout (location = 0) in vec4 inColor;
layout (location = 1) in vec3 inUV;

layout (location = 0) out vec4 outFragColor;

void main()
{
	vec4 c = texture(tex, inUV.xy);
	outFragColor = c;
}
