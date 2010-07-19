// -*-c++-*- Copyright (C) 2010 osgPango Development Team
// $Id$

#include <cmath>
#include <sstream>
#include <osg/AlphaFunc>
#include <osg/Depth>
#include <osg/Geode>
#include <osgCairo/Util>
#include <osgPango/ShaderManager>
#include <osgPango/GlyphRenderer>

namespace osgPango {

osg::Vec4 GlyphRenderer::getExtraGlyphExtents() const {
	if(!_layers.size()) return osg::Vec4(0.0f, 0.0f, 0.0f, 0.0f);
		
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
	
bool GlyphRenderer::renderLayer(
	unsigned int   layer,
	cairo_t*       c,
	cairo_glyph_t* glyph,
	unsigned int   width,
	unsigned int   height
) {
	if(layer < _layers.size()) return _layers[layer]->render(c, glyph, width, height);
	
	else return false;
}
	
bool GlyphRenderer::updateOrCreateState(int pass, osg::Geode* geode) {
	ShaderManager& sm = ShaderManager::instance();

	osg::Program* program = new osg::Program();
	osg::Shader*  vert    = sm.getShader("osgPango-vert");
	osg::Shader*  frag    = sm.getShader("osgPango-frag");

	program->setName("pangoRenderer");

	program->addShader(vert);
	program->addShader(frag);

	osg::StateSet* state = geode->getOrCreateStateSet();

	state->setAttributeAndModes(program);
	
	osg::Uniform* pangoTexture = new osg::Uniform(
		osg::Uniform::INT,
		"pangoTexture",
		_layers.size()
	);

	for(unsigned int i = 0; i < _layers.size(); ++i) pangoTexture->setElement(
		i,
		static_cast<int>(i)
	);

	state->getOrCreateUniform("pangoAlpha", osg::Uniform::FLOAT)->set(1.0f);
	state->addUniform(pangoTexture);
	
	state->setMode(GL_BLEND, osg::StateAttribute::ON);	
	//state->setAttributeAndModes(new osg::AlphaFunc(osg::AlphaFunc::GEQUAL, 0.01f));
	state->setAttribute(new osg::Depth(osg::Depth::LESS, 0.0, 1.0, false));

	return true;
}

bool GlyphRenderer::updateOrCreateState(
	osg::Geometry*            geometry,
	const GlyphGeometryState& ggs
) {
	if(
		ggs.textures.size() != _layers.size() ||
		ggs.colors.size() != _layers.size()
	) return false;
		
	osg::Uniform* pangoColor = new osg::Uniform(
		osg::Uniform::FLOAT_VEC3, 
		"pangoColor", 
		_layers.size()
	);

	osg::StateSet* state = geometry->getOrCreateStateSet();
	
	state->addUniform(pangoColor);
	
	for(unsigned int i = 0; i < _layers.size(); ++i) {
		pangoColor->setElement(i, ggs.colors[i]);
		
		state->setTextureAttributeAndModes(i, ggs.textures[i], osg::StateAttribute::ON);
	}

	return true;
}
	
void GlyphRenderer::addLayer(GlyphLayer *layer) {
	_layers.push_back(layer);
}

void GlyphRenderer::removeLayer(unsigned int index) {
	if(index < _layers.size()) _layers.erase(_layers.begin() + index);
}

void GlyphRenderer::replaceLayer(unsigned int index, GlyphLayer* layer) {
	if(index < _layers.size()) _layers[index] = layer;
}

bool GlyphRenderer::_setGetFragmentShader(osg::Geode* geode, const std::string& shaderName) {
	osg::StateSet* state   = geode->getOrCreateStateSet();
	osg::Program*  program = dynamic_cast<osg::Program*>(state->getAttribute(osg::StateAttribute::PROGRAM));

	if(!program) return false;
	
	osg::Shader* getFragmentShader = ShaderManager::instance().getShader(shaderName);

	if(!getFragmentShader) return false;

	program->addShader(getFragmentShader);

	return true;
}

GlyphRendererDefault::GlyphRendererDefault() {
	addLayer(new GlyphLayer());
}

bool GlyphRendererDefault::updateOrCreateState(int pass, osg::Geode* geode) {
	if(!GlyphRenderer::updateOrCreateState(pass, geode)) return false;

	return _setGetFragmentShader(geode, "osgPango-lib-1xLayer");
}

GlyphRendererOutline::GlyphRendererOutline(unsigned int outline) {
	addLayer(new GlyphLayer());
	addLayer(new GlyphLayerOutline(outline));
}

bool GlyphRendererOutline::updateOrCreateState(int pass, osg::Geode* geode) {
	if(!GlyphRenderer::updateOrCreateState(pass, geode)) return false;

	return _setGetFragmentShader(geode, "osgPango-lib-2xLayer");
}

GlyphRendererShadowOffset::GlyphRendererShadowOffset(int offsetX, int offsetY) {
	unsigned int xt = 0;
	unsigned int yt = 0;

	if(offsetX < 0) xt = std::abs(static_cast<double>(offsetX));

	if(offsetY < 0) yt = std::abs(static_cast<double>(offsetY));
	
	addLayer(new GlyphLayer());
	addLayer(new GlyphLayerShadowOffset(offsetX, offsetY));
}

bool GlyphRendererShadowOffset::updateOrCreateState(int pass, osg::Geode* geode) {
	if(!GlyphRenderer::updateOrCreateState(pass, geode)) return false;

	return _setGetFragmentShader(geode, "osgPango-lib-2xLayer");
}

GlyphRendererShadowGaussian::GlyphRendererShadowGaussian(unsigned int radius) {
	addLayer(new GlyphLayer());
	addLayer(new GlyphLayerShadowGaussian(radius));
}

bool GlyphRendererShadowGaussian::updateOrCreateState(int pass, osg::Geode* geode) {
	if(!GlyphRenderer::updateOrCreateState(pass, geode)) return false;

	return _setGetFragmentShader(geode, "osgPango-lib-2xLayer");
}

}
