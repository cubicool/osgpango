// -*-c++-*- Copyright (C) 2011 osgPango Development Team
// $Id$

#include <osg/Notify>

#include <sstream>
#include <osgPango/ShaderGenerator>

namespace osgPango {

std::string defaultVertexShader() {
	std::ostringstream shaderSource;
	
	shaderSource
		<< "varying vec4 pangoTexCoord;" << std::endl
		<< "void main() {" << std::endl
		<< "pangoTexCoord = gl_MultiTexCoord0;" << std::endl
		<< "gl_Position = ftransform();" << std::endl
		<< "}" << std::endl
	;

	return shaderSource.str();
}

std::string baseFragmentHeader(unsigned int num) {
	std::ostringstream source;
	
	source
		<< "#version 120" << std::endl
		<< "#define NUMLAYERS " << num << std::endl
		<< "#define CAIRO_FORMAT_A8 0.0" << std::endl
		<< "#define CAIRO_FORMAT_RGB24 1.0" << std::endl
		<< "#define CAIRO_FORMAT_ARGB32 2.0" << std::endl
		<< "varying vec4 pangoTexCoord;" << std::endl
		<< "uniform vec4 pangoColor[NUMLAYERS];" << std::endl
		<< "uniform sampler2D pangoTexture[NUMLAYERS];" << std::endl
		<< "uniform float pangoAlpha;" << std::endl
		<< "vec4 pangoGetColor(int i) {" << std::endl
		<< "vec4 c = pangoColor[i];" << std::endl
		<< "vec4 t = texture2D(pangoTexture[i], pangoTexCoord.st);" << std::endl
		<< "if(c.a == CAIRO_FORMAT_RGB24) return vec4(t.rgb, 1.0);" << std::endl
		<< "else if(c.a == CAIRO_FORMAT_ARGB32) return vec4(t.rgb, t.a);" << std::endl
		<< "else return vec4(c.rgb * t.a, t.a);" << std::endl
		<< "}" << std::endl
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
		<< "void main() {" << std::endl
		<< "vec4 frag = vec4(0.0, 0.0, 0.0, 0.0);" << std::endl
		<< "for(int i = (NUMLAYERS - 1); i >= 0; i--) {" << std::endl
		<< "vec4 col = pangoGetColor(i);" << std::endl
		<< "frag = (col + (1.0 - col.a) * frag) * pangoAlpha;" << std::endl
		<< "}" << std::endl
		<< baseFragmentFooter()
	;

	return shaderSource.str();
}

std::string createLayerIndexShader(unsigned int num, const LayerIndexVector& liv) {
	std::ostringstream shaderSource;

	shaderSource
		<< baseFragmentHeader(num)
		<< "void main() {" << std::endl
		<< "vec4 frag = vec4(0.0, 0.0, 0.0, 0.0);" << std::endl
		<< "int[" << liv.size() << "] indices;" << std::endl
	;

	for(unsigned int i = 0; i < liv.size(); i++) shaderSource
		<< "indices[" << i << "] = " << liv[i] << ";" << std::endl
	;

	shaderSource
		<< "for(int i = 0; i < "<< liv.size() << "; i++) {" << std::endl
		<< "vec4 col = pangoGetColor(indices[i]);" << std::endl
		<< "frag = (col + (1.0 - col.a) * frag) * pangoAlpha;" << std::endl
		<< "}" << std::endl
		<< baseFragmentFooter()
	;

	return shaderSource.str();
}

std::string createDistanceFieldShader() {
	std::ostringstream shaderSource;

	shaderSource
		<< baseFragmentHeader(1)
		<< "uniform float pangoDFMin;" << std::endl
		<< "uniform float pangoDFMax;" << std::endl
		<< "void main() {" << std::endl
		<< "gl_FragColor = vec4(pangoColor[0].rgb, pangoAlpha) * smoothstep(" << std::endl
		<< "pangoDFMin, pangoDFMax, texture2D(pangoTexture[0], pangoTexCoord.st).a" << std::endl
		<< ");" << std::endl
		<< "}" << std::endl
	;

	OSG_NOTICE << shaderSource.str() << std::endl;

	return shaderSource.str();
}

}

