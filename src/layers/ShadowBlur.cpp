// -*-c++-*- Copyright (C) 2011 osgPango Development Team
// $Id$

#include <cstdlib>
#include <osgPango/GlyphLayer>

namespace osgPango {

GlyphLayerShadowBlur::GlyphLayerShadowBlur(
	int          xOffset, 
	int          yOffset, 
	unsigned int radius, 
	unsigned int deviation
):
GlyphLayerInterfaceOffset (xOffset, yOffset),
GlyphLayerInterfaceBlur   (radius, deviation) {
}
	
bool GlyphLayerShadowBlur::render(
	cairo_t*       c,
	cairo_glyph_t* glyph,
	unsigned int   width,
	unsigned int   height
) {
	if(cairo_status(c) || !glyph) return false;
	
	cairo_translate(c, getOffsetX(), getOffsetY());
	cairo_push_group(c);
	cairo_set_line_join(c, CAIRO_LINE_JOIN_ROUND);
	cairo_set_line_width(c, _radius - 0.5f);
	cairo_glyph_path(c, glyph, 1);
	cairo_stroke_preserve(c);
	cairo_fill(c);
	
	cairo_pattern_t* pattern = cairo_pop_group(c);
	cairo_surface_t* tmp = createBlurredSurface(CAIRO_FORMAT_A8, pattern, width, height);

	cairo_set_source_surface(
		c,
		tmp,
		-static_cast<double>(_getBlurSize()) * 2,
		-static_cast<double>(_getBlurSize()) * 2
	);

	cairo_paint(c);
	cairo_surface_destroy(tmp);
	cairo_pattern_destroy(pattern);

	return true;
}
	
osg::Vec4 GlyphLayerShadowBlur::getExtraGlyphExtents() const {
	double offset   = std::max<double>(std::abs(getOffsetX()), std::abs(getOffsetY()));
	double blursize = _getBlurSize();
	
	return osg::Vec4(
		blursize + offset, 
		blursize + offset, 
		(blursize + offset) * 2.0, 
		(blursize + offset) * 2.0
	);
}

}
