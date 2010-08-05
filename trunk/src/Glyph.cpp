// -*-c++-*- Copyright (C) 2010 osgPango Development Team
// $Id$

#include <cstdlib>
#include <iostream>
#include <sstream>
#include <osg/Texture2D>
#include <osgDB/Registry>
#include <osgDB/ReadFile>
#include <osgCairo/Util>
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

const float GlyphCache::DEFAULT_BASE_X = 1.0f;
const float GlyphCache::DEFAULT_BASE_Y = 1.0f;
const float GlyphCache::DEFAULT_BASE_H = 0.0f;

GlyphCache::GlyphCache(
	GlyphRenderer* renderer,
	unsigned int   width,
	unsigned int   height
):
_renderer   (renderer),
_x          (DEFAULT_BASE_X),
_y          (DEFAULT_BASE_Y),
_h          (DEFAULT_BASE_H),
_imgWidth   (width),
_imgHeight  (height) {
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

	cairo_glyph_t g = { glyph, -r.x, -r.y };

	double w = r.width;
	double h = r.height;

	if(w <= 0.0f && h <= 0.0f) {
		_glyphs[glyph] = CachedGlyph();

		return const_cast<const CachedGlyph*>(&_glyphs[glyph]);
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
		_x  = DEFAULT_BASE_X;
		_y += _h + addh;
	}

	// Make sure we have enough vertical space, too.
	if(_y + h + addh >= _imgHeight || !_layers.size()) _newImageAndTexture();
	
	cairo_scaled_font_t* sf  = pango_cairo_font_get_scaled_font(PANGO_CAIRO_FONT(font));

	// Render glyph to layers.
	for(unsigned int layerIndex = 0; layerIndex < getNumLayers(); layerIndex++) {
		osgCairo::Image* img = _layers[layerIndex].back().first.get();
		cairo_t*         c   = img->createContext();

		if(cairo_status(c)) continue;

		cairo_set_scaled_font(c, sf);
		cairo_identity_matrix(c);

		// Set position in image and then move write position to origin of glyph. 
		// Each GlyphLayer can assume that writes on right position if don't apply any effect.
		cairo_translate(c, _x + extents[0], _y + extents[1]);
		cairo_save(c);

		if(!_renderer->renderLayer(layerIndex, c, &g, w, h)) osg::notify(osg::WARN) 
			<< "The GlyphRenderer object '" << _renderer->getName() << "' failed to render "
			<< "a glyph to the internal surface."
			<< std::endl
		;

		cairo_restore(c);
		cairo_destroy(c);
	}

	// TODO: Why can't I destroy this here? Is it because the 'font' object has it locked up?
	// cairo_scaled_font_destroy(sf);
	
	if(h > _h) _h = h;

	double tx = _x / _imgWidth;
	double ty = (_imgHeight - (h + extents[3]) - _y) / _imgHeight;
	double tw = (_x + w + extents[2]) / _imgWidth;
	double th = (_imgHeight - _y) / _imgHeight;
	
	if(!_layers.size()) {
		osg::notify(osg::WARN)
			<< "The internal layers container has no Image/Texture pair for this glyph. "
			<< "Your GlyphRenderer object is (mostly likely) reporting the wrong layer size."
			<< std::endl
		;

		return 0;
	}
	
	_glyphs[glyph] = CachedGlyph(
		_layers[0].size() - 1,
		osg::Vec2(r.x, -(h + r.y)),
		// We don't use addw/addh here because we don't want the extra 1.0f pixel,
		// which is used during linear filtering.
		osg::Vec2(w + extents[2], h + extents[3]),
		osg::Vec2(tx, ty),
		osg::Vec2(tw, ty),
		osg::Vec2(tw, th),
		osg::Vec2(tx, th)
	);
	
	_x += w + addw;
	
	return const_cast<const CachedGlyph*>(&_glyphs[glyph]);
}

void GlyphCache::writeImagesAsFiles(const std::string& prefix) const {
	for(unsigned int i = 0; i < getNumLayers(); i++) {
		std::ostringstream str;

		str << i;
		
		_writeImageVectorFiles(prefix + "layer" + str.str() + "_", "", _layers[i]);
	}
}

unsigned long GlyphCache::getMemoryUsageInBytes() const {
	unsigned long bytes = 0;

	for(Layers::const_iterator i = _layers.begin(); i != _layers.end(); i++) {
		for(Images::const_iterator j = i->begin(); j != i->end(); j++) {
			bytes += j->first->getImageSizeInBytes();
		}
	}

	return bytes;
}

