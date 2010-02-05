// -*-c++-*- osgPango - Copyright (C) 2008 Jeremy Moles

#include <cstdlib>
#include <iostream>
#include <sstream>
#include <osg/Texture2D>
#include <osg/TexMat>
#include <osg/TexEnvCombine>
#include <osgPango/Context>

namespace osgPango {

CachedGlyph::CachedGlyph(
	unsigned int     _img,
	const osg::Vec2& _origin,
	const osg::Vec2& _size,
	const osg::Vec2& _bl,
	const osg::Vec2& _br,
	const osg::Vec2& _ur,
	const osg::Vec2& _ul
):
img    (_img),
origin (_origin),
size   (_size),
bl     (_bl),
br     (_br),
ur     (_ur),
ul     (_ul) {
}

GlyphGeometryState::GlyphGeometryState(
	osg::Texture*    _texture,
	osg::Texture*    _effectsTexture,
	const osg::Vec3& _color,
	const osg::Vec3& _effectsColor,
	double           _alpha
):
texture        (_texture),
effectsTexture (_effectsTexture),
color          (_color),
effectsColor   (_effectsColor),
alpha          (_alpha) {
}

osg::Vec4 GlyphRenderer::getExtraGlyphExtents() const {
	return osg::Vec4(0.0f, 0.0f, 0.0f, 0.0f);
}

bool GlyphRenderer::renderGlyph(
	osgCairo::Image*        si,
	const osgCairo::Glyph&  g,
	unsigned int            w,
	unsigned int            h
) {
	if(!si) return false;
	
	si->glyphPath(g);
	si->fill();

	return true;
}

bool GlyphRenderer::renderGlyphEffects(
	osgCairo::Image*        si,
	const osgCairo::Glyph&  g,
	unsigned int            w,
	unsigned int            h
) {
	return false;
}

GlyphCache::GlyphCache(
	GlyphRenderer* renderer,
	unsigned int   width,
	unsigned int   height
):
_renderer   (renderer),
_x          (0.0f),
_y          (0.0f),
_h          (0.0f),
_imgWidth   (width),
_imgHeight  (height) {
	if(!_renderer.valid()) _renderer = new GlyphRenderer();
}

const CachedGlyph* GlyphCache::getCachedGlyph(unsigned int i) {
	GlyphMap::const_iterator g = _glyphs.find(i);

	// If we already have cached data, return it.
	if(g != _glyphs.end()) return &g->second;

	return 0;
};

const CachedGlyph* GlyphCache::createCachedGlyph(PangoFont* font, PangoGlyphInfo* gi) {
	unsigned int   glyph = gi->glyph;
	PangoRectangle r;

	pango_font_get_glyph_extents(font, glyph, &r, 0);
	pango_extents_to_pixels(&r, 0);

	osgCairo::Glyph g(glyph, -r.x, -r.y);

	double w = r.width;
	double h = r.height;

	if(w <= 0.0f && h <= 0.0f) {
		_glyphs[glyph] = CachedGlyph();

		return const_cast<const CachedGlyph*>(&_glyphs[glyph]);
	}

	// We can't do this in constructor because we need to give the object time
	// to allow the user to set various caching options.
	if(!_images.size()) _newImageAndTexture(_images, _textures);

	bool hasEffects = _renderer->hasEffects();

	if(hasEffects && !_effects.size()) _newImageAndTexture(_effects, _effectsTextures);

	osgCairo::ScaledFont sf(pango_cairo_font_get_scaled_font(PANGO_CAIRO_FONT(font)));

	osgCairo::Image* si  = _images.back().get();
	osgCairo::Image* sie = 0;
	
	si->setScaledFont(&sf);

	if(hasEffects) {
		sie = _effects.back().get();

		sie->setScaledFont(&sf);
	}

	osg::Vec4 extents = _renderer->getExtraGlyphExtents();

	double addw = extents[2] + 1.0f;
	double addh = extents[3] + 1.0f;

	if(w + addw >= _imgWidth || h + addh >= _imgHeight) {
		osg::notify(osg::WARN)
			<< "The single glyph " << glyph 
			<< " cannot fit on the allocated texture size; this is likely a critical"
			<< " bug. Please make sure you have a large enough texture to properly"
			<< " cache the desired font." << std::endl
		;

		return 0;
	}

	// If our remaining space isn't enough to accomodate another glyph, jump to another "row."
	if(_x + w + addw >= _imgWidth) {
		_x  = 1.0f;
		_y += _h + addh;
	}
	
	// Make sure we have enough vertical space, too.
	if(_y + h + addh >= _imgHeight) {
		_newImageAndTexture(_images, _textures);

		si = _images.back().get();

		si->setScaledFont(&sf);

		if(hasEffects) {
			_newImageAndTexture(_effects, _effectsTextures);

			sie = _effects.back().get();

			sie->setScaledFont(&sf);
		}
	}

	else {
		si->identityMatrix();
		si->translate(_x, _y);

		if(sie) {
			sie->identityMatrix();
			sie->translate(_x, _y);
		}
	}

	if(h > _h) _h = h;

	si->save();
	
	_renderer->renderGlyph(si, g, w, h);

	si->restore();

	if(sie) {
		sie->save();

		_renderer->renderGlyphEffects(sie, g, w, h);

		sie->restore();
	}

	double tx = _x / _imgWidth;
	double ty = (_imgHeight - (h + extents[3]) - _y) / _imgHeight;
	double tw = (_x + w + extents[2]) / _imgWidth;
	double th = (_imgHeight - _y) / _imgHeight;

	_glyphs[glyph] = CachedGlyph(
		_images.size() - 1,
		osg::Vec2(r.x, -(h + r.y)),
		// We don't use addw/addh here because we don't want the extra 1.0f pixel,
		// which is used during linear filtering.
		osg::Vec2(w + extents[2], h + extents[3]),
		osg::Vec2(tx, ty),
		osg::Vec2(tw, ty),
		osg::Vec2(tw, th),
		osg::Vec2(tx, th)
	);

	si->translate(w + addw, 0.0f);

	if(sie) sie->translate(w + addw, 0.0f);

	_x += w + addw;

	return const_cast<const CachedGlyph*>(&_glyphs[glyph]);
}

void GlyphCache::writeImagesAsFiles(const std::string& prefix) const {
	_writeImageVectorFiles(prefix, "", _images);
	_writeImageVectorFiles(prefix, "_effects", _effects);
}

bool GlyphCache::_newImageAndTexture(ImageVector& images, TextureVector& textures) {
	images.push_back(new osgCairo::Image(_imgWidth, _imgHeight, CAIRO_FORMAT_A8));

	osgCairo::Image* si = images[images.size() - 1].get();
	
	if(!si || !si->valid() || !si->createContext()) return false;

	osg::Texture2D* texture = new osg::Texture2D();

	texture->setImage(si);
	texture->setFilter(osg::Texture::MIN_FILTER, osg::Texture::LINEAR);
	texture->setFilter(osg::Texture::MAG_FILTER, osg::Texture::LINEAR);

	textures.push_back(texture);

	// Whenever a new image is created we reset our _x, _y, and _h values.
	// It's important that you do not create a new image unless you understand that this
	// will happen and how it will affect everything.
	
	_x = 1.0f;
	_y = 1.0f;
	_h = 0.0f;

	si->identityMatrix();
	si->translate(_x, _y);

	return true;
}

void GlyphCache::_writeImageVectorFiles(
	const std::string& prefix,
	const std::string& postfix,
	const ImageVector& images
) const {
	unsigned int num = 0;

	for(ImageVector::const_iterator i = images.begin(); i != images.end(); i++) {
		// This should never, ever happen. :(
		if(!i->get()) continue;

		std::ostringstream ss;

		ss << prefix << num << postfix << ".png";

		i->get()->writeToPNG(ss.str().c_str());

		osg::notify()
			<< "Wrote " << ss.str()
			<< "; " << i->get()->getImageSizeInBytes() / 1024.0f
			<< " KB internally." << std::endl
		;

		num++;
	}
}

osgCairo::Image* GlyphCache::_getImage(unsigned int index, bool effects) const {
	if(!effects) {
		if(index < _images.size()) return _images[index].get();

		else return 0;
	}

	else {
		if(index < _effects.size()) return _effects[index].get();

		else return 0;
	}	
}

osg::Texture* GlyphCache::_getTexture(unsigned int index, bool effects) const {
	if(!effects) {
		if(index < _textures.size()) return _textures[index].get();

		else return 0;
	}

	else {
		if(index < _effectsTextures.size()) return _effectsTextures[index].get();

		else return 0;
	}
}

osg::ref_ptr<osg::Vec3Array> GlyphGeometry::_norms;
osg::ref_ptr<osg::Vec4Array> GlyphGeometry::_cols;

GlyphGeometry::GlyphGeometry(bool hasEffects):
_numQuads(0) {
	if(!_norms.valid()) {
		_norms = new osg::Vec3Array(1);

		(*_norms)[0].set(0.0f, 0.0f, 1.0f);
		(*_norms)[0].normalize();
	}

	if(!_cols.valid()) {
		_cols = new osg::Vec4Array(1);

		(*_cols)[0].set(1.0f, 1.0f, 1.0f, 1.0f);
	}

	setVertexArray(new osg::Vec3Array());
	setTexCoordArray(0, new osg::Vec2Array());
	
	// If we need an additional array, add it now.
	if(hasEffects) setTexCoordArray(1, new osg::Vec2Array());
	
	setColorArray(_cols.get());
	setNormalArray(_norms.get());
	setNormalBinding(osg::Geometry::BIND_OVERALL);
	setColorBinding(osg::Geometry::BIND_OVERALL);
}

template<typename T>
bool _setGlyphGeometryState(T* obj, const GlyphGeometryState& gs) {
	if(!gs.texture) return false;
	
	osg::TexEnvCombine* te0   = new osg::TexEnvCombine();
	osg::StateSet*      state = obj->getOrCreateStateSet();

	// TODO: Put this somewhere else higher in the tree...
	state->setTextureAttributeAndModes(
		gs.effectsTexture ? 1 : 0,
		gs.texture,
		osg::StateAttribute::ON
	);

	osg::Matrix s = osg::Matrix::scale(1.0f, -1.0f, 1.0f);
	osg::Matrix t = osg::Matrix::translate(0.0f, -1.0, 0.0f);

	state->setTextureAttributeAndModes(
		gs.effectsTexture ? 1 : 0,
		new osg::TexMat(t * s),
		osg::StateAttribute::ON
	);

	// This is the color of the border...
	te0->setConstantColor(osg::Vec4(gs.effectsTexture ? gs.effectsColor : gs.color, 1.0f));

	// RGB setup for te0.
	te0->setCombine_RGB(osg::TexEnvCombine::MODULATE);
	te0->setSource0_RGB(osg::TexEnvCombine::CONSTANT);
	te0->setOperand0_RGB(osg::TexEnvCombine::SRC_COLOR);
	te0->setOperand1_RGB(osg::TexEnvCombine::SRC_ALPHA);

	// Alpha setup for te0.
	te0->setCombine_Alpha(osg::TexEnvCombine::REPLACE);
	te0->setSource0_Alpha(osg::TexEnvCombine::TEXTURE0);
	te0->setOperand0_Alpha(osg::TexEnvCombine::SRC_ALPHA);

	state->setTextureAttributeAndModes(0, te0, osg::StateAttribute::ON);

	if(gs.effectsTexture) {
		osg::TexEnvCombine* te1 = new osg::TexEnvCombine();

		// This is the color of the text...
		te1->setConstantColor(osg::Vec4(gs.color, 1.0f));

		// RGB setup for te1.
		te1->setCombine_RGB(osg::TexEnvCombine::INTERPOLATE);
		te1->setSource0_RGB(osg::TexEnvCombine::CONSTANT);
		te1->setSource1_RGB(osg::TexEnvCombine::PREVIOUS);
		te1->setSource2_RGB(osg::TexEnvCombine::TEXTURE1);
		te1->setOperand0_RGB(osg::TexEnvCombine::SRC_COLOR);
		te1->setOperand1_RGB(osg::TexEnvCombine::SRC_COLOR);
		te1->setOperand2_RGB(osg::TexEnvCombine::SRC_ALPHA);

		// Alpha setup for te1.
		te1->setCombine_Alpha(osg::TexEnvCombine::ADD);
		te1->setSource0_Alpha(osg::TexEnvCombine::TEXTURE1);
		te1->setSource1_Alpha(osg::TexEnvCombine::PREVIOUS);
		te1->setOperand0_Alpha(osg::TexEnvCombine::SRC_ALPHA);
		te1->setOperand1_Alpha(osg::TexEnvCombine::SRC_ALPHA);

		state->setTextureAttributeAndModes(
			0,
			gs.effectsTexture,
			osg::StateAttribute::ON
		);

		state->setTextureAttributeAndModes(
			0,
			new osg::TexMat(t * s),
			osg::StateAttribute::ON
		);

		state->setTextureAttributeAndModes(1, te1, osg::StateAttribute::ON);
	}

	state->setTextureAttributeAndModes(
		gs.effectsTexture ? 2 : 1,
		gs.texture,
		osg::StateAttribute::ON
	);

	// The ALPHA stuff.
	osg::TexEnvCombine* te2 = new osg::TexEnvCombine();
	
	te2->setConstantColor(osg::Vec4(0.0f, 0.0f, 0.0f, gs.alpha));

	te2->setCombine_RGB(osg::TexEnvCombine::REPLACE);
	te2->setSource0_RGB(osg::TexEnvCombine::PREVIOUS);
	te2->setOperand0_RGB(osg::TexEnvCombine::SRC_COLOR);
	te2->setCombine_Alpha(osg::TexEnvCombine::MODULATE);
	te2->setSource0_Alpha(osg::TexEnvCombine::CONSTANT);
	te2->setSource1_Alpha(osg::TexEnvCombine::PREVIOUS);
	te2->setOperand0_Alpha(osg::TexEnvCombine::SRC_ALPHA);
	te2->setOperand1_Alpha(osg::TexEnvCombine::SRC_ALPHA);

	state->setTextureAttributeAndModes(
		gs.effectsTexture ? 2 : 1,
		te2,
		osg::StateAttribute::ON
	);

	state->setMode(GL_BLEND, osg::StateAttribute::ON);
	state->setMode(GL_DEPTH_TEST, osg::StateAttribute::OFF);
	state->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);

