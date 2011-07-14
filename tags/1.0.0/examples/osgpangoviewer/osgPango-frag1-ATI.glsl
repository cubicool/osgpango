#version 120
#define NUMLAYERS 1

varying vec4      pangoTexCoord;
uniform vec4      pangoColor[NUMLAYERS];
uniform sampler2D pangoTexture[NUMLAYERS];
uniform float     pangoAlpha;

vec4 pangoGetColor0() {
	vec4 c = pangoColor[0];
	vec4 t = texture2D(pangoTexture[0], pangoTexCoord.st);
	
	return vec4(c.rgb * t.a, t.a);
}

void main() {
	vec4 frag = vec4(0.0, 0.0, 0.0, 0.0);
	vec4 col  = pangoGetColor0();
	
	gl_FragColor = (col + (1.0 - col.a) * frag) * pangoAlpha;
}

