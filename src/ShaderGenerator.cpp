// -*-c++-*- Copyright (C) 2010 osgPango Development Team
// $Id$

#include <sstream>
#include <osgPango/ShaderGenerator>

namespace osgPango {
namespace shadergen {

const char* DEFAULT_VERTEX_SHADER =
	"varying vec4 pangoTexCoord;"
	"void main() {"
	"	pangoTexCoord = gl_MultiTexCoord0;"
	"	gl_Position = ftransform();"
	"}"
;

const char* DEFAULT_FRAGMENT_SHADER =
	"#version 120\n"
	"varying vec4 pangoTexCoord;"
	"uniform vec3 pangoColor[8];"
	"uniform sampler2D pangoTexture[8];"
	"uniform float pangoAlpha;"
	"vec4 osgPango_GetFragment(vec4, sampler2D[8], vec3[8], float);"
	"void main() {"
	"	gl_FragColor = osgPango_GetFragment(pangoTexCoord, pangoTexture, pangoColor, pangoAlpha);"
	"}"
;

const char* GET_FRAGMENT_PROTOTYPE =
	"vec4 osgPango_GetFragment(vec4 coord, sampler2D textures[8], vec3 colors[8], float alpha)"
;

std::string create1xLayerShader(unsigned int layer) {
	std::ostringstream shaderSource;

	shaderSource
		<< "#version 120\n"
		<< GET_FRAGMENT_PROTOTYPE << "{"
		<< "return vec4(colors[" << layer
		<< "].rgb, texture2D(textures[" << layer
		<< "], coord.st).a * alpha);"
		<< "}"
	;

	return shaderSource.str();
}

// TODO: Refine this, make it iterate over a uniform.
std::string create2xLayerShader(unsigned int layer0, unsigned int layer1) {
	std::ostringstream shaderSource;

	shaderSource
		<< "#version 120\n"
		<< GET_FRAGMENT_PROTOTYPE << "{"
		<< "float alpha0 = texture2D(textures[" << layer0 << "], coord.st).a;"
		<< "float alpha1 = texture2D(textures[" << layer1 << "], coord.st).a;"
		<< "vec3 color0 = colors[" << layer0 << "];"
		<< "vec3 color1 = colors[" << layer1 << "];"
		<< "return mix(vec4(color0, alpha0), vec4(color1, alpha1), alpha1 * alpha);"
		<< "}"
	;

	return shaderSource.str();
}

/*
std::string create2xLayerShader_old(unsigned int layer0, unsigned int layer1) {
	std::ostringstream shaderSource;

	shaderSource
		<< "#version 120\n"
		<< GET_FRAGMENT_PROTOTYPE << "{"
		<< "float tex0 = texture2D(textures[" << layer0 << "], coord.st).a;"
		<< "float tex1 = texture2D(textures[" << layer1 << "], coord.st).a;"
		<< "vec3 color0 = colors[" << layer0 << "].rgb * tex0;"
		<< "vec3 color1 = colors[" << layer1 << "].rgb * tex1 + color0 * (1.0 - tex1);"
		<< "float alpha0 = tex0;"
		<< "float alpha1 = tex0 + tex1;"
		<< "return vec4(color1, alpha1 * alpha);"
		<< "}"
	;

	return shaderSource.str();
}
*/

const char* getDefaultVertexShader() {
	return DEFAULT_VERTEX_SHADER;
}

const char* getDefaultFragmentShader() {
	return DEFAULT_FRAGMENT_SHADER;
}

}
}