	return true;
}

bool setGlyphGeometryState(osg::Drawable* drawable, const GlyphGeometryState& gs) {
	return _setGlyphGeometryState(drawable, gs);
}

bool setGlyphGeometryState(osg::Node* node, const GlyphGeometryState& gs) {
	return _setGlyphGeometryState(node, gs);
}

bool GlyphGeometry::finalize(const GlyphGeometryState& gs) {
	if(!setGlyphGeometryState(this, gs)) return false;

	addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::QUADS, 0, _numQuads * 4));
	
	/*
	setDataVariance(osg::Object::STATIC);
	setUseDisplayList(false);
	setUseVertexBufferObjects(true);
	*/

	setDataVariance(osg::Object::DYNAMIC);
	setUseDisplayList(false);
	setUseVertexBufferObjects(false);

	return true;
}

bool GlyphGeometry::pushCachedGlyphAt(
	const CachedGlyph* cg,
	const osg::Vec2&   pos,
	bool               effects,
	GlyphEffectsMethod gem
) {
	static float z = 0.0f;

	osg::Vec3Array* verts = dynamic_cast<osg::Vec3Array*>(getVertexArray());
	osg::Vec2Array* texs  = dynamic_cast<osg::Vec2Array*>(getTexCoordArray(0));

	if(!verts || !texs) return false;

	osg::Vec2 origin = pos + cg->origin;

	verts->push_back(osg::Vec3(origin, z));
	verts->push_back(osg::Vec3(origin + osg::Vec2(cg->size.x(), 0.0f), z));
	verts->push_back(osg::Vec3(origin + cg->size, z));
	verts->push_back(osg::Vec3(origin + osg::Vec2(0.0f, cg->size.y()), z));

	// z -= 0.001f;

	texs->push_back(cg->bl);
	texs->push_back(cg->br);
	texs->push_back(cg->ur);
	texs->push_back(cg->ul);

	if(effects) {
		osg::Vec2Array* otexs = dynamic_cast<osg::Vec2Array*>(getTexCoordArray(1));

		if(!otexs) return false;

		otexs->push_back(cg->bl);
		otexs->push_back(cg->br);
		otexs->push_back(cg->ur);
		otexs->push_back(cg->ul);
	}

	_numQuads++;

	return true;
}

