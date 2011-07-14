// -*-c++-*- Copyright (C) 2011 osgPango Development Team
// $Id$

#include <cmath>
#include <osgPango/GlyphLayer>

namespace osgPango {

GlyphLayerShadow::GlyphLayerShadow(int xOffset, int yOffset): 
GlyphLayerInterfaceOffset(xOffset, yOffset) {
}

bool GlyphLayerShadow::render(
	cairo_t*       c,
	cairo_glyph_t* glyph,
	unsigned int   width,
	unsigned int   height
) {
	if(cairo_status(c) || !glyph) return false;

	// cairo_save(c);

	if(_xOffset > 0) cairo_translate(c, _xOffset, 0.0f);

	if(_yOffset > 0) cairo_translate(c, 0.0f, _yOffset);

	GlyphLayer::render(c, glyph, width, height);

	/*
	cairo_restore(c);
	cairo_set_operator(c, CAIRO_OPERATOR_CLEAR);

	if(_xOffset < 0) cairo_translate(c, std::abs(static_cast<double>(_xOffset)), 0.0f);

	if(_yOffset < 0) cairo_translate(c, 0.0f, std::abs(static_cast<double>(_yOffset)));

	GlyphLayer::render(c, glyph, width, height);
	*/

	return true;
}

osg::Vec4 GlyphLayerShadow::getExtraGlyphExtents() const {
	return osg::Vec4(
		0.0f,
		0.0f,
		std::abs(static_cast<double>(_xOffset)),
		std::abs(static_cast<double>(_yOffset))
	);
}

}
