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
layout (location = 1) in vec4	inSrc;			//source bounds
layout (location = 2) in flat int inPatType;

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
	vec4 c = vec4(0);
	switch(inPatType){
	case SOLID:
		c = inSrc;
		break;
	case SURFACE:
		vec2 srcUV = (gl_FragCoord.xy - inSrc.xy) / inSrc.zw;
		c = texture (source, srcUV);
		break;
	case LINEAR:
		float  alpha = atan( -uboGrad.cp[1].y + uboGrad.cp[0].y, uboGrad.cp[1].x - uboGrad.cp[0].x );
		float  gradientStartPosRotatedX = uboGrad.cp[0].x*cos(alpha) - uboGrad.cp[0].y*sin(alpha);
		float  gradientEndPosRotatedX   = uboGrad.cp[1].x*cos(alpha) - uboGrad.cp[1].y*sin(alpha);
		float  d = gradientEndPosRotatedX - gradientStartPosRotatedX;

		float y = inSrc.y - gl_FragCoord.y;
		float x = gl_FragCoord.x;
		float xLocRotated = x*cos( alpha ) - y*sin( alpha );

		c = mix(uboGrad.colors[0], uboGrad.colors[1], smoothstep( gradientStartPosRotatedX + uboGrad.stops[0].r*d, gradientStartPosRotatedX + uboGrad.stops[1].r*d, xLocRotated ) );
		for ( int i=1; i<uboGrad.count-1; ++i )
			c = mix(c, uboGrad.colors[i+1], smoothstep( gradientStartPosRotatedX + uboGrad.stops[i].r*d, gradientStartPosRotatedX + uboGrad.stops[i+1].r*d, xLocRotated ) );
		break;
	}

	if (inFontUV.z >= 0.0)
		c *= texture(fontMap, inFontUV).r;

	outFragColor = c;
}