bool GlyphGeometry::setAlpha(double alpha) {
	osg::StateSet* state = getOrCreateStateSet();

	if(!state) return false;

	osg::TexEnvCombine* tec = dynamic_cast<osg::TexEnvCombine*>(
		state->getTextureAttribute(
			getNumTexCoordArrays() == 2 ? 2 : 1,
			osg::StateAttribute::TEXENV
		)
	);

	if(!tec) return false;

	tec->setConstantColor(osg::Vec4(0.0f, 0.0f, 0.0f, alpha));

	return true;
}

GlyphRendererOutline::GlyphRendererOutline(unsigned int size):
_outline(size) {
}

osg::Vec4 GlyphRendererOutline::getExtraGlyphExtents() const {
	return osg::Vec4(_outline, _outline, _outline * 2, _outline * 2);
}

bool GlyphRendererOutline::renderGlyph(
	osgCairo::Image*       si,
	const osgCairo::Glyph& g,
	unsigned int           w,
	unsigned int           h
) {
	if(!si) return false;

	si->translate(_outline, _outline);

	GlyphRenderer::renderGlyph(si, g, w, h);

	return true;
}

bool GlyphRendererOutline::renderGlyphEffects(
	osgCairo::Image*       si,
	const osgCairo::Glyph& g,
	unsigned int           w,
	unsigned int           h
) {
	if(!si) return false;

	si->setLineJoin(CAIRO_LINE_JOIN_ROUND);
	si->setLineWidth((_outline * 2) - 0.5f);
	si->setAntialias(CAIRO_ANTIALIAS_SUBPIXEL);
	si->translate(_outline, _outline);
	si->glyphPath(g);
	si->strokePreserve();
	si->fill();
	si->setOperator(CAIRO_OPERATOR_CLEAR);
	
	GlyphRenderer::renderGlyph(si, g, w, h);

	return true;
}

