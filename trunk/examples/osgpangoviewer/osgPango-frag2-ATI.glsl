#version 120
#define NUMLAYERS 2

varying vec4      pangoTexCoord;
uniform vec4      pangoColor[NUMLAYERS];
uniform sampler2D pangoTexture[NUMLAYERS];
uniform float     pangoAlpha;

vec4 pangoHandleColor(vec4 c, vec4 t) {
	if(c.a == 1.0) return vec4(t.rgb, 1.0);
	
	else if(c.a == 2.0) return vec4(t.rgb, t.a);

	else return vec4(c.rgb * t.a, t.a);
}

vec4 pangoGetColor0() {
	vec4 c = pangoColor[0];
	vec4 t = texture2D(pangoTexture[0], pangoTexCoord.st);
	
	return pangoHandleColor(c, t);
}

vec4 pangoGetColor1() {
	vec4 c = pangoColor[1];
	vec4 t = texture2D(pangoTexture[1], pangoTexCoord.st);
	
	return pangoHandleColor(c, t);
}

void main() {
	vec4 frag = vec4(0.0, 0.0, 0.0, 0.0);
	vec4 col  = pangoGetColor1();

	frag = (col + (1.0 - col.a) * frag) * pangoAlpha;
	col  = pangoGetColor0();
	
	gl_FragColor = (col + (1.0 - col.a) * frag) * pangoAlpha;
}

