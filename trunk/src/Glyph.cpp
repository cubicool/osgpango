// -*-c++-*- osgPango - Copyright (C) 2008 Jeremy Moles

#include <iostream>
#include <sstream>
#include <osg/io_utils>
#include <osg/Texture2D>
#include <osg/TexMat>
#include <osg/TexEnvCombine>
#include <osgPango/Font>

#define DEBUG_GLYPH_BOXES 0

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

GlyphCache::GlyphCache(unsigned int width, unsigned int height):
_glyphEffects (0),
_x            (0.0f),
_y            (0.0f),
_h            (0.0f),
_outlineSize  (0.0f),
_imgWidth     (width ? width : DEFAULT_GCW),
_imgHeight    (height ? height : DEFAULT_GCH),
_shadowOffset (0) {
}

const CachedGlyph* GlyphCache::getCachedGlyph(unsigned int i) {
	GlyphMap::const_iterator g = _glyphs.find(i);

	// If we already have cached data, return it.
	if(g != _glyphs.end()) return &g->second;

	return 0;
};

const CachedGlyph* GlyphCache::createCachedGlyph(PangoFont* font, PangoGlyphInfo* gi) {
	unsigned int glyph = gi->glyph;
	PangoRectangle r;
	cairo_glyph_t  g;

	pango_font_get_glyph_extents(font, glyph, &r, 0);
	pango_extents_to_pixels(&r, 0);

	g.x     = -r.x;
	g.y     = -r.y;
	g.index = glyph;

	double w = r.width;
	double h = r.height;

	if(w <= 0.0f && h <= 0.0f) {
		_glyphs[glyph] = CachedGlyph();

		return const_cast<const CachedGlyph*>(&_glyphs[glyph]);
	}

	if(!_images.size()) _newImage();

	cairo_scaled_font_t*    sf  = pango_cairo_font_get_scaled_font(PANGO_CAIRO_FONT(font));
	osgCairo::SurfaceImage* si  = _images.back().get();
	osgCairo::SurfaceImage* sio = 0;
	
	cairo_set_scaled_font(si->getContext(), sf);

	if(_glyphEffects & GLYPH_EFFECT_OUTLINE) {
		sio = _outlines.back().get();

		cairo_set_scaled_font(sio->getContext(), sf);
	}

	/*
	// TODO: -----------------
	cairo_font_options_t* siFO  = 0;
	cairo_font_options_t* sioFO = 0;

	cairo_surface_get_font_options(si->getSurface(), siFO);
	cairo_surface_get_font_options(sio->getSurface(), sioFO);

	// std::cout << "Options: " <<wq
	// TODO: -----------------
	*/

	// This condition is met after we create a new Image...
	if(!_x && !_y && !_h) _calculateInitialOrigin();

	double add = round(_outlineSize);

	// If our remaining space isn't enough to accomodate another glyph, jump to another "row."
	if(!_confirmHorizontalSpaceAvailable(w)) {
		_x  = add + 1.0f;
		_y += _h + (add * 2.0f) + 1.0f;

		if(_confirmVerticalSpaceAvailable(h)) {
			si = _images.back().get();

			cairo_set_scaled_font(si->getContext(), sf);
		}

		else {
			si->identityMatrix();
			si->translate(_x, _y);

			if(sio) {
				sio->identityMatrix();
				sio->translate(_x, _y);
			}
		}
	}

	if(h > _h) _h = h;

	#if DEBUG_GLYPH_BOXES
		si->setSourceRGBA(1.0f, 1.0f, 1.0f, 0.1f);
		si->rectangle(0.0f, 0.0f, w, h);
		si->fill();
		si->setSourceRGBA(1.0f, 1.0f, 1.0f, 1.0f);
	#endif

	if(sio) {
		cairo_glyph_path(sio->getContext(), &g, 1);
	
		/*
		osgCairo::RadialPattern rp(
			w / 2.0f, h / 2.0f, 0.0f,
			w / 2.0f, h / 2.0f, w - _outlineSize
		);

		rp.addColorStopRGBA(0.0f, 0.0f, 0.0f, 0.0f, 1.0f);
		rp.addColorStopRGBA(1.0f, 0.0f, 0.0f, 0.0f, 0.0f);

		sio->setSource(&rp);
		*/

		sio->setAntialias(CAIRO_ANTIALIAS_SUBPIXEL);
		sio->setLineWidth(_outlineSize);
		sio->setSourceRGBA(1.0f, 1.0f, 1.0f, 1.0f);
		sio->strokePreserve();
		sio->fill();
		
		/*
		osgCairo::CairoOperator op = sio->getOperator();
		
		sio->setOperator(CAIRO_OPERATOR_CLEAR);

		cairo_show_glyphs(sio->getContext(), &g, 1);

		sio->setOperator(op);
		*/
	}

	// Show the "regular" font...
	cairo_show_glyphs(si->getContext(), &g, 1);

	double tx = (_x - add) / _imgWidth;
	double ty = (_imgHeight - h - (_y + add)) / _imgHeight;
	double tw = (w + _x + add) / _imgWidth;
	double th = (_imgHeight - (_y - add)) / _imgHeight;

	_glyphs[glyph] = CachedGlyph(
		_images.size() - 1,
		osg::Vec2(r.x, -(h + r.y)),
		osg::Vec2(w + (add * 2.0f), h + (add * 2.0f)),
		osg::Vec2(tx, ty),
		osg::Vec2(tw, ty),
		osg::Vec2(tw, th),
		osg::Vec2(tx, th)
	);

	si->translate(w + (add * 2.0f) + 1.0f, 0.0f);

	if(sio) sio->translate(w + (add * 2.0f) + 1.0f, 0.0f);

	_x += w + (add * 2.0f) + 1.0f;

	return const_cast<const CachedGlyph*>(&_glyphs[glyph]);
}

