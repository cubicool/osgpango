// -*-c++-*- Copyright (C) 2011 osgPango Development Team
// $Id$

#include <osg/Notify>
#include <osgPango/Util>
#include <osgPango/ShaderGenerator>
#include <osgPango/ShaderManager>

namespace osgPango {

ShaderManager ShaderManager::_shaderManager;

ShaderManager::~ShaderManager() {
}

ShaderManager& ShaderManager::instance() {
	return _shaderManager;
}

bool ShaderManager::addShader(const std::string& name, osg::Shader* shader) {
	if(name.empty() || !shader) return false;
	
	_shaders[name] = shader;

	return true;
}

bool ShaderManager::addShaderSource(
	const std::string& name,
	osg::Shader::Type  type,
	const std::string& source
) {
	if(name.empty() || source.empty()) return false;
	
	osg::Shader* shader = new osg::Shader(type, source);

	if(!shader) return false;

	shader->setName(name);

	_shaders[name] = shader;

	return true;
}

bool ShaderManager::addShaderFile(
	const std::string& name,
	osg::Shader::Type  type,
	const std::string& path
) {
	if(name.empty() || path.empty()) return false;

	std::string shaderFilePath = getFilePath(path);

	if(shaderFilePath.empty()) {
		osg::notify(osg::WARN)
			<< "Couldn't find the shader file '" << path
			<< "'; you likely specified the wrong path or haven't set OSGPANGO_FILE_PATH properly."
			<< std::endl;
		;

		return false;
	}

	osg::Shader* shader = osg::Shader::readShaderFile(type, shaderFilePath);

	if(!shader) return false;

	shader->setName(name);

	_shaders[name] = shader;

	return true;
}

osg::Shader* ShaderManager::getShader(const std::string& shader) {
	const ShaderMap::const_iterator i = _shaders.find(shader);

	if(i == _shaders.end()) return 0;

	return i->second;
}

ShaderManager::ShaderManager() {
	addShaderSource("osgPango-vert", osg::Shader::VERTEX, defaultVertexShader());
	addShaderSource("osgPango-frag1", osg::Shader::FRAGMENT, createBackToFrontShader(1));
	addShaderSource("osgPango-frag2", osg::Shader::FRAGMENT, createBackToFrontShader(2));
}

}
