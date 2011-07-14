#version 120
#define NUMLAYERS 4

varying vec4      pangoTexCoord;
uniform vec4      pangoColor[NUMLAYERS];
uniform sampler2D pangoTexture[NUMLAYERS];
uniform float     pangoAlpha;

vec4 pangoGetColor(int i) {
	vec4 c = pangoColor[i];
	vec4 t = texture2D(pangoTexture[i], pangoTexCoord.st);

	return vec4(c.rgb * t.a, t.a);
}

void main() {
	vec4 frag = vec4(0.0, 0.0, 0.0, 0.0);

	for(int i = (NUMLAYERS - 1); i >= 0; i--) {
		vec4 col = pangoGetColor(i);
		
		frag = (col + (1.0 - col.a) * frag) * pangoAlpha;
	}

	gl_FragColor = frag;
}

