#version 120
#define NUMLAYERS 4

varying vec4      pangoTexCoord;
uniform vec4      pangoColor[NUMLAYERS];
uniform sampler2D pangoTexture[NUMLAYERS];
uniform float     pangoAlpha;
uniform float     osg_SimulationTime;

vec4 pangoGetColor(int i) {
	vec4 c = pangoColor[i];
	vec4 t = texture2D(pangoTexture[i], pangoTexCoord.st);

	return vec4(c.rgb * t.a, t.a);
}

void main() {
	vec4 frag = vec4(0.0, 0.0, 0.0, 0.0);

	for(int i = (NUMLAYERS - 1); i >= 0; i--) {
		vec4 col = pangoGetColor(i);

        // Apply scanline effect to the target layer only
        if(i == 0) {
            // Convert time and texcoord to a moving pattern
            float y = gl_FragCoord.y; // or use pangoTexCoord.t * some_height
            float offset = osg_SimulationTime * 10.0;
            float pattern = mod(y + offset, 4.0); // 3px on, 3px off
            float intensity = step(pattern, 3.0); // 1.0 for scan line, 0.0 otherwise

            // Mix brighter color for scanline
            col.rgb *= (1.0 - intensity * 0.5);
        }

		frag = (col + (1.0 - col.a) * frag) * pangoAlpha;
	}

	gl_FragColor = frag;
}

