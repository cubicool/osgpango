#version 120

varying vec4      pangoTexCoord;
uniform vec4      pangoColor[4];
uniform sampler2D pangoTexture[4];
uniform float     pangoAlpha;

vec4 pangoGetColor(int i) {
	vec4 c = pangoColor[i];
	vec4 t = texture2D(pangoTexture[i], pangoTexCoord.st);
	
	if(c.a >= 1.0) return vec4(vec3(t.rgb), t.a);
	
	else return vec4(c.rgb, t.a);
}

void main() {
	vec4 frag = vec4(0.0, 0.0, 0.0, 0.0);

	for(int i = 3; i >= 0; i--) {
		vec4 col = pangoGetColor(i);
		
		frag = mix(frag, col, col.a * pangoAlpha);
	}

	gl_FragColor = vec4(frag.rgb, frag.a);
}

