// -*-c++-*- Copyright (C) 2010 osgPango Development Team
// $Id$

#include <cmath>
#include <osgCairo/Util>
#include <osgPango/GlyphLayer>

namespace osgPango {

GlyphLayer::GlyphLayer(unsigned int xt, unsigned int yt):
_xTranslate (xt),
_yTranslate (yt) {
}

bool GlyphLayer::render(
	osgCairo::Surface*     surface,
	const osgCairo::Glyph& glyph,
	unsigned int           width,
	unsigned int           height
) {
	if(!surface) return false;

	surface->translate(_xTranslate, _yTranslate);
	surface->glyphPath(glyph);
	surface->fill();

	return true;
}

osg::Vec4 GlyphLayer::getExtraGlyphExtents() const {
	return osg::Vec4(0.0, 0.0, 0.0, 0.0);
}

GlyphLayerOutline::GlyphLayerOutline(unsigned int outline):
_outline(outline) {
}

bool GlyphLayerOutline::render(
	osgCairo::Surface*     surface,
	const osgCairo::Glyph& glyph,
	unsigned int           width,
	unsigned int           height
) {
	if(!surface) return false;

	surface->setLineJoin(CAIRO_LINE_JOIN_ROUND);
	surface->setLineWidth((_outline * 2) - 0.5f);
	surface->setAntialias(CAIRO_ANTIALIAS_SUBPIXEL);
	surface->translate(_outline, _outline);
	surface->glyphPath(glyph);
	surface->strokePreserve();
	surface->fill();
	surface->setOperator(CAIRO_OPERATOR_CLEAR);
	
	GlyphLayer::render(surface, glyph, width, height);

	return true;
}

osg::Vec4 GlyphLayerOutline::getExtraGlyphExtents() const {
	return osg::Vec4(_outline, _outline, _outline * 2, _outline * 2);
}

GlyphLayerShadowOffset::GlyphLayerShadowOffset(int offsetX, int offsetY): 
_xOffset(offsetX),
_yOffset(offsetY) {
}

bool GlyphLayerShadowOffset::render(
	osgCairo::Surface*     surface,
	const osgCairo::Glyph& glyph,
	unsigned int           width,
	unsigned int           height
) {
	if(!surface) return false;

	surface->save();

	if(_xOffset > 0) surface->translate(_xOffset, 0.0f);

	if(_yOffset > 0) surface->translate(0.0f, _yOffset);

	GlyphLayer::render(surface, glyph, width, height);

	surface->restore();
	surface->setOperator(CAIRO_OPERATOR_CLEAR);

	if(_xOffset < 0) surface->translate(std::abs(_xOffset), 0.0f);

	if(_yOffset < 0) surface->translate(0.0f, std::abs(_yOffset));

	GlyphLayer::render(surface, glyph, width, height);

	return true;
}

osg::Vec4 GlyphLayerShadowOffset::getExtraGlyphExtents() const {
	return osg::Vec4(0.0f, 0.0f, std::abs(_xOffset), std::abs(_yOffset));
}

GlyphLayerShadowGaussian::GlyphLayerShadowGaussian(unsigned int radius):
_radius(radius) {
}
	
bool GlyphLayerShadowGaussian::render(
	osgCairo::Surface*     surface,
	const osgCairo::Glyph& glyph,
	unsigned int           width,
	unsigned int           height
) {
	if(!surface) return false;

	double add = _radius * 4.0f;
	
	// Create a temporary small surface and then copy that to the bigger one.
	osgCairo::Surface tmp(width + add, height + add, CAIRO_FORMAT_ARGB32);

	if(!tmp.createContext()) return false;
 
	osgCairo::CairoScaledFont* sf = surface->getScaledFont();

	tmp.setScaledFont(sf);
	tmp.setLineJoin(CAIRO_LINE_JOIN_ROUND);
	tmp.setLineWidth(_radius - 0.5f);
	tmp.setAntialias(CAIRO_ANTIALIAS_SUBPIXEL);
	tmp.translate(_radius * 2, _radius * 2);
	tmp.glyphPath(glyph);
	tmp.strokePreserve();
	tmp.fill();

	osgCairo::util::gaussianBlur(&tmp, _radius);
	
	tmp.setOperator(CAIRO_OPERATOR_CLEAR);
	
	GlyphLayer::render(&tmp, glyph, width, height);

	surface->setSourceSurface(&tmp, 0, 0);
	surface->paint();

	return true;
}
	
osg::Vec4 GlyphLayerShadowGaussian::getExtraGlyphExtents() const {
	return osg::Vec4(_radius * 2, _radius * 2, _radius * 4, _radius * 4);
}

}
