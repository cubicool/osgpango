// -*-c++-*- Copyright (C) 2011 osgPango Development Team
// $Id$

#include <cmath>
#include <sstream>
#include <osg/BlendFunc>
#include <osg/Depth>
#include <osg/Geode>
#include <osg/TexEnvCombine>
#include <osgCairo/Util>
#include <osgPango/ShaderManager>
#include <osgPango/GlyphRenderer>
#include <osgPango/Glyph>
#include <osgPango/Context>

namespace osgPango {

const unsigned int DEFAULT_CACHE_WIDTH  = 256;
const unsigned int DEFAULT_CACHE_HEIGHT = 256;

GlyphRenderer::GlyphRenderer():
_pixelSpacing  (1.0f),
_textureWidth  (DEFAULT_CACHE_WIDTH),
_textureHeight (DEFAULT_CACHE_HEIGHT),
_minFilter     (osg::Texture::LINEAR) {
}

GlyphRenderer::GlyphRenderer(const GlyphRenderer& gr, const osg::CopyOp& copyOp):
osg::Object(gr, copyOp),
_pixelSpacing  (gr._pixelSpacing),
_textureWidth  (gr._textureWidth),
_textureHeight (gr._textureHeight),
_minFilter     (gr._minFilter) {
}

osg::Vec4 GlyphRenderer::getExtraGlyphExtents() const {
	if(!_layers.size()) return osg::Vec4(0.0f, 0.0f, 0.0f, 0.0f);
		
	osg::Vec4 result = _layers[0]->getExtraGlyphExtents();
	
	for(unsigned int i = 1; i < _layers.size(); i++) {
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
) const {
	if(layer < _layers.size()) return _layers[layer]->render(c, glyph, width, height);
	
	else return false;
}

bool GlyphRenderer::updateOrCreateState(int pass, osg::Geode* geode) const {
	osg::StateSet* state = geode->getOrCreateStateSet();

	state->setMode(GL_BLEND, osg::StateAttribute::ON);	
	state->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
	state->setAttribute(new osg::Depth(osg::Depth::LESS, 0.0, 1.0, false));
	// state->setMode(GL_DEPTH_TEST, osg::StateAttribute::OFF);

	ShaderManager& sm = ShaderManager::instance();

	osg::Program* program = new osg::Program();
	osg::Shader*  vert    = sm.getShader("osgPango-vert");

	program->setName("pangoRenderer");
	program->addShader(vert);

	state->setAttributeAndModes(program);
	
	osg::Uniform* pangoTexture = new osg::Uniform(
		osg::Uniform::SAMPLER_2D,
		"pangoTexture",
		_layers.size()
	);

	for(unsigned int i = 0; i < _layers.size(); i++) pangoTexture->setElement(
		i,
		static_cast<int>(i)
	);

	state->addUniform(pangoTexture);
	
	return true;
}

bool GlyphRenderer::updateOrCreateState(
	osg::Geometry*            geometry,
	const GlyphGeometryState& ggs
) const {
	if(
		ggs.textures.size() != _layers.size() ||
		ggs.colors.size() != _layers.size()
	) return false;
	
	osg::Uniform* pangoColor = new osg::Uniform(
		osg::Uniform::FLOAT_VEC4, 
		"pangoColor", 
		_layers.size()
	);

	osg::StateSet* state = geometry->getOrCreateStateSet();
	
	state->addUniform(pangoColor);
	
	for(unsigned int i = 0; i < _layers.size(); i++) {
		osg::Vec4 color(ggs.colors[i], 0.0f);

		// We use a dirty trick here; the alpha part of this Vec4 isn't actually
		// interpreted as alpha! It is used as a boolean to determine whether the
		// default shader should use the RGB color found in the texture or not.
		if(_layers[i]->getCairoImageFormat() == CAIRO_FORMAT_RGB24) color[3] = 1.0f;
		
		else if(_layers[i]->getCairoImageFormat() == CAIRO_FORMAT_ARGB32) color[3] = 2.0f;

		pangoColor->setElement(i, color); 
		
		state->setTextureAttributeAndModes(i, ggs.textures[i], osg::StateAttribute::ON);
	}

	return true;
}
	
osg::Texture2D* GlyphRenderer::createTexture(osg::Image* image) const {
	osg::Texture2D* texture = new osg::Texture2D();

	texture->setImage(image);
	texture->setFilter(osg::Texture::MIN_FILTER, _minFilter);
	texture->setFilter(osg::Texture::MAG_FILTER, osg::Texture::LINEAR);

	return texture;
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

void GlyphRenderer::clearLayers() {
	_layers.clear();
}

cairo_format_t GlyphRenderer::getImageFormatForLayer(unsigned int index) const {
	if(index < _layers.size()) return _layers[index]->getCairoImageFormat();
	
	else return CAIRO_FORMAT_A8;
}

GlyphCache* GlyphRenderer::getGlyphCache(PangoFont* font) const {
	guint hashFont = _hashFont(font);
	
	FontGlyphCacheMap::const_iterator ct = _gc.find(hashFont);

	return ct != _gc.end() ? ct->second : 0;
}

GlyphCache* GlyphRenderer::getOrCreateGlyphCache(PangoFont* font) {
	guint hashFont = _hashFont(font);
	
	FontGlyphCacheMap::const_iterator ct = _gc.find(hashFont);

	if(ct != _gc.end()) return ct->second;
	
	GlyphCache* cache = new GlyphCache(this, _textureWidth, _textureHeight);
	
	_gc[hashFont] = cache;

	return cache;
}

bool GlyphRenderer::_setFragmentShader(osg::Geode* geode, const std::string& shaderName) const {
	osg::StateSet* state   = geode->getOrCreateStateSet();
	osg::Program*  program = dynamic_cast<osg::Program*>(state->getAttribute(osg::StateAttribute::PROGRAM));

	if(!program) return false;
	
	osg::Shader* getFragmentShader = ShaderManager::instance().getShader(shaderName);

	if(!getFragmentShader) return false;

	state->setAttributeAndModes(
		new osg::BlendFunc(osg::BlendFunc::ONE, osg::BlendFunc::ONE_MINUS_SRC_ALPHA)
	);

	program->addShader(getFragmentShader);
	
	return true;
}

guint GlyphRenderer::_hashFont(PangoFont* font) const {
	PangoFontDescription* d    = pango_font_describe(font);
	guint                 hash = pango_font_description_hash(d);

	pango_font_description_free(d);

	return hash;
}

}
