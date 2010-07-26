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
		<< "uniform vec3 pangoColor[" << num << "];" << std::endl
		<< "uniform sampler2D pangoTexture[" << num << "];" << std::endl
		<< "uniform float pangoAlpha;" << std::endl
		<< "void main() {" << std::endl
		<< "vec4 frag = vec4(0.0, 0.0, 0.0, 0.0);" << std::endl
	;

	return source.str();
}

std::string baseFragmentFooter() {
	std::ostringstream source;
	
	source
		<< "if(frag.a == 0.0) frag = vec4(c, a * pangoAlpha);" << std::endl
		<< "else frag = mix(frag, vec4(c, a), a * pangoAlpha);" << std::endl
		<< "}" << std::endl
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
		<< "float a = texture2D(pangoTexture[i], pangoTexCoord.st).a;" << std::endl
		<< "vec3 c = pangoColor[i];" << std::endl
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
		<< "int index = indices[i];" << std::endl
		<< "float a = texture2D(pangoTexture[index], pangoTexCoord.st).a;" << std::endl
		<< "vec3 c = pangoColor[index];" << std::endl
		<< baseFragmentFooter()
	;

	return shaderSource.str();
}

const char* getDefaultVertexShader() {
	return DEFAULT_VERTEX_SHADER;
}

}
