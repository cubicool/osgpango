// -*-c++-*- Copyright (C) 2011 osgPango Development Team
// $Id$

#include <cstdlib>
#include <osgCairo/Util>
#include <osgPango/GlyphLayer>

namespace osgPango {

GlyphLayerShadowInset::GlyphLayerShadowInset(
	int          xOffset,
	int          yOffset,
	unsigned int radius, 
	unsigned int deviation
):
GlyphLayerInterfaceOffset (xOffset, yOffset),
GlyphLayerInterfaceBlur   (radius, deviation) {
}
	
bool GlyphLayerShadowInset::render(
	cairo_t*       c,
	cairo_glyph_t* glyph,
	unsigned int   width,
	unsigned int   height
) {
	if(cairo_status(c) || !glyph) return false;
	
	cairo_push_group(c);
	cairo_glyph_path(c, glyph, 1);
	cairo_clip(c);
	cairo_translate(c, getOffsetX(), getOffsetY());
	// cairo_set_line_join(c, CAIRO_LINE_JOIN_ROUND);
	cairo_set_line_width(c, static_cast<double>(_radius) - 0.5f);
	cairo_glyph_path(c, glyph, 1);
	cairo_stroke(c);
	
	cairo_pattern_t* pattern = cairo_pop_group(c);
	cairo_surface_t* tmp     = createBlurredSurface(CAIRO_FORMAT_A8, pattern, width, height);

	cairo_set_source_surface(
		c,
		tmp,
		-static_cast<double>(_getBlurSize()) * 2,
		-static_cast<double>(_getBlurSize()) * 2
	);

	cairo_glyph_path(c, glyph, 1);
	cairo_clip(c);
	cairo_paint(c);

	cairo_surface_destroy(tmp);
	cairo_pattern_destroy(pattern);
	
	return true;
}

}
