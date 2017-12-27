#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec2 inPos;
layout (location = 1) in vec3 inUV;

layout (location = 0) out vec4 outColor;
layout (location = 1) out vec3 outUV;
layout (location = 2) out vec4 outSrcRect;

out gl_PerVertex
{
	vec4 gl_Position;
};

layout(push_constant) uniform PushConsts {
	vec4 source;
	vec2 scale;
	vec2 translate;
	int  srcType;
} pushConsts;

void main()
{
	outUV = inUV;
	if (pushConsts.srcType == 0){
		outSrcRect.z = -1;
		outColor = pushConsts.source;
	}else
		outSrcRect = pushConsts.source;

	gl_Position = vec4(inPos.xy*pushConsts.scale+pushConsts.translate,0.0, 1.0);
}