void GlyphCache::writeImagesAsFiles(const std::string& prefix) const {
	unsigned int num = 0;

	for(ImageVector::const_iterator i = _images.begin(); i != _images.end(); i++) {
		std::ostringstream ss;

		ss << prefix << num << ".png";

		i->get()->writeToPNG(ss.str().c_str());

		/*
		std::cout 
			<< "Wrote " << ss.str()
			<< "; " << i->get()->getImageSizeInBytes() / 1024.0f
			<< " KB internally." << std::endl
		;
		*/

		num++;
	}

	num = 0;

	for(ImageVector::const_iterator i = _outlines.begin(); i != _outlines.end(); i++) {
		std::ostringstream ss;

		ss << prefix << num << "_outline.png";

		i->get()->writeToPNG(ss.str().c_str());

		num++;
	}
}

bool GlyphCache::_newImage() {
	_images.push_back(new osgCairo::SurfaceImage(_imgWidth, _imgHeight, 0, CAIRO_FORMAT_A8));

	osgCairo::SurfaceImage* si = _images[_images.size() - 1].get();

	if(_glyphEffects & GLYPH_EFFECT_OUTLINE) {
		_outlines.push_back(
			new osgCairo::SurfaceImage(_imgWidth, _imgHeight, 0, CAIRO_FORMAT_A8)
		);
		
		osgCairo::SurfaceImage* sio = _outlines[_outlines.size() - 1].get();

		if(!sio || !sio->valid() || !sio->createContext()) return false;
	}

	if(!si || !si->valid() || !si->createContext()) return false;

	return true;
}

bool GlyphCache::_confirmHorizontalSpaceAvailable(unsigned int w) {
	if(_x + w + (round(_outlineSize) * 2.0f) + 1 >= _imgWidth) return false;

	return true;
}

bool GlyphCache::_confirmVerticalSpaceAvailable(unsigned int h) {
	if(_y + h + (round(_outlineSize) * 2.0f) >= _imgHeight) {
		if(!_newImage()) return false;

		_x = 0.0f;
		_y = 0.0f;
		_h = 0.0f;

		_calculateInitialOrigin();

		return true;
	}

	return false;
}

void GlyphCache::_calculateInitialOrigin() {
	osgCairo::SurfaceImage* si = _images.back().get();
	
	unsigned int v = round(_outlineSize) + 1.0f;

	_x = v;
	_y = v;

	si->identityMatrix();
	si->translate(_x, _y);

	if(_glyphEffects & GLYPH_EFFECT_OUTLINE) {
		si = _outlines.back().get();
		
		si->identityMatrix();
		si->translate(_x, _y);
	}
}

osgCairo::SurfaceImage* GlyphCache::_getImage(unsigned int index, bool outline) const {
	if(!outline) {
		if(index < _images.size()) return _images[index].get();

		else return 0;
	}

	else {
		if(index < _outlines.size()) return _outlines[index].get();

		else return 0;
	}	
}

osg::ref_ptr<osg::Vec3Array> GlyphGeometry::_norms;
osg::ref_ptr<osg::Vec4Array> GlyphGeometry::_cols;

GlyphGeometry::GlyphGeometry(bool outlined):
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

	setUseDisplayList(false);
	setDataVariance(osg::Object::DYNAMIC);
	setVertexArray(new osg::Vec3Array());
	setTexCoordArray(0, new osg::Vec2Array());
	
	if(outlined) setTexCoordArray(1, new osg::Vec2Array());
	
	setColorArray(_cols.get());
	setNormalArray(_norms.get());
	setNormalBinding(osg::Geometry::BIND_OVERALL);
	setColorBinding(osg::Geometry::BIND_OVERALL);
}

