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
		float dist = 1;
		vec2 p0 = uboGrad.cp[0].xy / inSrc.xy;
		vec2 p1 = uboGrad.cp[0].zw / inSrc.xy;
		p = gl_FragCoord.xy / inSrc.xy;

		float l = length (p1 - p0);
		vec2 u = normalize (p1 - p0);

		if (u.y == 0)
			if (u.x < 0)
				dist = -(p.x-p0.x) / l;
			else
				dist = (p.x-p0.x) / l;
		else {
			float m = -u.x / u.y;
			float bb = p0.y - m * p0.x;
			dist =((p.y - m * p.x - bb) / sqrt (1 + m * m)) / l;
			if (u.y < 0)
				dist = - dist;
		}

		c = mix(uboGrad.colors[0], uboGrad.colors[1], smoothstep(uboGrad.stops[0], uboGrad.stops[1], dist));
		for ( int i=1; i<uboGrad.count-1; ++i )
			c = mix(c, uboGrad.colors[i+1], smoothstep(uboGrad.stops[i], uboGrad.stops[i+1], dist));
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

	c.a *= inOpacity;
	outFragColor = c;
}

void op_CLEAR () {
	outFragColor = vec4 (0);
}
