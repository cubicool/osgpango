// -*-c++-*- Copyright (C) 2010 osgPango Development Team
// $Id$

#include <sstream>
#include <osgPango/ShaderGenerator>

namespace osgPango {

const char* DEFAULT_VERTEX_SHADER =
	"varying vec4 pangoTexCoord;"
	"void main() {"
	"pangoTexCoord = gl_MultiTexCoord0;"
	"gl_Position = ftransform();"
	"}"
;

std::string baseFragmentHeader(unsigned int num) {
	std::ostringstream source;
	
	source
		<< "#version 120" << std::endl
		<< "varying vec4 pangoTexCoord;" << std::endl
		<< "uniform vec4 pangoColor[" << num << "];" << std::endl
		<< "uniform sampler2D pangoTexture[" << num << "];" << std::endl
		<< "uniform float pangoAlpha;" << std::endl
		<< "vec4 pangoGetColor(int i) {" << std::endl
		<< "vec4 c = pangoColor[i];" << std::endl
		<< "vec4 t = texture2D(pangoTexture[i], pangoTexCoord.st);" << std::endl
		<< "if(c.a >= 1.0) return vec4(vec3(t.rgb), t.a);" << std::endl
		<< "else return vec4(c.rgb, t.a);" << std::endl
		<< "}" << std::endl
		<< "void main() {" << std::endl
		<< "vec4 frag = vec4(0.0, 0.0, 0.0, 0.0);" << std::endl
	;

	return source.str();
}

std::string baseFragmentFooter() {
	std::ostringstream source;
	
	source
		<< "gl_FragColor = frag;" << std::endl
		<< "}" << std::endl
	;

	return source.str();
}

std::string createBackToFrontShader(unsigned int num) {
	std::ostringstream shaderSource;

	shaderSource
		<< baseFragmentHeader(num)
		<< "for(int i = " << num - 1 << "; i >= 0; i--) {" << std::endl
		<< "vec4 col = pangoGetColor(i);" << std::endl
		<< "frag = mix(frag, col, col.a * pangoAlpha);" << std::endl
		<< "}" << std::endl
		<< baseFragmentFooter()
	;

	return shaderSource.str();
}

std::string createLayerIndexShader(unsigned int num, const LayerIndexVector& liv) {
	std::ostringstream shaderSource;

	shaderSource
		<< baseFragmentHeader(num)
		<< "int[" << liv.size() << "] indices;" << std::endl
	;

	for(unsigned int i = 0; i < liv.size(); i++) shaderSource
		<< "indices[" << i << "] = " << liv[i] << ";" << std::endl
	;

	shaderSource
		<< "for(int i = 0; i < "<< liv.size() << "; i++) {" << std::endl
		<< "vec4 col = pangoGetColor(indices[i]);" << std::endl
		<< "frag = mix(frag, col, col.a * pangoAlpha);" << std::endl
		<< "}" << std::endl
		<< baseFragmentFooter()
	;

	return shaderSource.str();
}

const char* getDefaultVertexShader() {
	return DEFAULT_VERTEX_SHADER;
}

}