GlyphRendererShadowOffset::GlyphRendererShadowOffset(int x, int y):
_xOffset (x),
_yOffset (y) {
}

osg::Vec4 GlyphRendererShadowOffset::getExtraGlyphExtents() const {
	return osg::Vec4(0.0f, 0.0f, std::abs(_xOffset), std::abs(_yOffset));
}

bool GlyphRendererShadowOffset::renderGlyph(
	osgCairo::Image*       si,
	const osgCairo::Glyph& g,
	unsigned int           w,
	unsigned int           h
) {
	if(!si) return false;
	
	if(_xOffset < 0) si->translate(std::abs(_xOffset), 0.0f);

	if(_yOffset < 0) si->translate(0.0f, std::abs(_yOffset));

	GlyphRenderer::renderGlyph(si, g, w, h);

	return true;
}

bool GlyphRendererShadowOffset::renderGlyphEffects(
	osgCairo::Image*       si,
	const osgCairo::Glyph& g,
	unsigned int           w,
	unsigned int           h
) {
	if(!si) return false;

	si->save();

	if(_xOffset > 0) si->translate(_xOffset, 0.0f);

	if(_yOffset > 0) si->translate(0.0f, _yOffset);

	GlyphRenderer::renderGlyph(si, g, w, h);

	si->restore();
	si->setOperator(CAIRO_OPERATOR_CLEAR);

	if(_xOffset < 0) si->translate(std::abs(_xOffset), 0.0f);

	if(_yOffset < 0) si->translate(0.0f, std::abs(_yOffset));

	GlyphRenderer::renderGlyph(si, g, w, h);

	return true;
}

