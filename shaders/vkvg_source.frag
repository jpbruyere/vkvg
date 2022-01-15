/*
 * Copyright (c) 2018-2021 Jean-Philippe Bruyère <jp_bruyere@hotmail.com>
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

#extension GL_ARB_separate_shader_objects	: enable
#extension GL_ARB_shading_language_420pack	: enable
#extension GL_EXT_scalar_block_layout		: require

layout (set=1, binding = 0) uniform sampler2D		source;

layout (location = 0) in vec4		inSrc;		//source bounds or color depending on pattern type
layout (location = 1) in flat float	inOpacity;
layout (location = 2) in mat3x2		inMat;

layout (location = 0) out vec4 outFragColor;

layout (constant_id = 0) const int NUM_SAMPLES = 8;

void main()
{	
	vec2 p = (gl_FragCoord.xy - inSrc.xy);
	vec2 uv = vec2(
		inMat[0][0] * p.x + inMat[1][0] * p.y + inMat[2][0],
		inMat[0][1] * p.x + inMat[1][1] * p.y + inMat[2][1]
	);

	vec4 c = texture (source, uv / inSrc.zw);
	c.a *= inOpacity;
	outFragColor = c;
}