bool GlyphCache::_newImageAndTexture() {
	if(!_layers.size()) _layers.resize(getNumLayers());
	
	for(unsigned int i = 0; i < getNumLayers(); ++i) {
		osgCairo::Image* img = new osgCairo::Image(
			_imgWidth, 
			_imgHeight, 
			_renderer->getImageFormatForLayer(i)
		);
	
		if(!img || !img->valid()) return false;
	
		osg::Texture2D* texture = getGlyphRenderer()->createTexture(img);
		
		_layers[i].push_back(std::make_pair(img, texture));
	}

	// Whenever a new image is created we reset our _x, _y, and _h values.
	// It's important that you do not create a new image unless you understand that this
	// will happen and how it will affect everything.
	_x = DEFAULT_BASE_X;
	_y = DEFAULT_BASE_Y;
	_h = DEFAULT_BASE_H;

	return true;
}

void GlyphCache::_writeImageVectorFiles(
	const std::string& prefix,
	const std::string& postfix,
	const Images&      images
) const {
	unsigned int num = 0;

	for(Images::const_iterator i = images.begin(); i != images.end(); i++) {
		// This should never, ever happen. :(
		if(!i->first.get()) continue;

		std::ostringstream ss;

		ss << prefix << num << postfix << ".png";

		osgCairo::util::writeToPNG(i->first->getSurface(), ss.str().c_str());

		osg::notify(osg::NOTICE)
			<< "Wrote " << ss.str()
			<< "; " << i->first.get()->getImageSizeInBytes() / 1024.0f
			<< " KB internally." << std::endl
		;

		num++;
	}
}

osgCairo::Image* GlyphCache::_getImage(unsigned int index, unsigned int layerIndex) const {
	if(
		layerIndex < _layers.size() &&
		index < _layers[layerIndex].size()
	) return _layers[layerIndex][index].first;
	
	else return 0;
}

osg::Texture* GlyphCache::_getTexture(unsigned int index, unsigned int layerIndex) const {
	if(
		layerIndex < _layers.size() &&
		index < _layers[layerIndex].size()
	) return _layers[layerIndex][index].second;

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

	setVertexArray(new osg::Vec3Array());
	setTexCoordArray(0, new osg::Vec2Array());
	
	setColorArray(_cols.get());
	setNormalArray(_norms.get());
	setNormalBinding(osg::Geometry::BIND_OVERALL);
	setColorBinding(osg::Geometry::BIND_OVERALL);
}

bool GlyphGeometry::finalize() {
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
	const osg::Vec2&   pos
) {
	static float z = 0.0f;
	static const osg::Matrix m(
		osg::Matrix::translate(0.0f, -1.0, 0.0f) *
		osg::Matrix::scale(1.0f, -1.0f, 1.0f));

	osg::Vec3Array* verts = dynamic_cast<osg::Vec3Array*>(getVertexArray());
	osg::Vec2Array* texs  = dynamic_cast<osg::Vec2Array*>(getTexCoordArray(0));

	if(!verts || !texs) return false;

	osg::Vec2 origin = pos + cg->origin;

	verts->push_back(osg::Vec3(origin, z));
	verts->push_back(osg::Vec3(origin + osg::Vec2(cg->size.x(), 0.0f), z));
	verts->push_back(osg::Vec3(origin + cg->size, z));
	verts->push_back(osg::Vec3(origin + osg::Vec2(0.0f, cg->size.y()), z));

	// z -= 0.001f;
	osg::Vec4 bl = osg::Vec4(cg->bl.x(), cg->bl.y(), 0.0, 1.0) * m;
	osg::Vec4 br = osg::Vec4(cg->br.x(), cg->br.y(), 0.0, 1.0) * m;
	osg::Vec4 ur = osg::Vec4(cg->ur.x(), cg->ur.y(), 0.0, 1.0) * m;
	osg::Vec4 ul = osg::Vec4(cg->ul.x(), cg->ul.y(), 0.0, 1.0) * m;

	texs->push_back(osg::Vec2(bl.x(), bl.y()));
	texs->push_back(osg::Vec2(br.x(), br.y()));
	texs->push_back(osg::Vec2(ur.x(), ur.y()));
	texs->push_back(osg::Vec2(ul.x(), ul.y()));

	_numQuads++;

	return true;
}

}
