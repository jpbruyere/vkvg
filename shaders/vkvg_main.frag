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
#extension GL_EXT_scalar_block_layout	: enable

layout (set=0, binding = 0) uniform sampler2DArray fontMap;
layout (set=1, binding = 0) uniform sampler2D		source;
layout (std430, set=2, binding = 0) uniform _uboGrad {
	vec4	colors[16];
	float	stops[16];
	vec4    cp[2];
	uint	count;
}uboGrad;

layout (location = 0) in vec3	inFontUV;		//if it is a text drawing, inFontUV.z hold fontMap layer
layout (location = 1) in vec4	inSrc;			//source bounds or color depending on pattern type
layout (location = 2) in flat int inPatType;	//pattern type
layout (location = 3) in mat3x2 inMat;

layout (location = 0) out vec4 outFragColor;

layout (constant_id = 0) const int NUM_SAMPLES = 8;


#define SOLID			0
#define SURFACE			1
#define LINEAR			2
#define RADIAL			3
#define MESH			4
#define RASTER_SOURCE	5

void main()
{
	vec4 c = inSrc;
	switch(inPatType){
	case SURFACE:
		vec2 p = (gl_FragCoord.xy - inSrc.xy);
		vec2 uv = vec2(
			inMat[0][0] * p.x + inMat[1][0] * p.y + inMat[2][0],
			inMat[0][1] * p.x + inMat[1][1] * p.y + inMat[2][1]
		);

		c = texture (source, uv / inSrc.zw);
		break;
	case LINEAR:
		//credit to Nikita Rokotyan for linear grad
		float  alpha = atan( -uboGrad.cp[0].w + uboGrad.cp[0].y, uboGrad.cp[0].z - uboGrad.cp[0].x );
		float  gradientStartPosRotatedX = uboGrad.cp[0].x*cos(alpha) - uboGrad.cp[0].y*sin(alpha);
		float  gradientEndPosRotatedX   = uboGrad.cp[0].z*cos(alpha) - uboGrad.cp[0].w*sin(alpha);
		float  d = gradientEndPosRotatedX - gradientStartPosRotatedX;

		float y = gl_FragCoord.y;//inSrc.y - gl_FragCoord.y;
		float x = gl_FragCoord.x;
		float xLocRotated = x*cos( alpha ) - y*sin( alpha );

		c = mix(uboGrad.colors[0], uboGrad.colors[1], smoothstep( gradientStartPosRotatedX + uboGrad.stops[0]*d, gradientStartPosRotatedX + uboGrad.stops[1]*d, xLocRotated ) );
		for ( int i=1; i<uboGrad.count-1; ++i )
			c = mix(c, uboGrad.colors[i+1], smoothstep( gradientStartPosRotatedX + uboGrad.stops[i]*d, gradientStartPosRotatedX + uboGrad.stops[i+1]*d, xLocRotated ) );
		break;
	case RADIAL:
		p = gl_FragCoord.xy / inSrc.xy;

		vec2 c0 = uboGrad.cp[0].xy / inSrc.xy;
		vec2 c1 = uboGrad.cp[1].xy / inSrc.xy;
		float r0 = uboGrad.cp[0].z / inSrc.x;
		float r1 = uboGrad.cp[1].z / inSrc.x;

		/// APPLY FOCUS MODIFIER
		//project a point on the circle such that it passes through the focus and through the coord,
		//and then get the distance of the focus from that point.
		//that is the effective gradient length
		float gradLength = 1.0;
		vec2 diff =c0 - c1;
		vec2 rayDir = normalize(p - c0);
		float a = dot(rayDir, rayDir);
		float b = 2.0 * dot(rayDir, diff);
		float cc = dot(diff, diff) - r1 * r1;
		float disc = b * b - 4.0 * a * cc;
		if (disc >= 0.0)
		{
			float t = (-b + sqrt(abs(disc))) / (2.0 * a);
			vec2 projection = c0 + rayDir * t;
			gradLength = distance(projection, c0)-r0;
		}
		else
		{
			//gradient is undefined for this coordinate
		}

		/// OUTPUT
		float grad = (distance(p, c0)-r0) / gradLength ;
		c = mix (uboGrad.colors[0], uboGrad.colors[1], smoothstep(uboGrad.stops[0],uboGrad.stops[1], grad));
		for (int i=2; i < uboGrad.count; i++ )
			c = mix(c, uboGrad.colors[i], smoothstep(uboGrad.stops[i-1],uboGrad.stops[i], grad));
		break;
	}

	if (inFontUV.z >= 0.0)
		c *= texture(fontMap, inFontUV).r;

	outFragColor = c;
}

void op_CLEAR () {
	outFragColor = vec4 (0);
}
