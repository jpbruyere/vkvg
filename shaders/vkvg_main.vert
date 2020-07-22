/*
 * Copyright (c) 2018 Jean-Philippe Bruyère <jp_bruyere@hotmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the
 * Software, and to permit persons to whom the Software is furnished to do so, subject
 * to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec2	inPos;
layout (location = 1) in vec4	inColor;
layout (location = 2) in vec3	inUV;

layout (location = 0) out vec3	outUV;
layout (location = 1) out vec4	outSrc;
layout (location = 2) out flat int outPatType;
layout (location = 3) out mat3x2 outMat;

out gl_PerVertex
{
	vec4 gl_Position;
};

layout(push_constant) uniform PushConsts {
	vec4 source;
	vec2 size;
	int  srcType;
	int  fullScreenQuad;
	mat3x2 mat;
	mat3x2 matInv;
} pc;

#define SOLID			0
#define SURFACE			1
#define LINEAR			2
#define RADIAL			3
#define MESH			4
#define RASTER_SOURCE	5

void main()
{
	outPatType	= pc.srcType;
	outMat		= pc.matInv;
	outSrc = pc.srcType == SOLID ? inColor : pc.source;

	if (pc.fullScreenQuad != 0) {
		gl_Position = vec4(inPos, 0.0f, 1.0f);
		outUV = vec3(0,0,-1);
		return;
	}

	outUV = inUV;

	vec2 p = vec2(
		pc.mat[0][0] * inPos.x + pc.mat[1][0] * inPos.y + pc.mat[2][0],
		pc.mat[0][1] * inPos.x + pc.mat[1][1] * inPos.y + pc.mat[2][1]
	);

	gl_Position = vec4(p * vec2(2) / pc.size - vec2(1), 0.0, 1.0);
}
