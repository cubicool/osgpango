// -*-c++-*- Copyright (C) 2010 osgPango Development Team
// $Id$

#include <osgPango/GlyphLayer>

namespace osgPango {

GlyphLayerOutline::GlyphLayerOutline(unsigned int outline):
_outline(outline) {
}

bool GlyphLayerOutline::render(
	cairo_t*       c,
	cairo_glyph_t* glyph,
	unsigned int   width,
	unsigned int   height
) {
	if(cairo_status(c) || !glyph) return false;
	
	cairo_set_line_join(c, CAIRO_LINE_JOIN_ROUND);
	cairo_set_line_width(c, (_outline * 2) - 0.5f);
	cairo_glyph_path(c, glyph, 1);
	cairo_stroke_preserve(c);
	cairo_fill(c);

	return true;
}

osg::Vec4 GlyphLayerOutline::getExtraGlyphExtents() const {
	return osg::Vec4(_outline, _outline, _outline * 2, _outline * 2);
}

}
