#version 120

varying vec4 pangoTexCoord;
uniform vec3 pangoColor[4];
uniform sampler2D pangoTexture[4];
uniform float pangoAlpha;

void main() {
	vec4 frag = vec4(0.0, 0.0, 0.0, 0.0);
	
	for(int i = 0; i < 4; i++) {
		float a = texture2D(pangoTexture[i], pangoTexCoord.st).a;
		vec3  c = pangoColor[i];
		
		if(frag.a == 0.0) frag = vec4(c, a * pangoAlpha);
		
		else frag = mix(frag, vec4(c, a), a * pangoAlpha);
	}
	
	gl_FragColor = frag;
}

