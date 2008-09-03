// -*-c++-*- osgPango - Copyright (C) 2008 Jeremy Moles

#include <iostream>
#include <sstream>
#include <osg/io_utils>
#include <osg/Texture2D>
#include <osg/TexMat>
#include <osgPango/Font>

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
_x         (1.0f),
_y         (1.0f),
_h         (1.0f),
_img       (0),
_imgWidth  (width),
_imgHeight (height) {
	_newImage();
}

const CachedGlyph* GlyphCache::getCachedGlyph(unsigned int i) {
	GlyphMap::const_iterator g = _glyphs.find(i);

	// If we already have cached data, return it.
	if(g != _glyphs.end()) return &g->second;

	// Otherwise, we need to start building the glyph cache.
	// This will try to create a new SurfaceImage (which acts like a canvas for our
	// glyph data) if the _img variable is larger than the actual size of our container.
	if(_images.size() < _img) _newImage();

	return 0;
};

const CachedGlyph* GlyphCache::createCachedGlyph(PangoFont* font, PangoGlyphInfo* gi) {
	unsigned int glyph = gi->glyph;
	PangoRectangle r;
	cairo_glyph_t  g;

	pango_font_get_glyph_extents(font, glyph, &r, 0);
	pango_extents_to_pixels(&r, 0);

	cairo_scaled_font_t*    sf = pango_cairo_font_get_scaled_font(PANGO_CAIRO_FONT(font));
	osgCairo::SurfaceImage* si = _images[_img].get();

	cairo_set_scaled_font(si->getContext(), sf);

	g.x     = -r.x;
	g.y     = -r.y;
	g.index = glyph;

	double w = r.width;
	double h = r.height;

	// If our remaining space isn't enough to accomodate another glyph, jump to
	// another "row."
	if(_x + w >= _imgWidth) {
		std::cout
			<< "Need to jump rows on; _imgWidth is: " << _imgWidth
			<< " and necessary space is: " << _x + w << std::endl
		;

		_x  = 1.0f;
		_y += _h + 1.0f;

		si->identityMatrix();

		std::cout << "Translating to: " << _x << " " << _y << std::endl;

		si->translate(_x, _y);
	}

	if(h > _h) _h = h;

	cairo_show_glyphs(si->getContext(), &g, 1);

	/*
	cairo_glyph_path(si->getContext(), &g, 1);
	si->setSourceRGBA(0.0f, 0.0f, 0.0f, 0.2f);
	si->setLineWidth(1.0f);
	si->stroke();
	*/

	_glyphs[glyph] = CachedGlyph(
		_img,
		osg::Vec2(r.x, -(h + r.y)),
		osg::Vec2(w, h),
		osg::Vec2(_x / _imgWidth      , (_imgHeight - h - _y) / _imgHeight),
		osg::Vec2((w + _x) / _imgWidth, (_imgHeight - h - _y) / _imgHeight),
		osg::Vec2((w + _x) / _imgWidth, (_imgHeight - _y) / _imgHeight),
		osg::Vec2(_x / _imgWidth      , (_imgHeight - _y) / _imgHeight)
	);

	if(w > 0.0f && h > 0.0f) {
		si->translate(w + 1.0f, 0.0f);

		_x += w + 1.0f;
	}

	return const_cast<const CachedGlyph*>(&_glyphs[glyph]);
}

void GlyphCache::writeImagesAsFiles(const std::string& prefix) const {
	unsigned int num = 0;

	for(ImageVector::const_iterator i = _images.begin(); i != _images.end(); i++) {
		std::ostringstream ss;

		ss << prefix << num << ".png";

		i->get()->writeToPNG(ss.str().c_str());

		num++;
	}
}

bool GlyphCache::_newImage() {
	_images.push_back(new osgCairo::SurfaceImage(_imgWidth, _imgHeight, 0, CAIRO_FORMAT_A8));

	osgCairo::SurfaceImage* si = _images[_images.size() - 1].get();

	if(!si || !si->valid() || !si->createContext()) return false;

	/*
	si->setSourceRGBA(1.0, 1.0, 1.0, 0.1f);
	si->rectangle(0.0f, 0.0f, _imgWidth, _imgHeight);
	si->fill();
	si->setSourceRGBA(1.0f, 1.0f, 1.0f, 1.0f);
	*/

	si->translate(1.0f, 1.0f);

	return true;
}

osgCairo::SurfaceImage* GlyphCache::_getImage(unsigned int index) const {
	if(index < _images.size()) return _images[index].get();

	else return 0;
}

osg::ref_ptr<osg::Vec3Array> GlyphGeometry::_norms;
osg::ref_ptr<osg::Vec4Array> GlyphGeometry::_cols;

GlyphGeometry::GlyphGeometry():
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
	setColorArray(_cols.get());
	setNormalArray(_norms.get());
	setNormalBinding(osg::Geometry::BIND_OVERALL);
	setColorBinding(osg::Geometry::BIND_OVERALL);
}

bool GlyphGeometry::finalize(osg::Image* image) {
	if(!image) return false;

	osg::Texture2D* texture = new osg::Texture2D();
	osg::StateSet*  state   = getOrCreateStateSet();

	texture->setImage(image);
	texture->setFilter(osg::Texture::MIN_FILTER, osg::Texture::LINEAR);
	texture->setFilter(osg::Texture::MAG_FILTER, osg::Texture::LINEAR);

	// TODO: Put this somewhere else higher in the tree...
	state->setTextureAttributeAndModes(
		0,
		texture,
		osg::StateAttribute::ON
	);

	osg::Matrix s = osg::Matrix::scale(1.0f, -1.0f, 1.0f);
	osg::Matrix t = osg::Matrix::translate(0.0f, -1.0, 0.0f);

	state->setTextureAttributeAndModes(
		0,
		new osg::TexMat(t * s),
		osg::StateAttribute::ON
	);

	state->setMode(GL_BLEND, osg::StateAttribute::ON);
	state->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);

	// std::cout << "Finalizing with " << _numQuads << " quads." << std::endl;

	addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::QUADS, 0, _numQuads * 4));

	return true;
}

bool GlyphGeometry::pushCachedGlyphAt(const CachedGlyph* cg, const osg::Vec2& pos) {
	osg::Vec3Array* verts = dynamic_cast<osg::Vec3Array*>(getVertexArray());
	osg::Vec2Array* texs  = dynamic_cast<osg::Vec2Array*>(getTexCoordArray(0));

	if(!verts) return false;

	osg::Vec2 origin = pos + cg->origin;

	/*
	std::cout 
		<< ">> pos (" << pos
		<< ") origin (" << cg->origin
		<< ") size (" << cg->size
		<< ") modified ( " << origin
		<< ")" << std::endl
	;
	*/

	verts->push_back(osg::Vec3(origin, 0.0f));
	verts->push_back(osg::Vec3(origin + osg::Vec2(cg->size.x(), 0.0f), 0.0f));
	verts->push_back(osg::Vec3(origin + cg->size, 0.0f));
	verts->push_back(osg::Vec3(origin + osg::Vec2(0.0f, cg->size.y()), 0.0f));

	texs->push_back(cg->bl);
	texs->push_back(cg->br);
	texs->push_back(cg->ur);
	texs->push_back(cg->ul);

	_numQuads++;

	return true;
}

}