bool GlyphGeometry::finalize(osg::Image* image, osg::Image* outlineImage) {
	if(!image) return false;

	osg::Texture2D* texture = new osg::Texture2D();
	osg::StateSet*  state   = getOrCreateStateSet();

	texture->setImage(image);
	texture->setFilter(osg::Texture::MIN_FILTER, osg::Texture::LINEAR);
	texture->setFilter(osg::Texture::MAG_FILTER, osg::Texture::LINEAR);

	// TODO: Put this somewhere else higher in the tree...
	state->setTextureAttributeAndModes(
		outlineImage ? 1 : 0,
		texture,
		osg::StateAttribute::ON
	);

	osg::Matrix s = osg::Matrix::scale(1.0f, -1.0f, 1.0f);
	osg::Matrix t = osg::Matrix::translate(0.0f, -1.0, 0.0f);

	state->setTextureAttributeAndModes(
		outlineImage ? 1 : 0,
		new osg::TexMat(t * s),
		osg::StateAttribute::ON
	);

	if(outlineImage) {
		osg::Texture2D*     otexture = new osg::Texture2D();
		osg::TexEnvCombine* te0      = new osg::TexEnvCombine();
		osg::TexEnvCombine* te1      = new osg::TexEnvCombine();

		otexture->setImage(outlineImage);
		otexture->setFilter(osg::Texture::MIN_FILTER, osg::Texture::LINEAR);
		otexture->setFilter(osg::Texture::MAG_FILTER, osg::Texture::LINEAR);

		te0->setCombine_RGB(osg::TexEnvCombine::MODULATE);
		te0->setSource0_RGB(osg::TexEnvCombine::CONSTANT);
		te0->setOperand0_RGB(osg::TexEnvCombine::SRC_COLOR);
		// This is the color of the border...
		te0->setConstantColor(osg::Vec4(1.0f, 1.0f, 1.0f, 0.0f));
		te0->setSource1_RGB(osg::TexEnvCombine::TEXTURE);
		te0->setOperand1_RGB(osg::TexEnvCombine::SRC_ALPHA);
		te0->setCombine_Alpha(osg::TexEnvCombine::REPLACE);
		te0->setSource0_Alpha(osg::TexEnvCombine::TEXTURE0);
		te0->setOperand0_Alpha(osg::TexEnvCombine::SRC_ALPHA);

		te1->setCombine_RGB(osg::TexEnvCombine::INTERPOLATE);
		te1->setSource1_RGB(osg::TexEnvCombine::PREVIOUS);
		te1->setOperand1_RGB(osg::TexEnvCombine::SRC_COLOR);
		te1->setSource2_RGB(osg::TexEnvCombine::TEXTURE1);
		te1->setOperand2_RGB(osg::TexEnvCombine::SRC_ALPHA);
		te1->setSource0_RGB(osg::TexEnvCombine::CONSTANT);
		te1->setOperand0_RGB(osg::TexEnvCombine::SRC_COLOR);
		// This is the color of the text...
		te1->setConstantColor(osg::Vec4(0.0f, 0.0f, 0.0f, 0.0f));
		te1->setCombine_Alpha(osg::TexEnvCombine::ADD);
		te1->setSource0_Alpha(osg::TexEnvCombine::TEXTURE1);
		te1->setOperand0_Alpha(osg::TexEnvCombine::SRC_ALPHA);
		te1->setSource1_Alpha(osg::TexEnvCombine::PREVIOUS);
		te1->setOperand1_Alpha(osg::TexEnvCombine::SRC_ALPHA);

		state->setTextureAttributeAndModes(
			0,
			otexture,
			osg::StateAttribute::ON
		);

		state->setTextureAttributeAndModes(
			0,
			new osg::TexMat(t * s),
			osg::StateAttribute::ON
		);

		state->setTextureAttributeAndModes(0, te0, osg::StateAttribute::ON);
		state->setTextureAttributeAndModes(1, te1, osg::StateAttribute::ON);
	}

	state->setMode(GL_BLEND, osg::StateAttribute::ON);
	state->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);

	addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::QUADS, 0, _numQuads * 4));

	return true;
}

bool GlyphGeometry::pushCachedGlyphAt(
	const CachedGlyph* cg,
	const osg::Vec2&   pos,
	double             outlineSize,
	unsigned int       shadowOffset,
	GlyphEffectsMethod gem
) {
	osg::Vec3Array* verts = dynamic_cast<osg::Vec3Array*>(getVertexArray());
	osg::Vec2Array* texs  = dynamic_cast<osg::Vec2Array*>(getTexCoordArray(0));

	if(!verts || !texs) return false;

	osg::Vec2 origin = pos + cg->origin;

	verts->push_back(osg::Vec3(origin, 0.0f));
	verts->push_back(osg::Vec3(origin + osg::Vec2(cg->size.x(), 0.0f), 0.0f));
	verts->push_back(osg::Vec3(origin + cg->size, 0.0f));
	verts->push_back(osg::Vec3(origin + osg::Vec2(0.0f, cg->size.y()), 0.0f));

	texs->push_back(cg->bl);
	texs->push_back(cg->br);
	texs->push_back(cg->ur);
	texs->push_back(cg->ul);

	_numQuads++;

	if(outlineSize > 0.0f) {
		osg::Vec2Array* otexs = dynamic_cast<osg::Vec2Array*>(getTexCoordArray(1));

		if(!otexs) return false;

		otexs->push_back(cg->bl);
		otexs->push_back(cg->br);
		otexs->push_back(cg->ur);
		otexs->push_back(cg->ul);
	}

	return true;
}

}
