// -*-c++-*- Copyright (C) 2010 osgPango Development Team
// $Id$

#include <osgCairo/Util>
#include <osgPango/GlyphLayer>

namespace osgPango {

GlyphLayer::GlyphLayer(cairo_format_t format):
_imageFormat(format) {
}

bool GlyphLayer::render(
	cairo_t*       c,
	cairo_glyph_t* glyph,
	unsigned int   width,
	unsigned int   height
) {
	if(cairo_status(c) || !glyph) return false;

	cairo_glyph_path(c, glyph, 1);
	cairo_fill(c);

	return true;
}

osg::Vec4 GlyphLayer::getExtraGlyphExtents() const {
	return osg::Vec4(0.0f, 0.0f, 0.0f, 0.0f);
}

}
