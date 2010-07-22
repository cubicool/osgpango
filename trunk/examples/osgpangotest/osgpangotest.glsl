#version 120

uniform int pangoNumLayers;

vec4 osgPango_GetFragment(vec4 coord, sampler2D textures[8], vec3 colors[8], float alpha) {
	vec4 frag = vec4(0.0, 0.0, 0.0, 0.0);
	
	// for(int i = 0; i < pangoNumLayers; i++) {
	for(int i = 0; i < 4; i++) {
		float a = texture2D(textures[i], coord.st).a;
		vec3  c = colors[i];

		frag = mix(frag, vec4(c, a), a * alpha);
	}

	return frag;
}