/*
GlyphCacheShadowGaussian::GlyphCacheShadowGaussian(
	unsigned int w,
	unsigned int h,
	unsigned int radius
):
GlyphRenderer (w, h, true),
_radius       (radius) {
}

osg::Vec4 GlyphCacheShadowGaussian::getExtraGlyphExtents() const {
	return osg::Vec4(_radius, _radius, _radius * 2, _radius * 2);
}

bool GlyphCacheShadowGaussian::renderGlyph(
	osgCairo::Image*       si,
	const osgCairo::Glyph& g,
	unsigned int           w,
	unsigned int           h
) {
	if(!si) return false;

	si->translate(_radius, _radius);

	GlyphCache::renderGlyph(si, g, w, h);

	return true;
}

bool GlyphCacheShadowGaussian::renderGlyphEffects(
	osgCairo::Image*        si,
	const osgCairo::Glyph&  g,
	unsigned int            w,
	unsigned int            h
) {
	if(!si) return false;

	si->setLineWidth(_radius - 0.5f);
	si->setAntialias(CAIRO_ANTIALIAS_SUBPIXEL);
	si->translate(_radius, _radius);
	si->glyphPath(g);
	si->strokePreserve();
	si->fill();
	//si->gaussianBlur(10);
	si->setOperator(CAIRO_OPERATOR_CLEAR);
	
	GlyphCache::renderGlyph(si, g, w, h);

	return true;
}
*/

}
