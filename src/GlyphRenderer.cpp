#include <cstdlib>
#include <iostream>
#include <sstream>
#include <osg/AlphaFunc>
#include <osg/Depth>
#include <osg/Texture2D>
#include <osg/Geode>
#include <osgCairo/Util>
#include <osgPango/GlyphRenderer>

namespace osgPango {
osg::Vec4 GlyphRenderer::getExtraGlyphExtents() const {
	return osg::Vec4(0.0f, 0.0f, 0.0f, 0.0f);
}

bool GlyphRenderer::updateOrCreateState(int pass, osg::Geode* geode) {
	return true;
}

bool GlyphRenderer::updateOrCreateState(osg::Geometry* geometry, const GlyphGeometryState& gs) {
	return true;
}

SinglePassGlyphRenderer::SinglePassGlyphRenderer() {
}
	
osg::Vec4 SinglePassGlyphRenderer::getExtraGlyphExtents() const {
	if(!_layers.size())
		return osg::Vec4(0.0f, 0.0f, 0.0f, 0.0f);
		
	osg::Vec4 result = _layers[0]->getExtraGlyphExtents();
	
	for(unsigned int i = 1; i < _layers.size(); ++i) {
		osg::Vec4 extents = _layers[i]->getExtraGlyphExtents();
		result[0] = std::max(result[0], extents[0]);
		result[1] = std::max(result[1], extents[1]);
		result[2] = std::max(result[2], extents[2]);
		result[3] = std::max(result[3], extents[3]);
	}
	return result;
}
	
bool SinglePassGlyphRenderer::renderLayer(
	unsigned int layer,
	osgCairo::Surface* si,
	const osgCairo::Glyph& g,
	unsigned int w, 
	unsigned int h
) {
	if(layer < _layers.size()) 
		return _layers[layer]->render(si, g, w, h);
	else
	  return false;
}
	
bool SinglePassGlyphRenderer::updateOrCreateState (int pass, osg::Geode* geode) {
	const char* VERTEX_SHADER =
		"#version 120\n"
		"varying vec4 pangoTexCoord;"
		"void main() {"
		"	pangoTexCoord = gl_MultiTexCoord0;"
		"	gl_Position = ftransform();"
		"}"
		;
		
	const char* OLD_FRAGMENT_SHADER = 
		"varying vec4 pangoTexCoord;"
		"uniform vec3 pangoColor[8];"
		"uniform sampler2D pangoTex0;"
		"uniform sampler2D pangoTex1;"
		"uniform float pangoAlpha;"
		"void main() {"
		"	float tex0   = texture2D(pangoTex1, pangoTexCoord.st).a;" // effect
		"	float tex1   = texture2D(pangoTex0, pangoTexCoord.st).a;" // base glyph
		"	vec3 color0  = pangoColor[1].rgb * tex0;"
		"	vec3 color1  = pangoColor[0].rgb * tex1 + color0 * (1.0 - tex1);"
		"	float alpha0 = tex0;"
		"	float alpha1 = tex0 + tex1;"
		"	gl_FragColor = vec4(color1, alpha1 * pangoAlpha);"
		"}"
		;
		
	//const char* SIMPLE_FRAGMENT = 
	//"varying vec4 pangoTexCoord;"
	//"uniform vec3 pangoColor[8];"
	//"uniform sampler2D pangoTex0;"
	//"uniform sampler2D pangoTex1;"
	//"uniform float pangoAlpha;"
	//"void main() {"
	//"	float tex0   = texture2D(pangoTex0, pangoTexCoord.st).a;"
	//"	vec3 color0  = pangoColor[0].rgb * tex0;"
	//"	float alpha0 = tex0;"
	//"	gl_FragColor = vec4(color0, alpha0);"
	//"}"
	//;
	
	
	std::ostringstream fragment;
	fragment 
		<< "varying vec4 pangoTexCoord;" << std::endl 
		<< "uniform float pangoAlpha;" << std::endl 
		<< "uniform vec3 pangoColor[" << _layers.size() << "];"  << std::endl;
		
	for(unsigned int i = 0; i < _layers.size(); ++i)
		fragment 
			<< "uniform sampler2D pangoTex" << i << ";" << std::endl;
		
	fragment 
		<< "void main() {" << std::endl;
	
	for(unsigned int i = 0; i < _layers.size(); ++i) {
		fragment 
			<< "	float tex" << i << " = texture2D(pangoTex" << i <<", pangoTexCoord.st).a;" << std::endl;
	}

	for(unsigned int i = 0; i < _layers.size(); ++i) {
		if(i == 0) {
			fragment 
				<< "	vec3 color0 = pangoColor[0].rgb * tex0;" << std::endl
			  << "	float alpha = tex0;" << std::endl;
		}
		
		else {
			fragment 
				<< "	vec3 color" << i << "  = pangoColor[" << i << "].rgb * tex" 
				<< i << " + color" << i - 1 << " * (1.0 - tex" << i << ");" << std::endl
				<< "	alpha = alpha + tex" << i << ";" << std::endl;
		}
	}
		
	fragment 
		<< "	gl_FragColor = vec4(color" << _layers.size() - 1 <<", alpha * pangoAlpha);" << std::endl
		<<"}" << std::endl;
		
	//std::cout << fragment.str() << std::endl;

	osg::Program* program = new osg::Program();
	osg::Shader*  vert    = new osg::Shader(osg::Shader::VERTEX, VERTEX_SHADER);
	//osg::Shader*  frag    = new osg::Shader(osg::Shader::FRAGMENT, SIMPLE_FRAGMENT);
	osg::Shader*  frag    = new osg::Shader(osg::Shader::FRAGMENT, OLD_FRAGMENT_SHADER);
	//osg::Shader*  frag    = new osg::Shader(osg::Shader::FRAGMENT, fragment.str().c_str());

	vert->setName("pangoRendererVert");
	frag->setName("pangoRendererFrag");
	program->setName("pangoRenderer");

	program->addShader(vert);
	program->addShader(frag);

	osg::StateSet* state = geode->getOrCreateStateSet();

	state->setAttributeAndModes(program);
	state->setMode(GL_BLEND, osg::StateAttribute::ON);
	state->setAttributeAndModes(new osg::AlphaFunc(osg::AlphaFunc::GEQUAL, 0.01f));
	state->setAttribute(new osg::Depth(osg::Depth::LESS, 0.0, 1.0, false));
	
	state->getOrCreateUniform("pangoAlpha", osg::Uniform::FLOAT)->set(1.0f);
	for(unsigned int i = 0; i < _layers.size(); ++i) {
		std::ostringstream str;
		
		str << "pangoTex" << i;
		
		state->getOrCreateUniform(str.str(), osg::Uniform::INT)->set(static_cast<int>(i));
	}
	return true;
}

bool SinglePassGlyphRenderer::updateOrCreateState (osg::Geometry* geometry, const GlyphGeometryState& gs) {
	if(gs.textures.size() != _layers.size() || gs.colors.size() != _layers.size()) 
		return false;
		
	osg::Uniform* pangoColor = new osg::Uniform(
		osg::Uniform::FLOAT_VEC3, 
		"pangoColor", 
		_layers.size());

	osg::StateSet* state = geometry->getOrCreateStateSet();
	state->addUniform(pangoColor);
	
	for(unsigned int i = 0; i < _layers.size(); ++i) {
		pangoColor->setElement(i, gs.colors[i]);
		state->setTextureAttributeAndModes(i, gs.textures[i], osg::StateAttribute::ON);
	}

	return true;
}
	
void SinglePassGlyphRenderer::addLayer(GlyphLayer *layer) {
	_layers.push_back(layer);
}

void SinglePassGlyphRenderer::removeLayer(unsigned int index) {
  if(index < _layers.size())
		_layers.erase(_layers.begin() + index);
}

void SinglePassGlyphRenderer::replaceLayer(unsigned int index, GlyphLayer* layer) {
	if(index < _layers.size())
		_layers[index] = layer;
}



GlyphRendererOutline::GlyphRendererOutline(unsigned int outline) {
	addLayer(new GlyphLayer());
	addLayer(new GlyphLayerOutline(outline));
}

GlyphRendererShadowOffset::GlyphRendererShadowOffset(int offsetX, int offsetY) {
	addLayer(new GlyphLayer());
	addLayer(new GlyphLayerShadowOffset(offsetX, offsetY));
}

GlyphRendererShadowGaussian::GlyphRendererShadowGaussian(unsigned int radius) {
	addLayer(new GlyphLayer());
	addLayer(new GlyphLayerShadowGaussian(radius));
}

GlyphRendererShadowGaussianMultipass::GlyphRendererShadowGaussianMultipass(unsigned int) {
}




}
